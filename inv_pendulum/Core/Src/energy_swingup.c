/*
 * energy_swingup.c
 *
 *  Created on: Nov 2, 2025
 *      Author: pboon
 */


#include "energy_swingup.h"

void EnergyCtrl_Init(EnergyCtrl *ec, float Mp, float Lp, float Jp, float g, float kE) {
    ec->Mp = Mp;
    ec->Lp = Lp;
    ec->Jp = Jp;
    ec->g = g;
    ec->kE = kE;
    ec->Eref = Mp * g * Lp * 2.0f; //upright position
}

float EnergyCtrl_Update(EnergyCtrl *ec, float alpha, float alpha_dot) {
    float E = 0.5f * ec->Jp * alpha_dot * alpha_dot + ec->Mp * ec->g * ec->Lp * (1 - cosf(alpha));
    float dE = E - ec->Eref;

    float ap = ec->kE * dE * alpha_dot * cosf(alpha);
    if (ap > 1.0f) ap = 1.0f;
    if (ap < -1.0f) ap = -1.0f;

    return ap;
}
