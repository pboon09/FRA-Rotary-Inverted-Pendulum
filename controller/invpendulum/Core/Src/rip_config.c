#include "rip_config.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
    Debug_Printf("\r\n========== System Initialization ==========\r\n");

    // Check priorities
    uint32_t systick_pri = NVIC_GetPriority(SysTick_IRQn);
    uint32_t tim2_pri = NVIC_GetPriority(TIM2_IRQn);

    Debug_Printf("SysTick Priority: %lu\r\n", systick_pri);
    Debug_Printf("TIM2 Priority: %lu\r\n", tim2_pri);

    if (tim2_pri <= systick_pri) {
        Debug_Printf("❌ ERROR: TIM2 must have lower priority than SysTick!\r\n");
        Error_Handler();
    } else {
        Debug_Printf("✅ Priority configuration OK\r\n");
    }


    /* Start TIM1 for motor PWM (also needed by LCD library) */
    Debug_Printf("Starting TIM1...\r\n");
    HAL_TIM_Base_Start(&htim1);
    HAL_Delay(100);

    /* Initialize encoders (required) */
    Debug_Printf("Initializing encoders...\r\n");
    QEI_init(&motor_encoder, ENC_TIM1, ENC_PPR, ENC_FREQ, MOTOR_RATIO);
    QEI_init(&pendulum_encoder, ENC_TIM2, ENC_PPR, ENC_FREQ, MOTOR_RATIO);

    /* Initialize motor driver (required) */
    Debug_Printf("Initializing motor driver...\r\n");
    MDXX_GPIO_init(&motor, MOTOR1_TIM, MOTOR1_TIM_CH, MOTOR1_GPIOx, MOTOR1_GPIO_Pin);
    MDXX_set_range(&motor, 2000, 0);

    /* Initialize filters (required) */
    Debug_Printf("Initializing filters...\r\n");
    FIR_init(&alpha_dot_filter, TAPS, CUTOFF, SAMPLING_RATE);
    FIR_init(&theta_dot_filter, TAPS, CUTOFF, SAMPLING_RATE);

    /* Initialize swing-up controller (required) */
    Debug_Printf("Initializing controllers...\r\n");
    EnergyCtrl_Init(&swingup, PENDULUM_MASS, PENDULUM_LENGTH, PENDULUM_INERTIA, GRAVITY, ENERYGY_GAIN);

    /* Initialize Kalman filter and LQR controller (required) */
    Debug_Printf("Initializing Kalman filter...\r\n");
    kf_init(&motor_filter, A, B, 1.0f, 0.0005f);
    Debug_Printf("Initializing LQR...\r\n");
    LQR_Init(&lqr_ctrl, K_matlab, VOLTAGE_LIMIT);

    /* Initialize LED matrix (optional) */
//    Debug_Printf("\r\n--- Optional Peripherals ---\r\n");
//    if (LED_Matrix_Init(&hmatrix) == HAL_OK) {
//        led_matrix_enabled = 1;
//        Debug_Printf("LED Matrix: ENABLED\r\n");
//    } else {
//        led_matrix_enabled = 0;
//        Debug_Printf("LED Matrix: DISABLED\r\n");
//    }

    /* Initialize SD card logger (optional) */
//    if (SD_Logger_Init(&sd_logger) == HAL_OK) {
//        sd_logger_enabled = 1;
//        Debug_Printf("SD Logger: ENABLED\r\n");
//    } else {
//        sd_logger_enabled = 0;
//        Debug_Printf("SD Logger: DISABLED\r\n");
//    }
//
//    /* Initialize LCD display (optional) */
//    if (LCD_Display_Init(&hlcd) == HAL_OK) {
//        lcd_display_enabled = 1;
//        LCD_Display_SetUpdateRate(&hlcd, LCD_UPDATE_DIVIDER);
////        LCD_Test_Display();
//        Debug_Printf("LCD Display: ENABLED\r\n");
//    } else {
//        lcd_display_enabled = 0;
//        Debug_Printf("LCD Display: DISABLED\r\n");
//    }

//    Debug_Printf("\r\n========== Initialization Complete ==========\r\n");
//    Debug_Printf("LED Matrix: %s\r\n", led_matrix_enabled ? "ON" : "OFF");
//    Debug_Printf("SD Logger: %s\r\n", sd_logger_enabled ? "ON" : "OFF");
//    Debug_Printf("LCD Display: %s\r\n", lcd_display_enabled ? "ON" : "OFF");
//    Debug_Printf("===========================================\r\n\r\n");

    /* Start control timer */
    HAL_TIM_Base_Start_IT(CONTROL_TIM);
}

