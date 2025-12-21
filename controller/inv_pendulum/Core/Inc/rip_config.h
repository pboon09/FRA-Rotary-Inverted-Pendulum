#ifndef INC_RIP_CONFIG_H
#define INC_RIP_CONFIG_H

#include "main.h"
#include "QEI.h"
#include "PWM.h"
#include "Cytron_MDXX.h"
#include "energy_swingup.h"
#include "FIR.h"
#include "lqr.h"
#include "MotorKalman.h"
#include "led_matrix.h"
#include "hc05.h"
#include "sd_logger.h"
#include "sd_spi.h"
#include "lcd_display.h"

extern TIM_HandleTypeDef htim2;
#define CONTROL_TIM &htim2

extern TIM_HandleTypeDef htim1;
extern MDXX motor;
#define MOTOR1_TIM &htim1
#define MOTOR1_TIM_CH TIM_CHANNEL_1
#define MOTOR1_GPIOx GPIOA
#define MOTOR1_GPIO_Pin GPIO_PIN_9

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern QEI motor_encoder;
extern QEI pendulum_encoder;
#define ENC_TIM1 &htim3
#define ENC_TIM2 &htim4
#define ENC_PPR 8192.0
#define ENC_FREQ 1000
#define MOTOR_RATIO 1.0

extern EnergyCtrl swingup;
#define PENDULUM_MASS 0.062
#define PENDULUM_LENGTH 55.75e-3
#define PENDULUM_INERTIA 130015.24e-9
#define GRAVITY 9.812
#define ENERYGY_GAIN 1.5

extern FIR alpha_dot_filter;
extern FIR theta_dot_filter;
#define TAPS 31.0
#define CUTOFF 15.0
#define SAMPLING_RATE 1000.0

extern LQR_Controller lqr_ctrl;
#define VOLTAGE_LIMIT 10.0
#define MOTOR_VOLTAGE_LIMIT 24.0

extern KalmanFilter motor_filter;

extern TIM_HandleTypeDef htim6;

extern LED_Matrix_Handle_t hmatrix;
extern HC05_Handle_t hc05;
extern SD_Logger_Handle_t sd_logger;
extern LCD_Handle_t hlcd;

#define LCD_UPDATE_DIVIDER  500

extern UART_HandleTypeDef huart3;

//typedef enum {
//    STATE_WAIT_BUTTON,
//    STATE_KICK,
//    STATE_SWINGUP,
//    STATE_LQR,
//    STATE_EMERGENCY
//} PendulumState;

extern volatile uint8_t led_matrix_enabled;
extern volatile uint8_t sd_logger_enabled;
extern volatile uint8_t lcd_display_enabled;
extern volatile uint8_t bluetooth_enabled;
extern volatile uint8_t logging_enabled;


void config_begin(void);
void config_begin_communication(void);
void HC05_CommandHandler(char *command);

#endif /* INC_RIP_CONFIG_H */
