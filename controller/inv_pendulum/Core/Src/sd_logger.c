#include "sd_logger.h"
#include "diskio.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

extern void Debug_Printf(const char* format, ...);

/* FatFS objects */
FATFS FatFs;
uint8_t fatfs_initialized = 0;

HAL_StatusTypeDef SD_Logger_Init(SD_Logger_Handle_t *hlogger)
{
    FRESULT res;

    hlogger->is_open = 0;
    hlogger->sample_count = 0;
    memset(hlogger->filename, 0, sizeof(hlogger->filename));

    Debug_Printf("SD_Logger_Init: Starting...\r\n");

    /* Check disk status */
    Debug_Printf("  Checking disk status...\r\n");
    DSTATUS disk_stat = disk_status(0);
    Debug_Printf("  Disk status: 0x%02X ", disk_stat);
    if (disk_stat & STA_NOINIT) Debug_Printf("(NOT_INIT) ");
    if (disk_stat & STA_NODISK) Debug_Printf("(NO_DISK) ");
    if (disk_stat & STA_PROTECT) Debug_Printf("(PROTECTED) ");
    Debug_Printf("\r\n");

    /* Initialize disk */
    Debug_Printf("  Initializing disk...\r\n");
    disk_stat = disk_initialize(0);
    Debug_Printf("  Disk init result: 0x%02X\r\n", disk_stat);

    if (disk_stat != 0) {
        Debug_Printf("  FAILED: Disk initialization error!\r\n");
        fatfs_initialized = 0;
        return HAL_ERROR;
    }
    Debug_Printf("  Disk initialized OK\r\n");

    /* Mount filesystem */
    Debug_Printf("  Mounting filesystem...\r\n");
    res = f_mount(&FatFs, "", 1);
    Debug_Printf("  f_mount result: %d ", res);

    switch(res) {
        case FR_OK:
            Debug_Printf("(OK)\r\n");
            fatfs_initialized = 1;
            break;
        case FR_DISK_ERR:
            Debug_Printf("(DISK_ERR - Low level I/O error)\r\n");
            fatfs_initialized = 0;
            return HAL_ERROR;
        case FR_NOT_READY:
            Debug_Printf("(NOT_READY - Drive not ready)\r\n");
            fatfs_initialized = 0;
            return HAL_ERROR;
        case FR_NO_FILESYSTEM:
            Debug_Printf("(NO_FILESYSTEM - Not FAT32)\r\n");
            fatfs_initialized = 0;
            return HAL_ERROR;
        default:
            Debug_Printf("(Error %d)\r\n", res);
            fatfs_initialized = 0;
            return HAL_ERROR;
    }

    Debug_Printf("SD_Logger_Init: SUCCESS!\r\n");
    return HAL_OK;
}

HAL_StatusTypeDef SD_Logger_StartLogging(SD_Logger_Handle_t *hlogger)
{
    FRESULT res;
    char header[128];
    UINT bw;

    if (!fatfs_initialized) {
        Debug_Printf("SD_Logger_Start: FAILED - SD not initialized\r\n");
        return HAL_ERROR;
    }

    if (hlogger->is_open) {
        Debug_Printf("SD_Logger_Start: Already logging\r\n");
        return HAL_ERROR;
    }

    Debug_Printf("SD_Logger_Start: Finding available filename...\r\n");

    /* Find next available filename */
    for (int i = 1; i < 1000; i++) {
        sprintf(hlogger->filename, "RIP_%03d.CSV", i);

        res = f_open(&hlogger->file, hlogger->filename, FA_READ);
        if (res == FR_NO_FILE) {
            Debug_Printf("  Found available: %s\r\n", hlogger->filename);
            break;
        }
        f_close(&hlogger->file);
    }

    /* Create and open file */
    Debug_Printf("  Creating file: %s\r\n", hlogger->filename);
    res = f_open(&hlogger->file, hlogger->filename, FA_CREATE_ALWAYS | FA_WRITE);

    if (res != FR_OK) {
        Debug_Printf("  FAILED: f_open error %d\r\n", res);
        return HAL_ERROR;
    }

    Debug_Printf("  File opened successfully\r\n");

    /* Write CSV header */
    sprintf(header, "Time(ms),Alpha(rad),AlphaDot(rad/s),Theta(rad),ThetaDot(rad/s)\r\n");
    res = f_write(&hlogger->file, header, strlen(header), &bw);

    if (res != FR_OK) {
        Debug_Printf("  FAILED: Header write error %d\r\n", res);
        f_close(&hlogger->file);
        return HAL_ERROR;
    }

    Debug_Printf("  Header written: %d bytes\r\n", bw);

    /* Sync to ensure header is written */
    f_sync(&hlogger->file);

    hlogger->is_open = 1;
    hlogger->sample_count = 0;

    Debug_Printf("SD_Logger_Start: SUCCESS\r\n");
    return HAL_OK;
}