void config_begin_communication() {
    if (HC05_Init(&hc05, &huart3, 9600) == HAL_OK) {
        HC05_SetCommandCallback(&hc05, HC05_CommandHandler);
        bluetooth_enabled = 1;

        /* Send welcome messages */
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

    /* Echo received command */
    HC05_SendFormatted(&hc05, "[RX]: %s\r\n", command);

    /* Convert to uppercase for comparison */
    for (char *p = command; *p; p++) {
        *p = toupper(*p);
    }

    /* Get access to pendulum state from main.c */
    extern PendulumState state;
    extern int kick_counter;

    /* Process commands */
    if (strcmp(command, "START") == 0) {
        if (state == STATE_WAIT_BUTTON || state == STATE_EMERGENCY) {
            kick_counter = 0;
            state = STATE_KICK;
            QEI_reset(&pendulum_encoder);
            QEI_reset(&motor_encoder);
            kf_clear(&motor_filter);
            HC05_Send(&hc05, "[TX]: Starting swing-up sequence\r\n");
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

        if (sd_logger_enabled && SD_Logger_IsOpen(&sd_logger)) {
            SD_Logger_Close(&sd_logger);
        }

        HC05_Send(&hc05, "[TX]: System reset\r\n");
    }
    else if (strcmp(command, "EMERGENCY") == 0) {
        state = STATE_EMERGENCY;
        MDXX_set_range(&motor, 2000, 0);
        HC05_Send(&hc05, "[TX]: EMERGENCY STOP activated\r\n");
    }
    else if (strcmp(command, "LOGSTART") == 0) {
        if (!sd_logger_enabled) {
            HC05_Send(&hc05, "[TX]: ERR - SD logger not available\r\n");
            return;
        }

        if (!SD_Logger_IsOpen(&sd_logger)) {
            char filename[32];
            static uint8_t log_num = 0;
            snprintf(filename, sizeof(filename), "log_%03d.csv", log_num++);

            if (SD_Logger_CreateFile(&sd_logger, filename)) {
                SD_Logger_WriteHeader(&sd_logger);
                logging_enabled = 1;
                HC05_SendFormatted(&hc05, "[TX]: Logging started (%s)\r\n", filename);
            } else {
                HC05_Send(&hc05, "[TX]: ERR - Failed to create log file\r\n");
            }
        } else {
            logging_enabled = 1;
            HC05_Send(&hc05, "[TX]: Logging resumed\r\n");
        }
    }
    else if (strcmp(command, "LOGSTOP") == 0) {
        if (logging_enabled) {
            logging_enabled = 0;
            if (sd_logger_enabled && SD_Logger_IsOpen(&sd_logger)) {
                SD_Logger_Sync(&sd_logger);
                HC05_SendFormatted(&hc05, "[TX]: Logging stopped (%lu samples)\r\n",
                                 SD_Logger_GetSampleCount(&sd_logger));
                SD_Logger_Close(&sd_logger);
            }
        } else {
            HC05_Send(&hc05, "[TX]: Logging already stopped\r\n");
        }
    }
    else if (strcmp(command, "LEDON") == 0) {
        if (LED_Matrix_Init(&hmatrix) == HAL_OK) {
            led_matrix_enabled = 1;
            HC05_Send(&hc05, "[TX]: LED display enabled\r\n");
        } else {
            HC05_Send(&hc05, "[TX]: ERR - LED matrix not available\r\n");
        }
    }
    else if (strcmp(command, "LEDOFF") == 0) {
        if (led_matrix_enabled) {
            led_matrix_enabled = 0;
            LED_Matrix_ClearBuffer(&hmatrix);
            HC05_Send(&hc05, "[TX]: LED display disabled\r\n");
        } else {
            HC05_Send(&hc05, "[TX]: ERR - LED matrix already disabled\r\n");
        }
    }
//    else if (strcmp(command, "LCDON") == 0) {
//        if (LCD_Display_Init(&hlcd) == HAL_OK) {
//            lcd_display_enabled = 1;
//            LCD_Display_SetUpdateRate(&hlcd, LCD_UPDATE_DIVIDER);
//            HC05_Send(&hc05, "[TX]: LCD display enabled\r\n");
//        } else {
//            HC05_Send(&hc05, "[TX]: ERR - LCD not available\r\n");
//        }
//    }
//    else if (strcmp(command, "LCDOFF") == 0) {
//        if (lcd_display_enabled) {
//            lcd_display_enabled = 0;
//            LCD_Display_Clear(&hlcd);
//            HC05_Send(&hc05, "[TX]: LCD display disabled\r\n");
//        } else {
//            HC05_Send(&hc05, "[TX]: ERR - LCD already disabled\r\n");
//        }
//    }
//    else if (strcmp(command, "LCDFAST") == 0) {
//        if (LCD_Display_IsInitialized(&hlcd)) {
//            LCD_Display_SetUpdateRate(&hlcd, 25);
//            HC05_Send(&hc05, "[TX]: LCD update rate: FAST (40 Hz)\r\n");
//        } else {
//            HC05_Send(&hc05, "[TX]: ERR - LCD not initialized\r\n");
//        }
//    }
//    else if (strcmp(command, "LCDSLOW") == 0) {
//        if (LCD_Display_IsInitialized(&hlcd)) {
//            LCD_Display_SetUpdateRate(&hlcd, 100);
//            HC05_Send(&hc05, "[TX]: LCD update rate: SLOW (10 Hz)\r\n");
//        } else {
//            HC05_Send(&hc05, "[TX]: ERR - LCD not initialized\r\n");
//        }
//    }
    else if (strcmp(command, "STATUS") == 0) {
        const char *state_str;
        switch (state) {
            case STATE_WAIT_BUTTON: state_str = "WAIT_BUTTON"; break;
            case STATE_KICK:        state_str = "KICK"; break;
            case STATE_SWINGUP:     state_str = "SWINGUP"; break;
            case STATE_LQR:         state_str = "LQR (BALANCING)"; break;
            case STATE_EMERGENCY:   state_str = "EMERGENCY"; break;
            default:                state_str = "UNKNOWN"; break;
        }

        HC05_SendFormatted(&hc05, "[TX]: Status:\r\n");
        HC05_SendFormatted(&hc05, "  State: %s\r\n", state_str);
        HC05_SendFormatted(&hc05, "  LED Matrix: %s\r\n", led_matrix_enabled ? "OK" : "NOT DETECTED");
        HC05_SendFormatted(&hc05, "  SD Logger: %s\r\n", sd_logger_enabled ? "OK" : "NOT DETECTED");
//        HC05_SendFormatted(&hc05, "  LCD Display: %s\r\n",
//                         LCD_Display_IsInitialized(&hlcd) ?
//                         (lcd_display_enabled ? "ON" : "OFF") : "NOT DETECTED");
        HC05_SendFormatted(&hc05, "  Bluetooth: %s\r\n", bluetooth_enabled ? "OK" : "FAIL");
        HC05_SendFormatted(&hc05, "  Logging: %s\r\n", logging_enabled ? "ON" : "OFF");

        if (sd_logger_enabled && SD_Logger_IsOpen(&sd_logger)) {
            HC05_SendFormatted(&hc05, "  Log file: %s\r\n", sd_logger.filename);
            HC05_SendFormatted(&hc05, "  Samples: %lu\r\n",
                             SD_Logger_GetSampleCount(&sd_logger));
        }
    }
    else if (strcmp(command, "HELP") == 0) {
        HC05_Send(&hc05, "[TX]: Available commands:\r\n");
        HC05_Send(&hc05, "  START     - Start swing-up sequence\r\n");
        HC05_Send(&hc05, "  STOP      - Stop system\r\n");
        HC05_Send(&hc05, "  RESET     - Full system reset\r\n");
        HC05_Send(&hc05, "  EMERGENCY - Emergency stop\r\n");
        HC05_Send(&hc05, "  LOGSTART  - Start SD card logging\r\n");
        HC05_Send(&hc05, "  LOGSTOP   - Stop SD card logging\r\n");
        HC05_Send(&hc05, "  LEDON     - Enable LED display\r\n");
        HC05_Send(&hc05, "  LEDOFF    - Disable LED display\r\n");
        HC05_Send(&hc05, "  LCDON     - Enable LCD display\r\n");
        HC05_Send(&hc05, "  LCDOFF    - Disable LCD display\r\n");
        HC05_Send(&hc05, "  LCDFAST   - LCD fast refresh (40 Hz)\r\n");
        HC05_Send(&hc05, "  LCDSLOW   - LCD slow refresh (10 Hz)\r\n");
        HC05_Send(&hc05, "  STATUS    - Show system status\r\n");
        HC05_Send(&hc05, "  HELP      - Show this help\r\n");
    }
    else {
        HC05_Send(&hc05, "[TX]: ERR - Unknown command (type HELP)\r\n");
    }

    /* Send prompt */
    HC05_Send(&hc05, "\r\n> ");
}
