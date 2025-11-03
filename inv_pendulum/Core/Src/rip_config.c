#include <rip_config.h>

MDXX motor;

QEI motor_encoder;
QEI pendulum_encoder;

FIR alpha_dot_filter;
FIR theta_dot_filter;

EnergyCtrl swingup;

LQR_Controller lqr_ctrl;

float32_t K_matlab[4] = { -3.2571f, 27.0816f, -2.0854f, 1.9780f };

void config_begin() {
	QEI_init(&motor_encoder, ENC_TIM1, ENC_PPR, ENC_FREQ, MOTOR_RATIO);
	QEI_init(&pendulum_encoder, ENC_TIM2, ENC_PPR, ENC_FREQ, MOTOR_RATIO);

	MDXX_GPIO_init(&motor, MOTOR1_TIM, MOTOR1_TIM_CH, MOTOR1_GPIOx,
	MOTOR1_GPIO_Pin);
	MDXX_set_range(&motor, 2000, 0);

	FIR_init(&alpha_dot_filter, TAPS, CUTOFF, SAMPLING_RATE);
	FIR_init(&theta_dot_filter, TAPS, CUTOFF, SAMPLING_RATE);

	EnergyCtrl_Init(&swingup, PENDULUM_MASS, PENDULUM_LENGTH, PENDULUM_INERTIA,
	GRAVITY, ENERYGY_GAIN);

	LQR_Init(&lqr_ctrl, K_matlab, VOLTAGE_LIMIT);

	HAL_TIM_Base_Start_IT(CONTROL_TIM);
}
