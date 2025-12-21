#include "rip_config.h"
#include "main.h"
#include "lcd_display.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

extern TIM_HandleTypeDef htim2;

/* LCD test function */
void LCD_Minimal_Test(void);

MDXX motor;

QEI motor_encoder;
QEI pendulum_encoder;

FIR alpha_dot_filter;
FIR theta_dot_filter;

EnergyCtrl swingup;

LQR_Controller lqr_ctrl;

KalmanFilter motor_filter;

LED_Matrix_Handle_t hmatrix;
HC05_Handle_t hc05;
SD_Logger_Handle_t sd_logger;
LCD_Handle_t hlcd;

float32_t K_matlab[4] = { -0.8197f, 11.3205f, -0.6469f, 1.1683f };

float32_t A[16] = {1.0f, 9.999812785357154e-04f, -1.149563041803406e-04f, 7.180678148697623e-06f,
                   0.0f, 0.999950617296464f,   -0.229910715302858f, 0.014322070901902f,
                   0.0f, 0.0f   ,   1.0f  , 0.0f,
                   0.0f,-0.004961131606500f, 5.718837195395508e-04f, 0.983689934032327f};

float32_t B[4] = {1.908889505894626e-07f,
                  5.718837195395508e-04f,
                  0.0f,
                  0.078991236957537f};

volatile uint8_t led_matrix_enabled = 0;
volatile uint8_t sd_logger_enabled = 0;
volatile uint8_t lcd_display_enabled = 0;
volatile uint8_t bluetooth_enabled = 0;
volatile uint8_t logging_enabled = 0;

extern void Debug_Printf(const char* format, ...);

void config_begin() {
    Debug_Printf("\r\nSystem Initialization\r\n");

    QEI_init(&motor_encoder, ENC_TIM1, ENC_PPR, ENC_FREQ, MOTOR_RATIO);
    QEI_init(&pendulum_encoder, ENC_TIM2, ENC_PPR, ENC_FREQ, MOTOR_RATIO);

    MDXX_GPIO_init(&motor, MOTOR1_TIM, MOTOR1_TIM_CH, MOTOR1_GPIOx, MOTOR1_GPIO_Pin);
    MDXX_set_range(&motor, 2000, 0);

    FIR_init(&alpha_dot_filter, TAPS, CUTOFF, SAMPLING_RATE);
    FIR_init(&theta_dot_filter, TAPS, CUTOFF, SAMPLING_RATE);
    EnergyCtrl_Init(&swingup, PENDULUM_MASS, PENDULUM_LENGTH, PENDULUM_INERTIA, GRAVITY, ENERYGY_GAIN);
    kf_init(&motor_filter, A, B, 1.0f, 0.0005f);
    LQR_Init(&lqr_ctrl, K_matlab, VOLTAGE_LIMIT);

    if (LED_Matrix_Init(&hmatrix) == HAL_OK) {
        led_matrix_enabled = 1;
        Debug_Printf("LED Matrix: ENABLED\r\n");
    } else {
        led_matrix_enabled = 0;
        Debug_Printf("LED Matrix: DISABLED\r\n");
    }

    if (SD_Logger_Init(&sd_logger) == HAL_OK) {
        sd_logger_enabled = 1;
        Debug_Printf("SD Logger: ENABLED\r\n");
    } else {
        sd_logger_enabled = 0;
        Debug_Printf("SD Logger: DISABLED\r\n");
    }

    if (LCD_Display_Init(&hlcd) == HAL_OK) {
        lcd_display_enabled = 1;
        LCD_Display_SetUpdateRate(&hlcd, LCD_UPDATE_DIVIDER);
        Debug_Printf("LCD Display: ENABLED\r\n");
    } else {
        lcd_display_enabled = 0;
        Debug_Printf("LCD Display: DISABLED\r\n");
    }

    Debug_Printf("\r\nInitialization Complete\r\n");
    Debug_Printf("LED Matrix: %s\r\n", led_matrix_enabled ? "ON" : "OFF");
    Debug_Printf("SD Logger: %s\r\n", sd_logger_enabled ? "ON" : "OFF");
    Debug_Printf("LCD Display: %s\r\n", lcd_display_enabled ? "ON" : "OFF");

    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start_IT(CONTROL_TIM);

}

