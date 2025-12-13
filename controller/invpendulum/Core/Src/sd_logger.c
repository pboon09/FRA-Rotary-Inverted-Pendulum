#include "sd_logger.h"
#include "sd_spi.h"
#include "diskio.h"
#include <string.h>
#include <stdio.h>

/* FatFS objects */
static FATFS FatFs;
static FIL LogFile;
static uint8_t fatfs_initialized = 0;

extern void Debug_Printf(const char* format, ...);

HAL_StatusTypeDef SD_Logger_Init(SD_Logger_Handle_t *hlogger)
{
    FRESULT res;

    hlogger->is_open = 0;
    hlogger->sample_count = 0;
    memset(hlogger->filename, 0, sizeof(hlogger->filename));

    /* Initialize SD card low-level driver */
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

uint8_t SD_Logger_CreateFile(SD_Logger_Handle_t *hlogger, const char *filename)
{
    FRESULT res;

    if (!fatfs_initialized) {
        Debug_Printf("SD_Logger_CreateFile: FatFS not initialized\r\n");
        return 0;
    }

    /* Close existing file if open */
    if (hlogger->is_open) {
        f_close(&LogFile);
        hlogger->is_open = 0;
    }

    /* Store filename */
    strncpy(hlogger->filename, filename, sizeof(hlogger->filename) - 1);

    /* Create new file (overwrite if exists) */
    res = f_open(&LogFile, filename, FA_WRITE | FA_CREATE_ALWAYS);

    if (res != FR_OK) {
        Debug_Printf("SD_Logger_CreateFile: Failed to create %s (error %d)\r\n", filename, res);
        return 0;
    }

    hlogger->is_open = 1;
    hlogger->sample_count = 0;

    Debug_Printf("SD_Logger_CreateFile: Created %s\r\n", filename);
    return 1;
}

void SD_Logger_Close(SD_Logger_Handle_t *hlogger)
{
    if (hlogger->is_open) {
        f_sync(&LogFile);
        f_close(&LogFile);
        hlogger->is_open = 0;
        Debug_Printf("SD_Logger_Close: Closed %s (%lu samples)\r\n",
                    hlogger->filename, hlogger->sample_count);
    }
}

uint8_t SD_Logger_WriteHeader(SD_Logger_Handle_t *hlogger)
{
    if (!hlogger->is_open) return 0;

    const char *header = "Sample,Alpha,Alpha_Dot,Theta,Theta_Dot\r\n";
    UINT bytesWritten;
    FRESULT res;

    res = f_write(&LogFile, header, strlen(header), &bytesWritten);

    if (res == FR_OK && bytesWritten == strlen(header)) {
        f_sync(&LogFile);
        return 1;
    }

    Debug_Printf("SD_Logger_WriteHeader: Failed (error %d)\r\n", res);
    return 0;
}

uint8_t SD_Logger_WriteData(SD_Logger_Handle_t *hlogger, float alpha, float alpha_dot, float theta, float theta_dot)
{
    if (!hlogger->is_open) return 0;

    char buffer[128];
    UINT bytesWritten;
    FRESULT res;

    /* Format: Sample,Alpha,Alpha_Dot,Theta,Theta_Dot */
    int len = snprintf(buffer, sizeof(buffer), "%lu,%.6f,%.6f,%.6f,%.6f\r\n",
                      hlogger->sample_count,
                      alpha, alpha_dot, theta, theta_dot);

    res = f_write(&LogFile, buffer, len, &bytesWritten);

    if (res == FR_OK && bytesWritten == len) {
        hlogger->sample_count++;

        /* Sync every 100 samples to prevent data loss */
        if (hlogger->sample_count % 100 == 0) {
            f_sync(&LogFile);
        }

        return 1;
    }

    return 0;
}

void SD_Logger_Sync(SD_Logger_Handle_t *hlogger)
{
    if (hlogger->is_open) {
        f_sync(&LogFile);
    }
}

uint8_t SD_Logger_IsOpen(SD_Logger_Handle_t *hlogger)
{
    return hlogger->is_open;
}

uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t *hlogger)
{
    return hlogger->sample_count;
}

uint8_t SD_Logger_IsFatFSInitialized(void)
{
    return fatfs_initialized;
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

/* Additional utility functions for general file operations */
uint8_t SD_Logger_WriteString(const char *filename, const char *data)
{
    FIL file;
    FRESULT res;
    UINT bytesWritten;

    if (!fatfs_initialized) return 1;

    res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) return 2;

    res = f_write(&file, data, strlen(data), &bytesWritten);
    f_close(&file);

    return (res == FR_OK && bytesWritten == strlen(data)) ? 0 : 3;
}

uint8_t SD_Logger_WriteLine(const char *filename, const char *data)
{
    FIL file;
    FRESULT res;
    UINT bytesWritten;

    if (!fatfs_initialized) return 1;

    res = f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) {
        res = f_open(&file, filename, FA_WRITE | FA_CREATE_NEW);
        if (res != FR_OK) return 2;
    }

    res = f_write(&file, data, strlen(data), &bytesWritten);
    if (res != FR_OK || bytesWritten != strlen(data)) {
        f_close(&file);
        return 3;
    }

    res = f_write(&file, "\r\n", 2, &bytesWritten);
    f_close(&file);

    return (res == FR_OK) ? 0 : 3;
}

uint8_t SD_Logger_WriteData_Binary(const char *filename, const uint8_t *data, uint32_t len)
{
    FIL file;
    FRESULT res;
    UINT bytesWritten;

    if (!fatfs_initialized) return 1;

    res = f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) {
        res = f_open(&file, filename, FA_WRITE | FA_CREATE_NEW);
        if (res != FR_OK) return 2;
    }

    res = f_write(&file, data, len, &bytesWritten);
    f_close(&file);

    return (res == FR_OK && bytesWritten == len) ? 0 : 3;
}
