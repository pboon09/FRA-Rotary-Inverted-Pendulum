/*
 * energy_swingup.h
 *
 *  Created on: Nov 2, 2025
 *      Author: pboon
 */

#ifndef INC_ENERGY_SWINGUP_H_
#define INC_ENERGY_SWINGUP_H_

#include "main.h"
#include "math.h"

typedef struct {
    float Mp;     // pendulum mass (kg)
    float Lp;     // pendulum length (m)
    float Jp;     // pendulum inertia (kg*m^2)
    float g;      // gravity
    float kE;     // energy gain
    float Eref;   // reference energy
    float alpha;  // pendulum angle (rad)
    float alpha_dot; // pendulum angular velocity (rad/s)
} EnergyCtrl;

void EnergyCtrl_Init(EnergyCtrl *ec, float Mp, float Lp, float Jp, float g, float kE);
float EnergyCtrl_Update(EnergyCtrl *ec, float alpha, float alpha_dot);

#endif /* INC_ENERGY_SWINGUP_H_ */
