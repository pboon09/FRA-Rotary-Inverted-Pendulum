#include "hc05.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

extern void Debug_Printf(const char* format, ...);

HAL_StatusTypeDef HC05_Init(HC05_Handle_t *hhc05, UART_HandleTypeDef *huart, uint32_t baudrate)
{
    if (hhc05 == NULL || huart == NULL) {
        Debug_Printf("HC05_Init: NULL parameter\r\n");
        return HAL_ERROR;
    }

    Debug_Printf("HC05_Init: Starting...\r\n");

    /* Store UART handle */
    hhc05->huart = huart;

    /* Initialize buffers */
    memset(hhc05->rx_dma_byte, 0, HC05_RX_DMA_SIZE);
    memset(hhc05->line_buffer, 0, HC05_LINE_BUFFER_SIZE);
    memset(hhc05->tx_buffer, 0, HC05_TX_BUFFER_SIZE);

    /* Initialize state variables */
    hhc05->line_index = 0;
    hhc05->line_ready = 0;
    hhc05->tx_busy = 0;
    hhc05->rx_active = 0;
    hhc05->command_callback = NULL;
    hhc05->error_count = 0;

    /* Configure UART baud rate */
    hhc05->huart->Init.BaudRate = baudrate;
    if (HAL_UART_Init(hhc05->huart) != HAL_OK) {
        Debug_Printf("HC05_Init: UART Init FAILED\r\n");
        return HAL_ERROR;
    }

    HAL_Delay(100);

    Debug_Printf("HC05_Init: SUCCESS\r\n");
    return HAL_OK;
}

void HC05_SetCommandCallback(HC05_Handle_t *hhc05, HC05_CommandCallback_t callback)
{
    hhc05->command_callback = callback;
}

void HC05_Start(HC05_Handle_t *hhc05)
{
    HAL_StatusTypeDef status;

    if (hhc05->rx_active) {
        Debug_Printf("HC05_Start: Already active\r\n");
        return;
    }

    Debug_Printf("HC05_Start: Attempting DMA receive...\r\n");

    /* Ensure UART is in a clean state */
    HAL_UART_AbortReceive(hhc05->huart);
    HAL_Delay(1);

    status = HAL_UART_Receive_DMA(hhc05->huart, hhc05->rx_dma_byte, HC05_RX_DMA_SIZE);

    if (status != HAL_OK) {
        Debug_Printf("HC05_Start: DMA failed (status=%d), trying interrupt mode...\r\n", status);

        /* Abort again before trying interrupt mode */
        HAL_UART_AbortReceive(hhc05->huart);
        HAL_Delay(1);

        status = HAL_UART_Receive_IT(hhc05->huart, hhc05->rx_dma_byte, HC05_RX_DMA_SIZE);

        if (status != HAL_OK) {
            Debug_Printf("HC05_Start: FAILED (status=%d)\r\n", status);
            hhc05->error_count++;
        } else {
            Debug_Printf("HC05_Start: SUCCESS (Interrupt mode)\r\n");
            hhc05->rx_active = 1;
        }
    } else {
        Debug_Printf("HC05_Start: SUCCESS (DMA mode)\r\n");
        hhc05->rx_active = 1;
    }
}

void HC05_Stop(HC05_Handle_t *hhc05)
{
    if (hhc05->rx_active) {
        HAL_UART_DMAStop(hhc05->huart);
        HAL_UART_AbortReceive(hhc05->huart);
        hhc05->rx_active = 0;
    }
}

void HC05_Send(HC05_Handle_t *hhc05, const char *msg)
{
    uint16_t len = strlen(msg);
    if (len > HC05_TX_BUFFER_SIZE)
        len = HC05_TX_BUFFER_SIZE;

    /* Wait if previous TX is still running */
    uint32_t timeout = HAL_GetTick() + 1000;
    while (hhc05->tx_busy) {
        if (HAL_GetTick() > timeout) {
            Debug_Printf("HC05_Send: TX timeout, forcing reset\r\n");
            hhc05->tx_busy = 0;
            hhc05->error_count++;
            return;
        }
    }

    /* Copy message to TX buffer */
    memcpy(hhc05->tx_buffer, msg, len);

    /* Try DMA transmission first */
    hhc05->tx_busy = 1;
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(hhc05->huart, hhc05->tx_buffer, len);

    if (status != HAL_OK) {
        /* Fallback to blocking transmission */
        status = HAL_UART_Transmit(hhc05->huart, hhc05->tx_buffer, len, 1000);
        hhc05->tx_busy = 0;
        if (status != HAL_OK) {
            hhc05->error_count++;
        }
    }
}

void HC05_SendFormatted(HC05_Handle_t *hhc05, const char *format, ...)
{
    char buffer[HC05_TX_BUFFER_SIZE];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0 && len < HC05_TX_BUFFER_SIZE) {
        HC05_Send(hhc05, buffer);
    }
}

