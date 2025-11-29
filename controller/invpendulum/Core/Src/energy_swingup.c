/*
 * energy_swingup.c
 *
 *  Created on: Nov 2, 2025
 *      Author: pboon
 */

#include "energy_swingup.h"

static inline float sat_smooth(float x, float limit) {
	return limit * tanhf(x / limit);
}

void EnergyCtrl_Init(EnergyCtrl *ec, float Mp, float Lp, float Jp, float g,
		float kE) {
	ec->Mp = Mp;
	ec->Lp = Lp;
	ec->Jp = Jp;
	ec->g = g;
	ec->kE = kE;

	ec->Eref = 2.0f * Mp * g * Lp;
	ec->E = 0.0;
	ec->dE = 0.0f;
	ec->ap = 0.0f;
}

float EnergyCtrl_Update(EnergyCtrl *ec, float alpha, float alpha_dot) {
	ec->E = 0.5f * ec->Jp * alpha_dot * alpha_dot
			+ ec->Mp * ec->g * ec->Lp * (1.0f - cosf(alpha));

	ec->dE = ec->E - ec->Eref;

	ec->dE_filt = 0.95f * ec->dE_filt + 0.05f * ec->dE;

	ec->ap = ec->kE * ec->dE_filt * alpha_dot * cosf(alpha) - 0.05f * alpha_dot;

	ec->ap = sat_smooth(ec->ap, 1.0f);

	return ec->ap;
}
