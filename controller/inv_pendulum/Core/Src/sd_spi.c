/*
 * sd_spi.c
 *
 *  Created on: Nov 6, 2025
 *      Author: bpbeam
 */

#include "sd_spi.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

extern void Debug_Printf(const char* format, ...);

static uint8_t sd_card_type = SD_TYPE_UNKNOWN;

static inline void SD_CS_LOW(void) {
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static inline void SD_CS_HIGH(void) {
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

static uint8_t SD_SPI_Transfer(uint8_t data) {
    uint8_t rxData = 0xFF;
    HAL_StatusTypeDef status;

    status = HAL_SPI_TransmitReceive(&hspi2, &data, &rxData, 1, 100);

    if (status != HAL_OK) {
        return 0xFF;
    }

    return rxData;
}

static void SD_SPI_SendDummy(uint16_t count) {
    for (uint16_t i = 0; i < count; i++) {
        SD_SPI_Transfer(0xFF);
    }
}

static uint8_t SD_WaitReady(uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms) {
        if (SD_SPI_Transfer(0xFF) == 0xFF) {
            return SD_SUCCESS;
        }
    }
    return SD_TIMEOUT;
}

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg) {
    uint8_t response;
    uint16_t retry = 0;

    if (SD_WaitReady(500) != SD_SUCCESS) {
        return 0xFF;
    }

    SD_SPI_Transfer(0x40 | cmd);
    SD_SPI_Transfer((uint8_t)(arg >> 24));
    SD_SPI_Transfer((uint8_t)(arg >> 16));
    SD_SPI_Transfer((uint8_t)(arg >> 8));
    SD_SPI_Transfer((uint8_t)arg);

    uint8_t crc = 0x01;
    if (cmd == CMD0) crc = 0x95;
    if (cmd == CMD8) crc = 0x87;
    SD_SPI_Transfer(crc);

    for (retry = 0; retry < 10; retry++) {
        response = SD_SPI_Transfer(0xFF);
        if (!(response & 0x80)) {
            return response;
        }
    }

    return 0xFF;
}

