#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include "main.h"
#include "ff.h"

/* SD Logger Handle */
typedef struct {
    FIL file;
    char filename[32];
    uint32_t sample_count;
    uint8_t is_open;
} SD_Logger_Handle_t;

/* Function prototypes */
HAL_StatusTypeDef SD_Logger_Init(SD_Logger_Handle_t *hlogger);
HAL_StatusTypeDef SD_Logger_StartLogging(SD_Logger_Handle_t *hlogger);
HAL_StatusTypeDef SD_Logger_StopLogging(SD_Logger_Handle_t *hlogger);
HAL_StatusTypeDef SD_Logger_WriteData(SD_Logger_Handle_t *hlogger,
                                       float alpha, float alpha_dot,
                                       float theta, float theta_dot);

uint8_t SD_Logger_IsLogging(SD_Logger_Handle_t *hlogger);
uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t *hlogger);
char* SD_Logger_GetFilename(SD_Logger_Handle_t *hlogger);
void SD_Logger_PrintStatus(void);

#endif /* SD_LOGGER_H */
