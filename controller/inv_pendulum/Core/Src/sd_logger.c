#include "sd_logger.h"
#include <string.h>
#include <stdio.h>

/* External debug function */
extern void Debug_Printf(const char* format, ...);

/* External RTC handle */
extern RTC_HandleTypeDef hrtc;

/* Helper function to convert state enum to string */
static const char* SD_Logger_StateToString(PendulumState state)
{
    switch (state) {
        case STATE_WAIT_BUTTON: return "WAITING";
        case STATE_KICK:        return "KICK";
        case STATE_SWINGUP:     return "SWINGUP";
        case STATE_LQR:         return "BALANCE";
        case STATE_EMERGENCY:   return "EMERGENCY";
        default:                return "UNKNOWN";
    }
}

HAL_StatusTypeDef SD_Logger_Init(SD_Logger_Handle_t* hlogger)
{
    if (hlogger == NULL) {
        Debug_Printf("SD_Logger_Init: NULL pointer\r\n");
        return HAL_ERROR;
    }

    /* Clear structure */
    memset(hlogger, 0, sizeof(SD_Logger_Handle_t));

    /* Test if SD card is accessible */
    DIR dir;
    FRESULT res = f_opendir(&dir, "/");
    Debug_Printf("SD_Logger_Init: Testing SD access, result=%d\r\n", res);

    if (res == FR_OK) {
        /* SD card is mounted and accessible */
        f_closedir(&dir);
        Debug_Printf("SD_Logger_Init: SD card accessible\r\n");
    } else if (res == 12) {  // FR_NO_FILESYSTEM
        /* Not mounted - try to mount it */
        Debug_Printf("SD_Logger_Init: SD not mounted, attempting mount...\r\n");

        /* FatFS variables should be in app_fatfs.c or fatfs.c */
        extern FATFS USERFatFs;
        extern char USERPath[4];

        res = f_mount(&USERFatFs, USERPath, 1);
        Debug_Printf("SD_Logger_Init: f_mount result=%d\r\n", res);

        if (res != FR_OK) {
            Debug_Printf("SD_Logger_Init: Mount FAILED!\r\n");
            return HAL_ERROR;
        }

        Debug_Printf("SD_Logger_Init: Mount SUCCESS\r\n");
    } else {
        /* Some other error */
        Debug_Printf("SD_Logger_Init: SD error %d\r\n", res);
        return HAL_ERROR;
    }

    /* SD card ready */
    hlogger->is_initialized = 1;
    Debug_Printf("SD_Logger_Init: SUCCESS\r\n");

    return HAL_OK;
}

HAL_StatusTypeDef SD_Logger_StartLogging(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger || !hlogger->is_initialized) {
        Debug_Printf("SD_Logger_Start: Not initialized\r\n");
        return HAL_ERROR;
    }

    if (hlogger->is_logging) {
        Debug_Printf("SD_Logger_Start: Already logging\r\n");
        return HAL_BUSY;
    }

    Debug_Printf("SD_Logger_Start: Finding available filename...\r\n");

    /* Find available filename - EXACT ORIGINAL LOGIC */
    for (int i = 1; i < 1000; i++) {
        sprintf(hlogger->filename, "RIP_%03d.CSV", i);

        FILINFO fno;
        FRESULT res = f_stat(hlogger->filename, &fno);

        /* Debug first few attempts */
        if (i <= 3) {
            Debug_Printf("  Try %d: %s -> res=%d\r\n", i, hlogger->filename, res);
        }

        if (res == FR_NO_FILE) {
            Debug_Printf("  Found available: %s\r\n", hlogger->filename);
            break;
        }

        if (i == 999) {
            Debug_Printf("SD_Logger_Start: No available filename\r\n");
            return HAL_ERROR;
        }
    }

    /* Create and open file */
    Debug_Printf("  Creating file: %s\r\n", hlogger->filename);
    FRESULT res = f_open(&hlogger->file, hlogger->filename,
                        FA_CREATE_ALWAYS | FA_WRITE);

    if (res != FR_OK) {
        Debug_Printf("SD_Logger_Start: f_open failed: %d\r\n", res);
        return HAL_ERROR;
    }

    Debug_Printf("  File opened successfully\r\n");

    /* Write CSV header with RTC and State columns */
    const char* header = "Date,Time,State,Sample,Alpha,Alpha_Dot,Theta,Theta_Dot\r\n";
    UINT bytes_written;
    res = f_write(&hlogger->file, header, strlen(header), &bytes_written);

    if (res != FR_OK || bytes_written != strlen(header)) {
        Debug_Printf("SD_Logger_Start: Header write failed\r\n");
        f_close(&hlogger->file);
        return HAL_ERROR;
    }

    Debug_Printf("  Header written: %u bytes\r\n", bytes_written);

    /* Initialize logging state */
    hlogger->buffer_level = 0;
    hlogger->sample_count = 0;
    hlogger->is_logging = 1;
    hlogger->is_flushing = 0;

    Debug_Printf("SD_Logger_Start: SUCCESS\r\n");
    return HAL_OK;
}

