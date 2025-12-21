#ifndef SD_LOGGER_H_
#define SD_LOGGER_H_

#include "stm32g4xx_hal.h"
#include "app_fatfs.h"


typedef enum {
    STATE_WAIT_BUTTON,
    STATE_KICK,
    STATE_SWINGUP,
    STATE_LQR,
    STATE_EMERGENCY
} PendulumState;

#define SD_LOGGER_BUFFER_SIZE 4096

typedef struct {
    FIL file;
    uint8_t buffer[SD_LOGGER_BUFFER_SIZE];
    uint32_t buffer_level;
    uint32_t sample_count;
    uint8_t is_logging;
    uint8_t is_initialized;
    uint8_t is_flushing;
    char filename[32];
} SD_Logger_Handle_t;

HAL_StatusTypeDef SD_Logger_Init(SD_Logger_Handle_t* hlogger);
HAL_StatusTypeDef SD_Logger_StartLogging(SD_Logger_Handle_t* hlogger);
HAL_StatusTypeDef SD_Logger_StopLogging(SD_Logger_Handle_t* hlogger);
HAL_StatusTypeDef SD_Logger_WriteDataWithState(SD_Logger_Handle_t* hlogger,
                                               RTC_TimeTypeDef* time,
                                               RTC_DateTypeDef* date,
                                               PendulumState state,
                                               float alpha, float alpha_dot,
                                               float theta, float theta_dot);
HAL_StatusTypeDef SD_Logger_WriteData(SD_Logger_Handle_t* hlogger,
                                      float alpha, float alpha_dot,
                                      float theta, float theta_dot);
HAL_StatusTypeDef SD_Logger_FlushBuffer(SD_Logger_Handle_t* hlogger);

uint8_t SD_Logger_ShouldFlush(SD_Logger_Handle_t* hlogger);
uint8_t SD_Logger_IsLogging(SD_Logger_Handle_t* hlogger);
uint8_t SD_Logger_IsInitialized(SD_Logger_Handle_t* hlogger);
const char* SD_Logger_GetFilename(SD_Logger_Handle_t* hlogger);
uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t* hlogger);
uint32_t SD_Logger_GetBufferLevel(SD_Logger_Handle_t* hlogger);
extern void Debug_Printf(const char* format, ...);
extern RTC_HandleTypeDef hrtc;

#endif /* SD_LOGGER_H_ */
