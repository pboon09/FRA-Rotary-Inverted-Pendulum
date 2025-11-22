/*
 * MotorKalman.h
 *
 *  Created on: Nov 20, 2025
 *      Author: pboon
 */

#ifndef INC_MOTORKALMAN_H_
#define INC_MOTORKALMAN_H_

#include "main.h"
#include "arm_math.h"

typedef struct {
    float32_t state[4];
    float32_t covariance[16];
    float32_t system_mat[16];
    float32_t input_mat[4];
    float32_t measurement_mat[4];
    float32_t process_noise_mat[4];
    float32_t process_noise;
    float32_t measurement_noise[1];
    float32_t sys_transpose[16];
    float32_t meas_transpose[4];
    float32_t noise_transpose[4];
    float32_t noise_product[16];
    float32_t scaled_noise[16];
    float32_t pred_state[4];
    float32_t scaled_input[4];
    float32_t innovation_cov[4];
    float32_t innovation_product[1];
    float32_t innovation_sum[1];
    float32_t gain[4];
    float32_t gain_temp[4];
    float32_t inv_innovation[1];
    float32_t predicted_meas[1];
    float32_t innovation[1];
    float32_t correction[4];
    float32_t output[1];
    float32_t identity[16];
    float32_t sensor_data[1];

    arm_matrix_instance_f32 mat_state;
    arm_matrix_instance_f32 mat_cov;
    arm_matrix_instance_f32 mat_sys;
    arm_matrix_instance_f32 mat_sys_t;
    arm_matrix_instance_f32 mat_identity;
    arm_matrix_instance_f32 mat_input;
    arm_matrix_instance_f32 mat_meas;
    arm_matrix_instance_f32 mat_meas_t;
    arm_matrix_instance_f32 mat_pn;
    arm_matrix_instance_f32 mat_pn_t;
    arm_matrix_instance_f32 mat_output;
    arm_matrix_instance_f32 mat_noise_prod;
    arm_matrix_instance_f32 mat_scaled_noise;
    arm_matrix_instance_f32 mat_scaled_input;
    arm_matrix_instance_f32 mat_pred_state;
    arm_matrix_instance_f32 mat_innov_cov;
    arm_matrix_instance_f32 mat_innov_prod;
    arm_matrix_instance_f32 mat_innov_sum;
    arm_matrix_instance_f32 mat_gain;
    arm_matrix_instance_f32 mat_gain_temp;
    arm_matrix_instance_f32 mat_inv_innov;
    arm_matrix_instance_f32 mat_pred_meas;
    arm_matrix_instance_f32 mat_innov;
    arm_matrix_instance_f32 mat_correction;
    arm_matrix_instance_f32 mat_meas_noise;
    arm_matrix_instance_f32 mat_sensor;
    arm_matrix_instance_f32 mat_position;

    float velocity_estimate;
} KalmanFilter;

void kf_init(KalmanFilter* kf, float32_t* sys_matrix, float32_t* input_matrix, float32_t proc_noise, float32_t meas_noise);
float kf_update(KalmanFilter* kf, float32_t control_input, float32_t measurement);
void kf_clear(KalmanFilter* kf);

#endif /* INC_MOTORKALMAN_H_ */
