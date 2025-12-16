#ifndef HC05_H
#define HC05_H

#include "stm32g4xx_hal.h"
#include "main.h"
#include <stdint.h>

/* Buffer sizes */
#define HC05_RX_DMA_SIZE        1
#define HC05_LINE_BUFFER_SIZE   128
#define HC05_TX_BUFFER_SIZE     1024

/* Command callback type */
typedef void (*HC05_CommandCallback_t)(char *command);

/* HC05 handle structure */
typedef struct {
    UART_HandleTypeDef *huart;

    /* RX buffers */
    uint8_t rx_dma_byte[HC05_RX_DMA_SIZE];
    uint8_t line_buffer[HC05_LINE_BUFFER_SIZE];
    uint16_t line_index;
    uint8_t line_ready;

    /* TX buffer */
    uint8_t tx_buffer[HC05_TX_BUFFER_SIZE];
    uint8_t tx_busy;

    /* State tracking */
    uint8_t rx_active;
    uint32_t error_count;

    /* Callback */
    HC05_CommandCallback_t command_callback;
} HC05_Handle_t;

/* Function prototypes */
HAL_StatusTypeDef HC05_Init(HC05_Handle_t *hhc05, UART_HandleTypeDef *huart, uint32_t baudrate);
void HC05_SetCommandCallback(HC05_Handle_t *hhc05, HC05_CommandCallback_t callback);
void HC05_Start(HC05_Handle_t *hhc05);
void HC05_Stop(HC05_Handle_t *hhc05);
void HC05_Send(HC05_Handle_t *hhc05, const char *msg);
void HC05_SendFormatted(HC05_Handle_t *hhc05, const char *format, ...);
void HC05_Process(HC05_Handle_t *hhc05);

/* Callback functions (call from HAL callbacks) */
void HC05_UART_RxCpltCallback(HC05_Handle_t *hhc05);
void HC05_UART_TxCpltCallback(HC05_Handle_t *hhc05);
void HC05_UART_ErrorCallback(HC05_Handle_t *hhc05);

/* Utility functions */
uint8_t HC05_IsLineReady(HC05_Handle_t *hhc05);
char* HC05_GetLine(HC05_Handle_t *hhc05);
void HC05_ClearLine(HC05_Handle_t *hhc05);
void HC05_ProcessReceivedByte(HC05_Handle_t *hhc05, uint8_t byte);
void HC05_TrimLine(char *line);

/* Error tracking */
uint32_t HC05_GetErrorCount(HC05_Handle_t *hhc05);
void HC05_ResetErrorCount(HC05_Handle_t *hhc05);

#endif /* HC05_H */
