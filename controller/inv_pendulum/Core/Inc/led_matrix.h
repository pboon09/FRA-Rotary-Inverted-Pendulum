#ifndef LED_MATRIX_H_
#define LED_MATRIX_H_

#include "stm32g4xx_hal.h"
#include <math.h>

/* Constants */
#define LED_MATRIX_ROWS 8
#define LED_MATRIX_COLS 8
#define LED_MATRIX_DM163_CHANNELS 24
#define LED_MATRIX_CENTER_ROW 3
#define LED_MATRIX_CENTER_COL 3
#define LED_MATRIX_PENDULUM_LEN 4

/* Color structure */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} LED_Matrix_Color_t;

/* Predefined colors */
#define LED_MATRIX_COLOR_WHITE  ((LED_Matrix_Color_t){255, 255, 255})
#define LED_MATRIX_COLOR_RED    ((LED_Matrix_Color_t){255, 0, 0})
#define LED_MATRIX_COLOR_GREEN  ((LED_Matrix_Color_t){0, 255, 0})
#define LED_MATRIX_COLOR_BLUE   ((LED_Matrix_Color_t){0, 0, 255})
#define LED_MATRIX_COLOR_OFF    ((LED_Matrix_Color_t){0, 0, 0})

/* LED Matrix handle structure */
typedef struct {
    uint8_t frame_buffer[LED_MATRIX_ROWS][LED_MATRIX_COLS][3];
    uint8_t current_row;
    uint32_t last_refresh_tick;
    uint8_t is_initialized;
} LED_Matrix_Handle_t;

/* Initialization */
HAL_StatusTypeDef LED_Matrix_Init(LED_Matrix_Handle_t* hmatrix);
HAL_StatusTypeDef LED_Matrix_Reset(LED_Matrix_Handle_t* hmatrix);

/* Row control */
void LED_Matrix_DisableAllRows(LED_Matrix_Handle_t* hmatrix);
void LED_Matrix_EnableRow(LED_Matrix_Handle_t* hmatrix, uint8_t row);

/* Data transfer */
void LED_Matrix_SendData(LED_Matrix_Handle_t* hmatrix, uint8_t data);
void LED_Matrix_LatchData(LED_Matrix_Handle_t* hmatrix);

/* Frame buffer operations */
void LED_Matrix_ClearBuffer(LED_Matrix_Handle_t* hmatrix);
void LED_Matrix_SetPixel(LED_Matrix_Handle_t* hmatrix, uint8_t row, uint8_t col, LED_Matrix_Color_t color);
void LED_Matrix_SetPixelRGB(LED_Matrix_Handle_t* hmatrix, uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b);
LED_Matrix_Color_t LED_Matrix_GetPixel(LED_Matrix_Handle_t* hmatrix, uint8_t row, uint8_t col);

/* Display refresh - blocking mode (for testing) */
void LED_Matrix_RefreshRow(LED_Matrix_Handle_t* hmatrix, uint8_t row);
void LED_Matrix_RefreshDisplay(LED_Matrix_Handle_t* hmatrix, uint16_t duration_ms);

/* Display refresh - interrupt mode (for production) */
void LED_Matrix_RefreshISR(LED_Matrix_Handle_t* hmatrix);

/* Drawing functions */
void LED_Matrix_DrawLine(LED_Matrix_Handle_t* hmatrix, int x0, int y0, int x1, int y1, LED_Matrix_Color_t color);
void LED_Matrix_DrawCross(LED_Matrix_Handle_t* hmatrix, uint8_t center_row, uint8_t center_col, LED_Matrix_Color_t color);
void LED_Matrix_DrawDiagonal(LED_Matrix_Handle_t* hmatrix, LED_Matrix_Color_t color);
void LED_Matrix_DrawPendulum(LED_Matrix_Handle_t* hmatrix, float theta, LED_Matrix_Color_t color);

/* Test functions */
void LED_Matrix_Test_RowScan(LED_Matrix_Handle_t* hmatrix);
void LED_Matrix_Test_ColumnScan(LED_Matrix_Handle_t* hmatrix, uint8_t test_row);
void LED_Matrix_Test_AllWhite(LED_Matrix_Handle_t* hmatrix);
void LED_Matrix_Test_SingleLED(LED_Matrix_Handle_t* hmatrix, uint8_t row, uint8_t col, LED_Matrix_Color_t color);

/* Utility */
void LED_Matrix_Delay_us(uint32_t us);

/* DM163 driver functions */
void DM163_SendByte(uint8_t data);
void DM163_LoadRowData(LED_Matrix_Handle_t* hmatrix, uint8_t row);

/* Debug function */
extern void Debug_Printf(const char* format, ...);

#endif /* LED_MATRIX_H_ */
