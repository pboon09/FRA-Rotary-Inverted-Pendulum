/*
 * lqr.h
 *
 *  Created on: Nov 2, 2025
 *      Author: pboon
 */

#ifndef INC_LQR_H_
#define INC_LQR_H_

#include "main.h"
#include "arm_math.h"

typedef struct {
	arm_matrix_instance_f32 K;
	arm_matrix_instance_f32 x;
	arm_matrix_instance_f32 u;
	float32_t K_data[4];
	float32_t x_data[4];
	float32_t u_data[1];
	float32_t u_limit;
} LQR_Controller;

void LQR_Init(LQR_Controller *ctrl, float32_t *K_values, float32_t u_limit);
float32_t LQR_Update(LQR_Controller *ctrl);
void LQR_SetState(LQR_Controller *ctrl, float32_t theta, float32_t alpha,
		float32_t theta_dot, float32_t alpha_dot);

#endif /* INC_LQR_H_ */

