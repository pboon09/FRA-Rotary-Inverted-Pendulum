/*
 * logger.h
 *
 *  Created on: Nov 6, 2025
 *      Author: bpbeam
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include "ff.h"

// Logger functions
uint8_t Logger_Init(void);
uint8_t Logger_WriteString(const char *filename, const char *data);
uint8_t Logger_WriteLine(const char *filename, const char *data);
uint8_t Logger_WriteData(const char *filename, const uint8_t *data, uint32_t len);
void Logger_PrintStatus(void);
void Logger_Test(void);

#endif
