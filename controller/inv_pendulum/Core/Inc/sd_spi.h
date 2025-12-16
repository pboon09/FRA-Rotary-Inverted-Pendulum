#ifndef SD_SPI_H
#define SD_SPI_H

#include "stm32g4xx_hal.h"
#include "main.h"
#include <stdint.h>

/* SD Card Commands */
#define CMD0    0   /* GO_IDLE_STATE */
#define CMD8    8   /* SEND_IF_COND */
#define CMD16   16  /* SET_BLOCKLEN */
#define CMD17   17  /* READ_SINGLE_BLOCK */
#define CMD24   24  /* WRITE_BLOCK */
#define CMD55   55  /* APP_CMD */
#define CMD58   58  /* READ_OCR */
#define ACMD41  41  /* SD_SEND_OP_COND */

/* SD Card Types */
#define SD_TYPE_UNKNOWN 0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_SDHC    3

/* Return Values */
#define SD_SUCCESS      0
#define SD_INIT_ERROR   1
#define SD_TIMEOUT      2

/* Function Prototypes */
uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t blockAddr, uint8_t *buffer);
uint8_t SD_WriteBlock(uint32_t blockAddr, const uint8_t *buffer);

#endif /* SD_SPI_H */