void config_begin_communication() {
    if (HC05_Init(&hc05, &huart3, 9600) == HAL_OK) {
        HC05_SetCommandCallback(&hc05, HC05_CommandHandler);
        bluetooth_enabled = 1;

        const char *msg1 = "\r\n===== RIP Control System =====\r\n";
        const char *msg2 = "Type HELP for commands\r\n";
        const char *msg3 = "\r\n> ";

        HAL_UART_Transmit(&huart3, (uint8_t*)msg1, strlen(msg1), 1000);
        HAL_UART_Transmit(&huart3, (uint8_t*)msg2, strlen(msg2), 1000);
        HAL_UART_Transmit(&huart3, (uint8_t*)msg3, strlen(msg3), 1000);

        HC05_Start(&hc05);
    } else {
        bluetooth_enabled = 0;
    }
}

void HC05_CommandHandler(char *command) {
    if (!bluetooth_enabled) return;

    HC05_SendFormatted(&hc05, "[RX]: %s\r\n", command);


    for (char *p = command; *p; p++) {
        *p = toupper(*p);
    }

    extern volatile PendulumState state;
    extern int kick_counter;

    /* ========== SYSTEM CONTROL ========== */
    if (strcmp(command, "START") == 0) {
        PendulumState current_state = state;
        if (current_state == STATE_WAIT_BUTTON || current_state == STATE_EMERGENCY) {
            kick_counter = 0;
            state = STATE_KICK;
            QEI_reset(&pendulum_encoder);
            QEI_reset(&motor_encoder);
            kf_clear(&motor_filter);
            HC05_Send(&hc05, "[TX]: Starting kick sequence\r\n");
        } else {
            HC05_Send(&hc05, "[TX]: System already running\r\n");
        }
    }
    else if (strcmp(command, "STOP") == 0) {
        state = STATE_WAIT_BUTTON;
        MDXX_set_range(&motor, 2000, 0);
        HC05_Send(&hc05, "[TX]: System stopped\r\n");
    }
    else if (strcmp(command, "RESET") == 0) {
        state = STATE_WAIT_BUTTON;
        logging_enabled = 0;
        MDXX_set_range(&motor, 2000, 0);
        QEI_reset(&motor_encoder);
        QEI_reset(&pendulum_encoder);
        kf_clear(&motor_filter);

        if (sd_logger_enabled && SD_Logger_IsLogging(&sd_logger)) {
            SD_Logger_StopLogging(&sd_logger);
        }

        HC05_Send(&hc05, "[TX]: System reset\r\n");
    }
    else if (strcmp(command, "EMERGENCY") == 0) {
        state = STATE_EMERGENCY;
        MDXX_set_range(&motor, 2000, 0);
        HC05_Send(&hc05, "[TX]: EMERGENCY STOP activated\r\n");
    }

    /* ========== SD CARD LOGGING ========== */
    else if (strcmp(command, "LOGSTART") == 0) {
        if (!sd_logger_enabled) {
            HC05_Send(&hc05, "[TX]: ERR - SD card not available\r\n");
        } else if (SD_Logger_IsLogging(&sd_logger)) {
            HC05_Send(&hc05, "[TX]: ERR - Already logging\r\n");
        } else {
            if (SD_Logger_StartLogging(&sd_logger) == HAL_OK) {
                logging_enabled = 1;
                char msg[64];
                sprintf(msg, "[TX]: Log started: %s\r\n", SD_Logger_GetFilename(&sd_logger));
                HC05_Send(&hc05, msg);
            } else {
                HC05_Send(&hc05, "[TX]: ERR - Failed to start logging\r\n");
            }
        }
    }
    else if (strcmp(command, "LOGSTOP") == 0) {
        if (!sd_logger_enabled) {
            HC05_Send(&hc05, "[TX]: ERR - SD card not available\r\n");
        } else if (!SD_Logger_IsLogging(&sd_logger)) {
            HC05_Send(&hc05, "[TX]: ERR - Not logging\r\n");
        } else {
            uint32_t samples = SD_Logger_GetSampleCount(&sd_logger);
            SD_Logger_StopLogging(&sd_logger);
            logging_enabled = 0;

            char msg[64];
            sprintf(msg, "[TX]: Log stopped - %lu samples\r\n", samples);
            HC05_Send(&hc05, msg);
        }
    }

    /* ========== LED MATRIX ========== */
    else if (strcmp(command, "LEDON") == 0) {
        if (!led_matrix_enabled) {
            if (LED_Matrix_Init(&hmatrix) == HAL_OK) {
                led_matrix_enabled = 1;
                HC05_Send(&hc05, "[TX]: LED display enabled\r\n");
            } else {
                HC05_Send(&hc05, "[TX]: ERR - LED matrix not available\r\n");
            }
        } else {
            HC05_Send(&hc05, "[TX]: LED display already enabled\r\n");
        }
    }
    else if (strcmp(command, "LEDOFF") == 0) {
        if (led_matrix_enabled) {
            led_matrix_enabled = 0;
            LED_Matrix_ClearBuffer(&hmatrix);
            LED_Matrix_DisableAllRows(&hmatrix);
            HC05_Send(&hc05, "[TX]: LED display disabled\r\n");
        } else {
            HC05_Send(&hc05, "[TX]: LED display already disabled\r\n");
        }
    }

    /* ========== LCD DISPLAY ========== */
    else if (strcmp(command, "LCDON") == 0) {
        if (!lcd_display_enabled) {
            /* NO NEED TO STOP CONTROL LOOP - LCD init is safe while running */
            HC05_Send(&hc05, "[TX]: Initializing LCD...\r\n");

            if (LCD_Display_Init(&hlcd) == HAL_OK) {
                lcd_display_enabled = 1;
                LCD_Display_SetUpdateRate(&hlcd, LCD_UPDATE_DIVIDER);
                HC05_Send(&hc05, "[TX]: LCD display enabled\r\n");
            } else {
                HC05_Send(&hc05, "[TX]: ERR - LCD init failed\r\n");
            }
        } else {
            HC05_Send(&hc05, "[TX]: LCD display already enabled\r\n");
        }
    }
    else if (strcmp(command, "LCDOFF") == 0) {
        if (lcd_display_enabled) {
            lcd_display_enabled = 0;
            LCD_Display_Clear(&hlcd);
            HC05_Send(&hc05, "[TX]: LCD display disabled\r\n");
        } else {
            HC05_Send(&hc05, "[TX]: LCD display already disabled\r\n");
        }
    }

    else if (strncmp(command, "SETTIME", 7) == 0) {
        int hours, minutes, seconds;
        if (sscanf(command + 8, "%d %d %d", &hours, &minutes, &seconds) == 3) {
            if (hours >= 0 && hours < 24 &&
                minutes >= 0 && minutes < 60 &&
                seconds >= 0 && seconds < 60) {

                RTC_TimeTypeDef sTime = {0};
                sTime.Hours = hours;
                sTime.Minutes = minutes;
                sTime.Seconds = seconds;
                sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
                sTime.StoreOperation = RTC_STOREOPERATION_RESET;

                if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) {
                    char msg[64];
                    sprintf(msg, "[TX]: Time set to %02d:%02d:%02d\r\n",
                           hours, minutes, seconds);
                    HC05_Send(&hc05, msg);
                } else {
                    HC05_Send(&hc05, "[TX]: ERR - Failed to set time\r\n");
                }
            } else {
                HC05_Send(&hc05, "[TX]: ERR - Invalid time\r\n");
            }
        } else {
            HC05_Send(&hc05, "[TX]: Usage: SETTIME HH MM SS\r\n");
        }
    }
    else if (strncmp(command, "SETDATE", 7) == 0) {
        int year, month, day;
        if (sscanf(command + 8, "%d %d %d", &year, &month, &day) == 3) {
            if (year >= 2000 && year < 2100 &&
                month >= 1 && month <= 12 &&
                day >= 1 && day <= 31) {

                RTC_DateTypeDef sDate = {0};
                sDate.Year = year - 2000;
                sDate.Month = month;
                sDate.Date = day;
                sDate.WeekDay = RTC_WEEKDAY_MONDAY;

                if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK) {
                    char msg[64];
                    sprintf(msg, "[TX]: Date set to %04d-%02d-%02d\r\n",
                           year, month, day);
                    HC05_Send(&hc05, msg);
                } else {
                    HC05_Send(&hc05, "[TX]: ERR - Failed to set date\r\n");
                }
            } else {
                HC05_Send(&hc05, "[TX]: ERR - Invalid date\r\n");
            }
        } else {
            HC05_Send(&hc05, "[TX]: Usage: SETDATE YYYY MM DD\r\n");
        }
    }
    else if (strcmp(command, "GETTIME") == 0) {
        RTC_TimeTypeDef sTime;
        RTC_DateTypeDef sDate;

        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);  // Must read date after time

        char msg[128];
        sprintf(msg, "[TX]: %04d-%02d-%02d %02d:%02d:%02d\r\n",
               2000 + sDate.Year, sDate.Month, sDate.Date,
               sTime.Hours, sTime.Minutes, sTime.Seconds);
        HC05_Send(&hc05, msg);
    }

    /* ========== SYSTEM STATUS ========== */
    else if (strcmp(command, "STATUS") == 0) {
        const char *state_str;
        PendulumState current_state = state;
        switch (current_state) {
            case STATE_WAIT_BUTTON: state_str = "WAIT_BUTTON"; break;
            case STATE_KICK:        state_str = "KICK"; break;
            case STATE_SWINGUP:     state_str = "SWINGUP"; break;
            case STATE_LQR:         state_str = "LQR (BALANCING)"; break;
            case STATE_EMERGENCY:   state_str = "EMERGENCY"; break;
            default:                state_str = "UNKNOWN"; break;
        }

        /* Send all status info in ONE message to avoid buffer overflow */
        char status_msg[512];
        int len = 0;

        len += sprintf(status_msg + len, "[TX]: === SYSTEM STATUS ===\r\n");
        len += sprintf(status_msg + len, "  State: %s\n", state_str);
        len += sprintf(status_msg + len, "  LED Matrix: %s\n",
                      led_matrix_enabled ? "ENABLED" : "DISABLED");
        len += sprintf(status_msg + len, "  LCD Display: %s\n",
                      LCD_Display_IsInitialized(&hlcd) ?
                      (lcd_display_enabled ? "ENABLED" : "DISABLED") : "NOT DETECTED");
        len += sprintf(status_msg + len, "  SD Logger: %s\n",
                      sd_logger_enabled ? "READY" : "NOT DETECTED");
        len += sprintf(status_msg + len, "  Logging: %s\n",
                      logging_enabled ? "ACTIVE" : "INACTIVE");

        if (sd_logger_enabled && SD_Logger_IsLogging(&sd_logger)) {
            len += sprintf(status_msg + len, "  Log file: %s\n",
                          SD_Logger_GetFilename(&sd_logger));
            len += sprintf(status_msg + len, "  Samples: %lu\n",
                          SD_Logger_GetSampleCount(&sd_logger));
            len += sprintf(status_msg + len, "  Buffer: %lu bytes\n",
                          SD_Logger_GetBufferLevel(&sd_logger));
        }

        /* Send entire status in one go */
        HC05_Send(&hc05, status_msg);
    }

    /* ========== HELP ========== */
    else if (strcmp(command, "HELP") == 0) {
        char help_msg[570];
        int len = 0;

        len += sprintf(help_msg + len, "[TX]: === AVAILABLE COMMANDS ===\r\n");
        len += sprintf(help_msg + len, "  System Control:\n");
        len += sprintf(help_msg + len, "    START     - Start kick sequence\n");
        len += sprintf(help_msg + len, "    STOP      - Stop system\n");
        len += sprintf(help_msg + len, "    RESET     - Full system reset\n");
        len += sprintf(help_msg + len, "    EMERGENCY - Emergency stop\r\n");
        len += sprintf(help_msg + len, "  Data Logging:\n");
        len += sprintf(help_msg + len, "    LOGSTART  - Start SD logging\n");
        len += sprintf(help_msg + len, "    LOGSTOP   - Stop SD logging\r\n");
        len += sprintf(help_msg + len, "  Displays:\n");
        len += sprintf(help_msg + len, "    LEDON     - Enable LED matrix\n");
        len += sprintf(help_msg + len, "    LEDOFF    - Disable LED matrix\n");
        len += sprintf(help_msg + len, "    LCDON     - Enable LCD display\n");
        len += sprintf(help_msg + len, "    LCDOFF    - Disable LCD display\r\n");
        len += sprintf(help_msg + len, "  Real Time Clock:\n");
        len += sprintf(help_msg + len, "    SETTIME HH MM SS	- Set the system time\n");
        len += sprintf(help_msg + len, "    SETDATE YYYY MM DD	- Set the system date\n");
        len += sprintf(help_msg + len, "    GETTIME	  - Get the current time\r\n");
        len += sprintf(help_msg + len, "  Info:\n");
        len += sprintf(help_msg + len, "    STATUS    - Show system status\n");
        len += sprintf(help_msg + len, "    HELP      - Show this help");

        /* Send entire help in one go */
        HC05_Send(&hc05, help_msg);
    }

    else {
        HC05_Send(&hc05, "[TX]: ERR - Unknown command (type HELP)\r\n");
    }

    HC05_Send(&hc05, "\r\n> ");
}
