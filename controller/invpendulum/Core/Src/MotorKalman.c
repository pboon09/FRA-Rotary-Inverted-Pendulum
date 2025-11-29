/*
 * MotorKalman.c
 *
 *  Created on: Nov 20, 2025
 *      Author: pboon
 */

#include "MotorKalman.h"
#include "arm_math.h"
#include <string.h>

void kf_init(KalmanFilter* kf, float32_t* sys_matrix, float32_t* input_matrix, float32_t proc_noise, float32_t meas_noise) {
    kf->process_noise = proc_noise;
    kf->measurement_noise[0] = meas_noise;

    float32_t meas_vec[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float32_t noise_vec[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    float32_t identity_mat[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f};
    float32_t init_state[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    kf->output[1] = 0.0f;

    for(int i = 0; i < 16; i++) {
        kf->system_mat[i] = sys_matrix[i];
        kf->identity[i] = identity_mat[i];
        kf->covariance[i] = 0.0f;
    }

    for(int i = 0; i < 4; i++) {
        kf->state[i] = init_state[i];
        kf->input_mat[i] = input_matrix[i];
        kf->measurement_mat[i] = meas_vec[i];
        kf->process_noise_mat[i] = noise_vec[i];
    }

    arm_mat_init_f32(&kf->mat_state, 4, 1, kf->state);
    arm_mat_init_f32(&kf->mat_cov, 4, 4, kf->covariance);
    arm_mat_init_f32(&kf->mat_sys, 4, 4, kf->system_mat);
    arm_mat_init_f32(&kf->mat_input, 4, 1, kf->input_mat);
    arm_mat_init_f32(&kf->mat_meas, 1, 4, kf->measurement_mat);
    arm_mat_init_f32(&kf->mat_pn, 4, 1, kf->process_noise_mat);
    arm_mat_init_f32(&kf->mat_sys_t, 4, 4, kf->sys_transpose);
    arm_mat_init_f32(&kf->mat_meas_t, 4, 1, kf->meas_transpose);
    arm_mat_init_f32(&kf->mat_pn_t, 1, 4, kf->noise_transpose);
    arm_mat_init_f32(&kf->mat_noise_prod, 4, 4, kf->noise_product);
    arm_mat_init_f32(&kf->mat_scaled_noise, 4, 4, kf->scaled_noise);
    arm_mat_init_f32(&kf->mat_scaled_input, 4, 1, kf->scaled_input);
    arm_mat_init_f32(&kf->mat_pred_state, 4, 1, kf->pred_state);
    arm_mat_init_f32(&kf->mat_innov_cov, 1, 4, kf->innovation_cov);
    arm_mat_init_f32(&kf->mat_innov_prod, 1, 1, kf->innovation_product);
    arm_mat_init_f32(&kf->mat_innov_sum, 1, 1, kf->innovation_sum);
    arm_mat_init_f32(&kf->mat_gain, 4, 1, kf->gain);
    arm_mat_init_f32(&kf->mat_gain_temp, 4, 1, kf->gain_temp);
    arm_mat_init_f32(&kf->mat_inv_innov, 1, 1, kf->inv_innovation);
    arm_mat_init_f32(&kf->mat_pred_meas, 1, 1, kf->predicted_meas);
    arm_mat_init_f32(&kf->mat_innov, 1, 1, kf->innovation);
    arm_mat_init_f32(&kf->mat_correction, 4, 1, kf->correction);
    arm_mat_init_f32(&kf->mat_output, 1, 1, kf->output);
    arm_mat_init_f32(&kf->mat_identity, 4, 4, kf->identity);
    arm_mat_init_f32(&kf->mat_meas_noise, 1, 1, kf->measurement_noise);
    arm_mat_init_f32(&kf->mat_sensor, 1, 1, kf->sensor_data);
}

float kf_update(KalmanFilter* kf, float32_t control_input, float32_t measurement) {
    arm_mat_init_f32(&kf->mat_position, 1, 1, &measurement);

    arm_mat_trans_f32(&kf->mat_sys, &kf->mat_sys_t);
    arm_mat_trans_f32(&kf->mat_meas, &kf->mat_meas_t);
    arm_mat_trans_f32(&kf->mat_pn, &kf->mat_pn_t);

    arm_mat_scale_f32(&kf->mat_input, control_input, &kf->mat_scaled_input);
    arm_mat_mult_f32(&kf->mat_sys, &kf->mat_state, &kf->mat_pred_state);
    arm_mat_add_f32(&kf->mat_pred_state, &kf->mat_scaled_input, &kf->mat_state);

    arm_mat_mult_f32(&kf->mat_sys, &kf->mat_cov, &kf->mat_cov);
    arm_mat_mult_f32(&kf->mat_cov, &kf->mat_sys_t, &kf->mat_cov);
    arm_mat_mult_f32(&kf->mat_pn, &kf->mat_pn_t, &kf->mat_noise_prod);
    arm_mat_scale_f32(&kf->mat_noise_prod, kf->process_noise, &kf->mat_scaled_noise);
    arm_mat_add_f32(&kf->mat_cov, &kf->mat_scaled_noise, &kf->mat_cov);

    arm_mat_mult_f32(&kf->mat_meas, &kf->mat_cov, &kf->mat_innov_cov);
    arm_mat_mult_f32(&kf->mat_innov_cov, &kf->mat_meas_t, &kf->mat_innov_prod);
    arm_mat_add_f32(&kf->mat_innov_prod, &kf->mat_meas_noise, &kf->mat_innov_sum);

    arm_mat_inverse_f32(&kf->mat_innov_sum, &kf->mat_inv_innov);

    arm_mat_mult_f32(&kf->mat_cov, &kf->mat_meas_t, &kf->mat_gain_temp);
    arm_mat_mult_f32(&kf->mat_gain_temp, &kf->mat_inv_innov, &kf->mat_gain);

    arm_mat_mult_f32(&kf->mat_meas, &kf->mat_state, &kf->mat_pred_meas);
    arm_mat_sub_f32(&kf->mat_position, &kf->mat_pred_meas, &kf->mat_innov);
    arm_mat_mult_f32(&kf->mat_gain, &kf->mat_innov, &kf->mat_correction);
    arm_mat_add_f32(&kf->mat_state, &kf->mat_correction, &kf->mat_state);

    arm_mat_mult_f32(&kf->mat_meas, &kf->mat_state, &kf->mat_output);

    arm_matrix_instance_f32 temp_mat;
    float32_t temp_data[16];
    arm_mat_init_f32(&temp_mat, 4, 4, temp_data);

    arm_mat_mult_f32(&kf->mat_gain, &kf->mat_meas, &temp_mat);
    arm_mat_sub_f32(&kf->mat_identity, &temp_mat, &temp_mat);
    arm_mat_mult_f32(&temp_mat, &kf->mat_cov, &kf->mat_cov);

    kf->velocity_estimate = kf->state[1];
    return kf->velocity_estimate;
}

void kf_clear(KalmanFilter* kf) {
    memset(kf->state, 0, sizeof(kf->state));
    memset(kf->covariance, 0, sizeof(kf->covariance));

    for(int i = 0; i < 4; i++) {
        kf->covariance[i * 4 + i] = 100.0f;
    }

    kf->velocity_estimate = 0.0f;
    memset(kf->output, 0, sizeof(kf->output));
    memset(kf->scaled_input, 0, sizeof(kf->scaled_input));
    memset(kf->pred_state, 0, sizeof(kf->pred_state));
    memset(kf->gain, 0, sizeof(kf->gain));
}
