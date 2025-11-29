/*
 * sd_spi.h
 *
 *  Created on: Nov 6, 2025
 *      Author: bpbeam
 */

#ifndef SD_SPI_H
#define SD_SPI_H

//#include "stm32g4xx_hal.h"
#include <stdint.h>

// SD Card Commands
#define CMD0    0   // GO_IDLE_STATE
#define CMD8    8   // SEND_IF_COND
#define CMD9    9   // SEND_CSD
#define CMD10   10  // SEND_CID
#define CMD12   12  // STOP_TRANSMISSION
#define CMD16   16  // SET_BLOCKLEN
#define CMD17   17  // READ_SINGLE_BLOCK
#define CMD18   18  // READ_MULTIPLE_BLOCK
#define CMD23   23  // SET_BLOCK_COUNT
#define CMD24   24  // WRITE_BLOCK
#define CMD25   25  // WRITE_MULTIPLE_BLOCK
#define CMD55   55  // APP_CMD
#define CMD58   58  // READ_OCR
#define ACMD41  41  // SD_SEND_OP_COND

// Response types
#define SD_SUCCESS          0x00
#define SD_INIT_ERROR       0x01
#define SD_TIMEOUT          0x02

// SD Card Types
#define SD_TYPE_UNKNOWN     0
#define SD_TYPE_V1          1
#define SD_TYPE_V2          2
#define SD_TYPE_SDHC        3

// Function prototypes
uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t blockAddr, uint8_t *buffer);
uint8_t SD_WriteBlock(uint32_t blockAddr, const uint8_t *buffer);
uint8_t SD_GetCardInfo(void);

#endif
