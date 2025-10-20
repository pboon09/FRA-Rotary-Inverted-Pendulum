#include <config.h>

MDXX motor1;

QEI encoder1;
QEI encoder2;

PID_CONTROLLER pid1;
PID_CONTROLLER pid2;

void config_begin() {

	QEI_init(&encoder1, ENC_TIM1, ENC_PPR, ENC_FREQ, MOTOR_RATIO);
	QEI_init(&encoder2, ENC_TIM2, ENC_PPR, ENC_FREQ, MOTOR_RATIO);

	MDXX_GPIO_init(&motor1, MOTOR1_TIM, MOTOR1_TIM_CH, MOTOR1_GPIOx, MOTOR1_GPIO_Pin);
	MDXX_set_range(&motor1, 2000, 0);

	PID_CONTROLLER_Init(&pid1, 10000, 500, 0, 65535);

	HAL_TIM_Base_Start_IT(CONTROL_TIM);
}
