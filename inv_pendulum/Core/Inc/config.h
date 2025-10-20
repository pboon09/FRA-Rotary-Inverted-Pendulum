#ifndef INC_TRANSPORTER_CONFIG_H_
#define INC_TRANSPORTER_CONFIG_H_

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include <string.h>
#include "math.h"
#include "QEI.h"
#include "PWM.h"
#include "Cytron_MDXX.h"
#include "Controller.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

extern TIM_HandleTypeDef htim2; //For Control Loop
#define CONTROL_TIM &htim2 //For Control Loop

extern PID_CONTROLLER pid1;

// Motor 1
extern TIM_HandleTypeDef htim1;
extern MDXX motor1;
#define MOTOR1_TIM &htim1
#define MOTOR1_TIM_CH TIM_CHANNEL_1
#define MOTOR1_GPIOx GPIOA
#define MOTOR1_GPIO_Pin GPIO_PIN_9

// Encoder
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern QEI encoder1;
extern QEI encoder2;
#define ENC_TIM1 &htim3
#define ENC_TIM2 &htim4
#define ENC_PPR 8192.0
#define ENC_FREQ 1000
#define MOTOR_RATIO 1.0

void config_begin();

#endif /* INC_TRANSPORTER_CONFIG_H_ */
