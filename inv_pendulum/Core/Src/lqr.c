/*
 * lqr.c
 *
 *  Created on: Nov 2, 2025
 *      Author: pboon
 */

#include "lqr.h"

void LQR_Init(LQR_Controller *ctrl, float32_t *K_values, float32_t u_limit)
{
    for (int i = 0; i < 4; i++)
        ctrl->K_data[i] = K_values[i];

    ctrl->u_limit = u_limit;

    arm_mat_init_f32(&ctrl->K, 1, 4, ctrl->K_data);
    arm_mat_init_f32(&ctrl->x, 4, 1, ctrl->x_data);
    arm_mat_init_f32(&ctrl->u, 1, 1, ctrl->u_data);

    for (int i = 0; i < 4; i++)
        ctrl->x_data[i] = 0.0f;
    ctrl->u_data[0] = 0.0f;
}

void LQR_SetState(LQR_Controller *ctrl, float32_t theta, float32_t alpha, float32_t theta_dot, float32_t alpha_dot)
{
    ctrl->x_data[0] = theta;
    ctrl->x_data[1] = alpha;
    ctrl->x_data[2] = theta_dot;
    ctrl->x_data[3] = alpha_dot;
}

float32_t LQR_Update(LQR_Controller *ctrl)
{
    arm_mat_mult_f32(&ctrl->K, &ctrl->x, &ctrl->u);
    ctrl->u_data[0] = -ctrl->u_data[0];

    if (ctrl->u_data[0] > ctrl->u_limit)
        ctrl->u_data[0] = ctrl->u_limit;
    else if (ctrl->u_data[0] < -ctrl->u_limit)
        ctrl->u_data[0] = -ctrl->u_limit;

    return ctrl->u_data[0];
}
