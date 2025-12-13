#include "lcd_display.h"
#include "rip_config.h"
#include "fonts.h"
#include "tft.h"
#include "functions.h"
#include <stdio.h>

extern void Debug_Printf(const char* format, ...);

/* External variables from main.c */
extern float alpha, alpha_dot, theta, theta_dot;
extern PendulumState state;

/* Colors */
#define COLOR_BG            BLACK
#define COLOR_TEXT          WHITE
#define COLOR_VALUE         YELLOW
#define COLOR_STATE_SWING   RED
#define COLOR_STATE_STAB    GREEN
#define COLOR_TITLE         BLUE
#define COLOR_SECTION       CYAN

/* Layout */
#define Y_HEADER            18
#define Y_ARM_LABEL         38
#define Y_ARM_POS_L         58
#define Y_ARM_POS_V         78
#define Y_ARM_VEL_L         100
#define Y_ARM_VEL_V         120
#define Y_PEN_LABEL         145
#define Y_PEN_POS_L         165
#define Y_PEN_POS_V         185
#define Y_PEN_VEL_L         207
#define Y_PEN_VEL_V         227
#define Y_STATE_LABEL       250

/**
 * @brief  Protected text write - disables interrupts during write
 */
static void LCD_ProtectedWrite(uint16_t y_pos, uint16_t color, const char *text)
{
    uint32_t primask = __get_PRIMASK();  // Save interrupt state
    __disable_irq();                      // Disable interrupts

    setTextSize(1);
    setTextColor(color);
    setCursor(10, y_pos);
    printstr((uint8_t*)text);

    __set_PRIMASK(primask);              // Restore interrupt state
}

/**
 * @brief  Protected state write - disables interrupts during write
 */
static void LCD_ProtectedStateWrite(uint16_t color, const char *text)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    setCursor(0, Y_STATE_LABEL);
    setTextColor(color);
    printstr((uint8_t*)text);

    __set_PRIMASK(primask);
}

/**
 * @brief  Initialize LCD
 */
HAL_StatusTypeDef LCD_Display_Init(LCD_Handle_t *hlcd)
{
    char buffer[32];

    if (hlcd == NULL) {
        Debug_Printf("LCD_Init: NULL handle\r\n");
        return HAL_ERROR;
    }

    Debug_Printf("LCD_Init: Starting...\r\n");

    hlcd->initialized = 0;
    hlcd->update_counter = 0;
    hlcd->update_divider = LCD_UPDATE_DIVIDER;

    HAL_Delay(100);

    /* Read LCD ID */
    hlcd->lcd_id = readID();
    Debug_Printf("LCD_Init: LCD ID = 0x%04X\r\n", hlcd->lcd_id);

    if (hlcd->lcd_id == 0x0000 || hlcd->lcd_id == 0xFFFF) {
        Debug_Printf("LCD_Init: FAILED\r\n");
        return HAL_ERROR;
    }

    /* Disable interrupts during init */
    __disable_irq();

    /* Initialize TFT */
    tft_init(hlcd->lcd_id);
    setRotation(0);

    /* Draw static layout */
    fillScreen(COLOR_BG);

    sprintf(buffer, "INVERTED PENDULUM");
    printnewtstr(Y_HEADER, COLOR_TITLE, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "ARM:");
    printnewtstr(Y_ARM_LABEL, COLOR_SECTION, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "Position:");
    printnewtstr(Y_ARM_POS_L, COLOR_TEXT, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "Velocity:");
    printnewtstr(Y_ARM_VEL_L, COLOR_TEXT, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "PENDULUM:");
    printnewtstr(Y_PEN_LABEL, COLOR_SECTION, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "Position:");
    printnewtstr(Y_PEN_POS_L, COLOR_TEXT, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "Velocity:");
    printnewtstr(Y_PEN_VEL_L, COLOR_TEXT, &mono12x7bold, 1, (uint8_t*)buffer);

    sprintf(buffer, "STATE:");
    printnewtstr(Y_STATE_LABEL, COLOR_SECTION, &mono12x7bold, 1, (uint8_t*)buffer);

    __enable_irq();  // Re-enable interrupts

    hlcd->initialized = 1;
    HAL_Delay(100);

    Debug_Printf("LCD_Init: SUCCESS!\r\n");
    return HAL_OK;
}

/**
 * @brief  Update display with interrupt protection
 */
void LCD_Display_Update(LCD_Handle_t *hlcd)
{
    char buffer[32];
    int int_part, frac_part;

    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }

    /* ARM POSITION */
    fillRect(0, Y_ARM_POS_V - 15, 240, 20, COLOR_BG);
    int_part = (int)theta;
    frac_part = (int)((theta - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d rad", int_part, frac_part);
    LCD_ProtectedWrite(Y_ARM_POS_V, COLOR_VALUE, buffer);

    /* ARM VELOCITY */
    fillRect(0, Y_ARM_VEL_V - 15, 240, 20, COLOR_BG);
    int_part = (int)theta_dot;
    frac_part = (int)((theta_dot - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d r/s", int_part, frac_part);
    LCD_ProtectedWrite(Y_ARM_VEL_V, COLOR_VALUE, buffer);

    /* PENDULUM POSITION */
    fillRect(0, Y_PEN_POS_V - 15, 240, 20, COLOR_BG);
    int_part = (int)alpha;
    frac_part = (int)((alpha - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d rad", int_part, frac_part);
    LCD_ProtectedWrite(Y_PEN_POS_V, COLOR_VALUE, buffer);

    /* PENDULUM VELOCITY */
    fillRect(0, Y_PEN_VEL_V - 15, 240, 20, COLOR_BG);
    int_part = (int)alpha_dot;
    frac_part = (int)((alpha_dot - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d r/s", int_part, frac_part);
    LCD_ProtectedWrite(Y_PEN_VEL_V, COLOR_VALUE, buffer);

    /* STATE */
    fillRect(0, Y_STATE_LABEL - 15, 240, 20, COLOR_BG);

    if (state == STATE_LQR) {
        LCD_ProtectedStateWrite(COLOR_STATE_STAB, "STATE:STABILIZE");
    } else if (state == STATE_KICK || state == STATE_SWINGUP) {
        LCD_ProtectedStateWrite(COLOR_STATE_SWING, "STATE:SWING UP");
    } else if (state == STATE_EMERGENCY) {
        LCD_ProtectedStateWrite(RED, "STATE:EMERGENCY");
    } else {
        LCD_ProtectedStateWrite(YELLOW, "STATE:WAITING");
    }
}

uint8_t LCD_Display_ShouldUpdate(LCD_Handle_t *hlcd) {
    if (hlcd == NULL || !hlcd->initialized) {
        return 0;
    }
    return (hlcd->update_counter == 0);
}

void LCD_Display_IncrementCounter(LCD_Handle_t *hlcd) {
    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }
    if (++hlcd->update_counter >= hlcd->update_divider) {
        hlcd->update_counter = 0;
    }
}

void LCD_Display_SetUpdateRate(LCD_Handle_t *hlcd, uint16_t divider) {
    if (hlcd == NULL) {
        return;
    }
    if (divider > 0) {
        hlcd->update_divider = divider;
        hlcd->update_counter = 0;
    }
}

void LCD_Display_Clear(LCD_Handle_t *hlcd) {
    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }
    __disable_irq();
    fillScreen(COLOR_BG);
    __enable_irq();
}
