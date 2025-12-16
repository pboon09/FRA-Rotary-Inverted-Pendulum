/*
 * sd_logger.h - Header file for SD card logging with RTC and state
 */

#ifndef SD_LOGGER_H_
#define SD_LOGGER_H_

#include "stm32g4xx_hal.h"
#include "app_fatfs.h"

/* Duplicate PendulumState enum here to avoid circular dependency */
typedef enum {
    STATE_WAIT_BUTTON,
    STATE_KICK,
    STATE_SWINGUP,
    STATE_LQR,
    STATE_EMERGENCY
} PendulumState;

/* Buffer size for SD card writes */
#define SD_LOGGER_BUFFER_SIZE 4096

/* SD Logger handle structure */
typedef struct {
    FIL file;                               /* FatFS file object */
    uint8_t buffer[SD_LOGGER_BUFFER_SIZE];  /* Data buffer */
    uint32_t buffer_level;                   /* Current buffer usage */
    uint32_t sample_count;                   /* Total samples logged */
    uint8_t is_logging;                      /* Logging active flag */
    uint8_t is_initialized;                  /* Initialization flag */
    uint8_t is_flushing;                     /* Flush in progress flag */
    char filename[32];                       /* Current log filename */
} SD_Logger_Handle_t;

/* ============================================
 * Initialization Functions
 * ============================================ */

/**
 * @brief Initialize SD logger
 * @param hlogger: Pointer to SD logger handle
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SD_Logger_Init(SD_Logger_Handle_t* hlogger);

/**
 * @brief Start logging to a new CSV file
 * @param hlogger: Pointer to SD logger handle
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SD_Logger_StartLogging(SD_Logger_Handle_t* hlogger);

/**
 * @brief Stop logging and close file
 * @param hlogger: Pointer to SD logger handle
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SD_Logger_StopLogging(SD_Logger_Handle_t* hlogger);

/* ============================================
 * Data Writing Functions
 * ============================================ */

/**
 * @brief Write data sample with RTC timestamp and controller state
 * @param hlogger: Pointer to SD logger handle
 * @param time: Pointer to RTC time structure
 * @param date: Pointer to RTC date structure
 * @param state: Current controller state
 * @param alpha: Pendulum angle (rad)
 * @param alpha_dot: Pendulum angular velocity (rad/s)
 * @param theta: Arm angle (rad)
 * @param theta_dot: Arm angular velocity (rad/s)
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SD_Logger_WriteDataWithState(SD_Logger_Handle_t* hlogger,
                                               RTC_TimeTypeDef* time,
                                               RTC_DateTypeDef* date,
                                               PendulumState state,
                                               float alpha, float alpha_dot,
                                               float theta, float theta_dot);

/**
 * @brief Write data sample (legacy function without RTC)
 * @param hlogger: Pointer to SD logger handle
 * @param alpha: Pendulum angle (rad)
 * @param alpha_dot: Pendulum angular velocity (rad/s)
 * @param theta: Arm angle (rad)
 * @param theta_dot: Arm angular velocity (rad/s)
 * @return HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SD_Logger_WriteData(SD_Logger_Handle_t* hlogger,
                                      float alpha, float alpha_dot,
                                      float theta, float theta_dot);

/**
 * @brief Flush buffer to SD card
 * @param hlogger: Pointer to SD logger handle
 * @return HAL_OK if successful, HAL_BUSY if partial write, HAL_ERROR on failure
 */
HAL_StatusTypeDef SD_Logger_FlushBuffer(SD_Logger_Handle_t* hlogger);

/* ============================================
 * Query Functions
 * ============================================ */

/**
 * @brief Check if buffer should be flushed (75% full)
 * @param hlogger: Pointer to SD logger handle
 * @return 1 if should flush, 0 otherwise
 */
uint8_t SD_Logger_ShouldFlush(SD_Logger_Handle_t* hlogger);

/**
 * @brief Check if currently logging
 * @param hlogger: Pointer to SD logger handle
 * @return 1 if logging, 0 otherwise
 */
uint8_t SD_Logger_IsLogging(SD_Logger_Handle_t* hlogger);

/**
 * @brief Check if initialized
 * @param hlogger: Pointer to SD logger handle
 * @return 1 if initialized, 0 otherwise
 */
uint8_t SD_Logger_IsInitialized(SD_Logger_Handle_t* hlogger);

/**
 * @brief Get current log filename
 * @param hlogger: Pointer to SD logger handle
 * @return Pointer to filename string
 */
const char* SD_Logger_GetFilename(SD_Logger_Handle_t* hlogger);

/**
 * @brief Get total sample count
 * @param hlogger: Pointer to SD logger handle
 * @return Number of samples logged
 */
uint32_t SD_Logger_GetSampleCount(SD_Logger_Handle_t* hlogger);

/**
 * @brief Get current buffer level
 * @param hlogger: Pointer to SD logger handle
 * @return Number of bytes in buffer
 */
uint32_t SD_Logger_GetBufferLevel(SD_Logger_Handle_t* hlogger);

/* ============================================
 * External Functions
 * ============================================ */

/**
 * @brief Debug printf function (defined in main.c)
 */
extern void Debug_Printf(const char* format, ...);

/**
 * @brief RTC handle (defined in rtc.c or main.c)
 */
extern RTC_HandleTypeDef hrtc;

#endif /* SD_LOGGER_H_ */