uint8_t SD_Init(void) {
    uint8_t response;
    uint32_t retry;

    Debug_Printf("\r\n=== SD_Init START ===\r\n");

    Debug_Printf("Step 1: Power-up delay...\r\n");
    HAL_Delay(10);
    Debug_Printf("  OK\r\n");

    Debug_Printf("Step 2: Set CS HIGH...\r\n");
    SD_CS_HIGH();
    Debug_Printf("  OK\r\n");

    Debug_Printf("Step 3: Sending dummy clocks...\r\n");
    SD_SPI_SendDummy(10);
    Debug_Printf("  OK (sent 10 bytes)\r\n");

    Debug_Printf("Step 4: Set CS LOW...\r\n");
    SD_CS_LOW();
    HAL_Delay(1);
    Debug_Printf("  OK\r\n");

    Debug_Printf("Step 5: Sending CMD0 (GO_IDLE_STATE)...\r\n");
    response = SD_SendCommand(CMD0, 0);
    Debug_Printf("  Response: 0x%02X ", response);
    if (response == 0x01) {
        Debug_Printf("OK (idle state)\r\n");
    } else if (response == 0xFF) {
        Debug_Printf("FAIL (no response)\r\n");
        Debug_Printf("  Possible causes:\r\n");
        Debug_Printf("    - SD card not inserted\r\n");
        Debug_Printf("    - Wrong wiring (MOSI/MISO/SCK/CS)\r\n");
        Debug_Printf("    - SPI speed too fast\r\n");
        Debug_Printf("    - Bad SD card\r\n");
        SD_CS_HIGH();
        return SD_INIT_ERROR;
    } else {
        Debug_Printf("FAIL (unexpected: 0x%02X)\r\n", response);
        SD_CS_HIGH();
        return SD_INIT_ERROR;
    }

    Debug_Printf("Step 6: Sending CMD8 (SEND_IF_COND)...\r\n");
    response = SD_SendCommand(CMD8, 0x1AA);
    Debug_Printf("  Response: 0x%02X\r\n", response);

    if (response == 0x01) {
        Debug_Printf("  Card type: SD Ver2.x\r\n");
        uint8_t ocr[4];
        for (int i = 0; i < 4; i++) {
            ocr[i] = SD_SPI_Transfer(0xFF);
        }

        if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
            Debug_Printf("Step 7: Sending ACMD41 (waiting for ready)...\r\n");
            retry = 0;
            do {
                response = SD_SendCommand(CMD55, 0);
                if (response > 1) break;

                response = SD_SendCommand(ACMD41, 0x40000000);

                HAL_Delay(1);
                retry++;
            } while (response != 0x00 && retry < 1000);

            if (retry >= 1000) {
                SD_CS_HIGH();
                return SD_TIMEOUT;
            }

            Debug_Printf("Step 8: Reading OCR (CMD58)...\r\n");
            response = SD_SendCommand(CMD58, 0);
            if (response == 0x00) {
                uint8_t ocr_reg[4];
                for (int i = 0; i < 4; i++) {
                    ocr_reg[i] = SD_SPI_Transfer(0xFF);
                }

                if (ocr_reg[0] & 0x40) {
                    sd_card_type = SD_TYPE_SDHC;
                } else {
                    sd_card_type = SD_TYPE_V2;
                }
            }
        }
    } else {
        sd_card_type = SD_TYPE_V1;

        retry = 0;
        do {
            response = SD_SendCommand(CMD55, 0);
            if (response > 1) break;

            response = SD_SendCommand(ACMD41, 0);

            HAL_Delay(1);
            retry++;
        } while (response != 0x00 && retry < 1000);

        if (retry >= 1000) {
            SD_CS_HIGH();
            return SD_TIMEOUT;
        }
    }

    response = SD_SendCommand(CMD16, 512);
    if (response != 0x00) {
        SD_CS_HIGH();
        return SD_INIT_ERROR;
    }

    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);

    return SD_SUCCESS;
}

uint8_t SD_ReadBlock(uint32_t blockAddr, uint8_t *buffer) {
    uint8_t response;

    if (sd_card_type != SD_TYPE_SDHC) {
        blockAddr *= 512;
    }

    SD_CS_LOW();

    response = SD_SendCommand(CMD17, blockAddr);
    if (response != 0x00) {
        SD_CS_HIGH();
        return response;
    }

    uint16_t timeout = 0;
    while (SD_SPI_Transfer(0xFF) != 0xFE) {
        if (++timeout > 50000) {
            SD_CS_HIGH();
            return SD_TIMEOUT;
        }
    }

    for (uint16_t i = 0; i < 512; i++) {
        buffer[i] = SD_SPI_Transfer(0xFF);
    }

    SD_SPI_Transfer(0xFF);
    SD_SPI_Transfer(0xFF);

    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);

    return SD_SUCCESS;
}

uint8_t SD_WriteBlock(uint32_t blockAddr, const uint8_t *buffer) {
    uint8_t response;

    if (sd_card_type != SD_TYPE_SDHC) {
        blockAddr *= 512;
    }

    SD_CS_LOW();

    response = SD_SendCommand(CMD24, blockAddr);
    if (response != 0x00) {
        SD_CS_HIGH();
        return response;
    }

    SD_SPI_Transfer(0xFE);

    for (uint16_t i = 0; i < 512; i++) {
        SD_SPI_Transfer(buffer[i]);
    }

    SD_SPI_Transfer(0xFF);
    SD_SPI_Transfer(0xFF);

    response = SD_SPI_Transfer(0xFF);
    if ((response & 0x1F) != 0x05) {
        SD_CS_HIGH();
        return SD_INIT_ERROR;
    }

    if (SD_WaitReady(5000) != SD_SUCCESS) {
        SD_CS_HIGH();
        return SD_TIMEOUT;
    }

    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);

    return SD_SUCCESS;
}
