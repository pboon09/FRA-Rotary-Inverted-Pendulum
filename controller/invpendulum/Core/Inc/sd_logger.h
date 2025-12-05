#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include "main.h"
#include "ff.h"

typedef struct {
    uint8_t is_open;
    uint32_t sample_count;
    char filename[32];
} SD_Logger_Handle_t;

/* Initialization */
void SD_Logger_Init(SD_Logger_Handle_t *hlogger);
uint8_t SD_Logger_CreateFile(SD_Logger_Handle_t *hlogger, const char *filename);
void SD_Logger_Close(SD_Logger_Handle_t *hlogger);

/* Logging functions */
uint8_t SD_Logger_WriteHeader(SD_Logger_Handle_t *hlogger);
uint8_t SD_Logger_WriteData(SD_Logger_Handle_t *hlogger, float alpha, float alpha_dot, float theta, float theta_dot);
void SD_Logger_Sync(SD_Logger_Handle_t *hlogger);

/* Status */
uint8_t SD_Logger_IsOpen(SD_Logger_Handle_t *hlogger);
uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t *hlogger);
uint8_t SD_Logger_IsFatFSInitialized(void);
void SD_Logger_PrintStatus(void);

/* General file operations (from logger.c) */
uint8_t SD_Logger_WriteString(const char *filename, const char *data);
uint8_t SD_Logger_WriteLine(const char *filename, const char *data);
uint8_t SD_Logger_WriteData_Binary(const char *filename, const uint8_t *data, uint32_t len);

#endif /* SD_LOGGER_H */