HAL_StatusTypeDef SD_Logger_StopLogging(SD_Logger_Handle_t *hlogger)
{
    if (!hlogger->is_open) {
        Debug_Printf("SD_Logger_Stop: Not logging\r\n");
        return HAL_ERROR;
    }

    Debug_Printf("SD_Logger_Stop: Closing file...\r\n");
    Debug_Printf("  Total samples written: %lu\r\n", hlogger->sample_count);

    /* Sync and close file */
    f_sync(&hlogger->file);
    f_close(&hlogger->file);

    hlogger->is_open = 0;

    Debug_Printf("SD_Logger_Stop: SUCCESS\r\n");
    return HAL_OK;
}

HAL_StatusTypeDef SD_Logger_WriteData(SD_Logger_Handle_t *hlogger,
                                       float alpha, float alpha_dot,
                                       float theta, float theta_dot)
{
    char line[128];
    UINT bw;
    FRESULT res;

    if (!hlogger->is_open) {
        return HAL_ERROR;
    }

    /* Format data line */
    uint32_t time_ms = HAL_GetTick();
    sprintf(line, "%lu,%.4f,%.4f,%.4f,%.4f\r\n",
            time_ms, alpha, alpha_dot, theta, theta_dot);

    /* Write to file */
    res = f_write(&hlogger->file, line, strlen(line), &bw);

    if (res != FR_OK) {
        Debug_Printf("SD_Logger_Write: ERROR %d at sample %lu\r\n", res, hlogger->sample_count);
        return HAL_ERROR;
    }

    hlogger->sample_count++;

    /* Sync every 100 samples to prevent data loss */
    if (hlogger->sample_count % 100 == 0) {
        f_sync(&hlogger->file);
        Debug_Printf("SD_Logger: Synced at sample %lu\r\n", hlogger->sample_count);
    }

    return HAL_OK;
}

uint8_t SD_Logger_IsLogging(SD_Logger_Handle_t *hlogger)
{
    return hlogger->is_open;
}

uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t *hlogger)
{
    return hlogger->sample_count;
}

char* SD_Logger_GetFilename(SD_Logger_Handle_t *hlogger)
{
    return hlogger->filename;
}

void SD_Logger_PrintStatus(void)
{
    if (!fatfs_initialized) {
        Debug_Printf("Logger not initialized\r\n");
        return;
    }

    FATFS *fs;
    DWORD fre_clust;
    FRESULT res = f_getfree("", &fre_clust, &fs);

    if (res == FR_OK) {
        uint32_t total = (fs->n_fatent - 2) * fs->csize / 2; // KB
        uint32_t free = fre_clust * fs->csize / 2; // KB
        Debug_Printf("SD Card: %lu KB total, %lu KB free\r\n", total, free);
    } else {
        Debug_Printf("Error reading SD card info: %d\r\n", res);
    }
}