void HC05_Process(HC05_Handle_t *hhc05)
{
    if (hhc05->line_ready) {
        hhc05->line_ready = 0;

        if (hhc05->command_callback != NULL) {
            hhc05->command_callback((char*)hhc05->line_buffer);
        }
    }

    /* Recovery mechanism - restart RX if not active (but not too frequently) */
    if (!hhc05->rx_active && hhc05->error_count < 10) {
        static uint32_t last_restart = 0;
        uint32_t now = HAL_GetTick();

        /* Only attempt restart every 1 second */
        if (now - last_restart > 1000) {
            HC05_Start(hhc05);
            last_restart = now;
        }
    }
}

void HC05_UART_RxCpltCallback(HC05_Handle_t *hhc05)
{
    uint8_t byte = hhc05->rx_dma_byte[0];

    /* Mark as not active immediately to allow restart */
    hhc05->rx_active = 0;

    /* Echo byte back */
    HAL_UART_Transmit(hhc05->huart, &byte, 1, 10);

    /* Process received byte */
    HC05_ProcessReceivedByte(hhc05, byte);

    /* Restart reception without abort (cleaner restart) */
    HAL_StatusTypeDef status = HAL_UART_Receive_DMA(hhc05->huart, hhc05->rx_dma_byte, HC05_RX_DMA_SIZE);

    if (status != HAL_OK) {
        /* If DMA fails, try interrupt mode */
        status = HAL_UART_Receive_IT(hhc05->huart, hhc05->rx_dma_byte, HC05_RX_DMA_SIZE);
    }

    if (status == HAL_OK) {
        hhc05->rx_active = 1;
    } else {
        /* Will be restarted by HC05_Process recovery mechanism */
        hhc05->error_count++;
    }
}

void HC05_UART_TxCpltCallback(HC05_Handle_t *hhc05)
{
    hhc05->tx_busy = 0;
}

void HC05_UART_ErrorCallback(HC05_Handle_t *hhc05)
{
    hhc05->rx_active = 0;
    hhc05->error_count++;

    Debug_Printf("HC05_Error: UART error (count=%lu)\r\n", hhc05->error_count);

    /* Clear all error flags */
    __HAL_UART_CLEAR_FLAG(hhc05->huart, UART_CLEAR_OREF | UART_CLEAR_NEF |
                          UART_CLEAR_PEF | UART_CLEAR_FEF);

    /* Abort any ongoing operations */
    HAL_UART_AbortReceive(hhc05->huart);

    /* Attempt immediate restart if error count is low */
    if (hhc05->error_count < 5) {
        HAL_Delay(10);
        HAL_StatusTypeDef status = HAL_UART_Receive_DMA(hhc05->huart, hhc05->rx_dma_byte, HC05_RX_DMA_SIZE);
        if (status != HAL_OK) {
            status = HAL_UART_Receive_IT(hhc05->huart, hhc05->rx_dma_byte, HC05_RX_DMA_SIZE);
        }

        if (status == HAL_OK) {
            hhc05->rx_active = 1;
            Debug_Printf("HC05_Error: Recovery successful\r\n");
        }
    }
}

uint8_t HC05_IsLineReady(HC05_Handle_t *hhc05)
{
    return hhc05->line_ready;
}

char* HC05_GetLine(HC05_Handle_t *hhc05)
{
    return (char*)hhc05->line_buffer;
}

void HC05_ClearLine(HC05_Handle_t *hhc05)
{
    hhc05->line_ready = 0;
    hhc05->line_index = 0;
}

void HC05_ProcessReceivedByte(HC05_Handle_t *hhc05, uint8_t byte)
{
    /* Check for line ending */
    if (byte == '\n' || byte == '\r') {
        HAL_UART_Transmit(hhc05->huart, (uint8_t*)"\r\n", 2, 10);

        if (hhc05->line_index > 0) {
            hhc05->line_buffer[hhc05->line_index] = '\0';
            HC05_TrimLine((char*)hhc05->line_buffer);

            Debug_Printf("HC05_Line: '%s'\r\n", hhc05->line_buffer);

            hhc05->line_ready = 1;
        }
        hhc05->line_index = 0;
    }
    else if (byte == 127 || byte == 8) {
        if (hhc05->line_index > 0) {
            hhc05->line_index--;
            HAL_UART_Transmit(hhc05->huart, (uint8_t*)"\b \b", 3, 10);
        }
    }
    else if (byte >= 32 && byte < 127) {
        if (hhc05->line_index < HC05_LINE_BUFFER_SIZE - 1) {
            hhc05->line_buffer[hhc05->line_index++] = byte;
        }
        else {
            Debug_Printf("HC05: Buffer overflow\r\n");
            hhc05->line_index = 0;
        }
    }
}

void HC05_TrimLine(char *line)
{
    char *start = line;
    while (*start == ' ' || *start == '\t') start++;

    if (start != line) {
        memmove(line, start, strlen(start) + 1);
    }

    char *end = line + strlen(line) - 1;
    while (end > line && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }
}

uint32_t HC05_GetErrorCount(HC05_Handle_t *hhc05)
{
    return hhc05->error_count;
}

void HC05_ResetErrorCount(HC05_Handle_t *hhc05)
{
    hhc05->error_count = 0;
}