HAL_StatusTypeDef SD_Logger_StopLogging(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger || !hlogger->is_logging) {
        return HAL_ERROR;
    }

    Debug_Printf("SD_Logger_Stop: Flushing final data...\r\n");

    /* Flush any remaining data */
    if (hlogger->buffer_level > 0) {
        SD_Logger_FlushBuffer(hlogger);
    }

    /* Close file */
    FRESULT res = f_close(&hlogger->file);
    if (res != FR_OK) {
        Debug_Printf("SD_Logger_Stop: f_close failed: %d\r\n", res);
    }

    hlogger->is_logging = 0;
    Debug_Printf("SD_Logger_Stop: Logged %lu samples\r\n",
                hlogger->sample_count);

    return HAL_OK;
}

HAL_StatusTypeDef SD_Logger_WriteDataWithState(SD_Logger_Handle_t* hlogger,
                                               RTC_TimeTypeDef* time,
                                               RTC_DateTypeDef* date,
                                               PendulumState state,
                                               float alpha, float alpha_dot,
                                               float theta, float theta_dot)
{
    if (!hlogger || !hlogger->is_logging) {
        return HAL_ERROR;
    }

    if (!time || !date) {
        return HAL_ERROR;
    }

    /* Skip if flushing */
    if (hlogger->is_flushing) {
        return HAL_BUSY;
    }

    /* Check if buffer is nearly full */
    if (hlogger->buffer_level >= (SD_LOGGER_BUFFER_SIZE - 200)) {
        HAL_StatusTypeDef status = SD_Logger_FlushBuffer(hlogger);

        if (status != HAL_OK && status != HAL_BUSY) {
            /* Flush failed - try once more */
            HAL_Delay(10);
            status = SD_Logger_FlushBuffer(hlogger);

            if (status != HAL_OK && status != HAL_BUSY) {
                Debug_Printf("SD: Multiple flush failures, stopping\r\n");
                SD_Logger_StopLogging(hlogger);
                return HAL_ERROR;
            }
        }

        /* If buffer still too full after flush, drop this sample */
        if (hlogger->buffer_level >= (SD_LOGGER_BUFFER_SIZE - 200)) {
            Debug_Printf("SD: Buffer full, dropping sample\r\n");
            return HAL_BUSY;
        }
    }

    /* Format CSV line: Date,Time,State,Sample,Alpha,Alpha_Dot,Theta,Theta_Dot */
    int len = sprintf((char*)&hlogger->buffer[hlogger->buffer_level],
                     "%04d-%02d-%02d,%02d:%02d:%02d,%s,%lu,%.6f,%.6f,%.6f,%.6f\r\n",
                     2000 + date->Year,        // Full year (2025)
                     date->Month,              // Month (1-12)
                     date->Date,               // Day (1-31)
                     time->Hours,              // Hours (0-23)
                     time->Minutes,            // Minutes (0-59)
                     time->Seconds,            // Seconds (0-59)
                     SD_Logger_StateToString(state),  // State string
                     hlogger->sample_count,    // Sample number
                     alpha,                    // Pendulum angle (rad)
                     alpha_dot,                // Pendulum angular velocity (rad/s)
                     theta,                    // Arm angle (rad)
                     theta_dot);               // Arm angular velocity (rad/s)

    if (len > 0 && len < 200) {
        hlogger->buffer_level += len;
        hlogger->sample_count++;
        return HAL_OK;
    } else {
        Debug_Printf("SD: Format error (len=%d)\r\n", len);
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef SD_Logger_WriteData(SD_Logger_Handle_t* hlogger,
                                      float alpha, float alpha_dot,
                                      float theta, float theta_dot)
{
    /* Legacy function - use HAL_GetTick() as timestamp */
    RTC_TimeTypeDef dummy_time = {0};
    RTC_DateTypeDef dummy_date = {0};

    uint32_t tick = HAL_GetTick();
    dummy_time.Hours = (tick / 3600000) % 24;
    dummy_time.Minutes = (tick / 60000) % 60;
    dummy_time.Seconds = (tick / 1000) % 60;

    dummy_date.Year = 0;
    dummy_date.Month = 1;
    dummy_date.Date = 1;

    return SD_Logger_WriteDataWithState(hlogger,
                                       &dummy_time,
                                       &dummy_date,
                                       STATE_WAIT_BUTTON,
                                       alpha, alpha_dot,
                                       theta, theta_dot);
}

HAL_StatusTypeDef SD_Logger_FlushBuffer(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger || !hlogger->is_logging) {
        return HAL_OK;
    }

    if (hlogger->buffer_level == 0) {
        return HAL_OK;
    }

    /* Set flushing flag to prevent concurrent writes */
    hlogger->is_flushing = 1;

    UINT bytes_written = 0;
    FRESULT res = f_write(&hlogger->file,
                         hlogger->buffer,
                         hlogger->buffer_level,
                         &bytes_written);

    if (res != FR_OK) {
        Debug_Printf("SD_Flush: f_write ERROR %d\r\n", res);

        /* Attempt recovery with f_sync */
        Debug_Printf("SD_Flush: Attempting recovery...\r\n");
        res = f_sync(&hlogger->file);

        if (res != FR_OK) {
            Debug_Printf("SD_Flush: Recovery FAILED (error %d)\r\n", res);
            hlogger->is_flushing = 0;
            return HAL_ERROR;
        }

        /* Retry write after sync */
        res = f_write(&hlogger->file,
                     hlogger->buffer,
                     hlogger->buffer_level,
                     &bytes_written);
        if (res != FR_OK) {
            Debug_Printf("SD_Flush: Retry FAILED (error %d)\r\n", res);
            hlogger->is_flushing = 0;
            return HAL_ERROR;
        }
    }

    /* Check for partial write */
    if (bytes_written < hlogger->buffer_level) {
        Debug_Printf("SD_Flush: Partial write (%lu/%lu bytes)\r\n",
                    bytes_written, hlogger->buffer_level);

        /* Shift remaining data to start of buffer */
        uint32_t remaining = hlogger->buffer_level - bytes_written;
        memmove(hlogger->buffer,
               &hlogger->buffer[bytes_written],
               remaining);
        hlogger->buffer_level = remaining;

        hlogger->is_flushing = 0;
        return HAL_BUSY;  // Partial write, try again later
    }

    /* Success - clear buffer */
    hlogger->buffer_level = 0;
    hlogger->is_flushing = 0;

    /* Periodic f_sync to ensure data is written to card */
    static uint32_t sync_counter = 0;
    if (++sync_counter >= 10) {  // Sync every 10 flushes
        sync_counter = 0;
        f_sync(&hlogger->file);
    }

    return HAL_OK;
}

uint8_t SD_Logger_ShouldFlush(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger || !hlogger->is_logging) {
        return 0;
    }

    /* Flush when buffer is 75% full */
    return (hlogger->buffer_level >= (SD_LOGGER_BUFFER_SIZE * 3 / 4));
}

uint8_t SD_Logger_IsLogging(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger) {
        return 0;
    }
    return hlogger->is_logging;
}

uint8_t SD_Logger_IsInitialized(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger) {
        return 0;
    }
    return hlogger->is_initialized;
}

const char* SD_Logger_GetFilename(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger) {
        return "";
    }
    return hlogger->filename;
}

uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger) {
        return 0;
    }
    return hlogger->sample_count;
}

uint32_t SD_Logger_GetBufferLevel(SD_Logger_Handle_t* hlogger)
{
    if (!hlogger) {
        return 0;
    }
    return hlogger->buffer_level;
}
