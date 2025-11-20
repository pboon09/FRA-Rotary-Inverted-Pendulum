/*
 * FIR.c
 *
 *  Created on: Nov 2, 2025
 *      Author: pboon
 */

#include "FIR.h"
#include "arm_math.h"

void FIR_init(FIR *fir, uint16_t numTaps, float cutoffFreq, float samplingFreq) {
    if (numTaps % 2 == 0) {
        numTaps += 1;
    }

    fir->numTaps = numTaps;
    fir->bufferIndex = 0;

    fir->coeffs = (float*)malloc(numTaps * sizeof(float));
    fir->buffer = (float*)malloc(numTaps * sizeof(float));

    if (fir->coeffs != NULL && fir->buffer != NULL) {
        for (uint16_t i = 0; i < numTaps; i++) {
            fir->buffer[i] = 0.0f;
        }

        float omega = cutoffFreq / samplingFreq;

        int16_t half_taps = numTaps / 2;
        for (int16_t i = 0; i < numTaps; i++) {
            if (i == half_taps) {
                fir->coeffs[i] = 2.0f * omega;
            } else {
                int16_t n = i - half_taps;
                fir->coeffs[i] = sinf(2.0f * PI * omega * n) / (PI * n);
            }

            fir->coeffs[i] *= (0.54f - 0.46f * cosf(2.0f * PI * i / (numTaps - 1)));
        }

        float sum = 0.0f;
        for (uint16_t i = 0; i < numTaps; i++) {
            sum += fir->coeffs[i];
        }

        if (sum != 0.0f) {
            for (uint16_t i = 0; i < numTaps; i++) {
                fir->coeffs[i] /= sum;
            }
        }
    }
}

float FIR_process(FIR *fir, float input) {
    fir->buffer[fir->bufferIndex] = input;

    float output = 0.0f;
    uint16_t index = fir->bufferIndex;

    for (uint16_t i = 0; i < fir->numTaps; i++) {
        output += fir->buffer[index] * fir->coeffs[i];

        if (index == 0) {
            index = fir->numTaps - 1;
        } else {
            index--;
        }
    }

    fir->bufferIndex++;
    if (fir->bufferIndex >= fir->numTaps) {
        fir->bufferIndex = 0;
    }

    return output;
}

void FIR_reset(FIR *fir) {
    for (uint16_t i = 0; i < fir->numTaps; i++) {
        fir->buffer[i] = 0.0f;
    }
    fir->bufferIndex = 0;
}

void FIR_delete(FIR *fir) {
    if (fir->coeffs != NULL) {
        free(fir->coeffs);
        fir->coeffs = NULL;
    }

    if (fir->buffer != NULL) {
        free(fir->buffer);
        fir->buffer = NULL;
    }
}
