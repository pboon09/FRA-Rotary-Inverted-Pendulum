#include "lcd_display.h"
#include "rip_config.h"
#include "fonts.h"
#include "tft.h"
#include "functions.h"
#include <stdio.h>
#include <string.h>

extern void Debug_Printf(const char* format, ...);

/* External variables from main.c */
extern float alpha, alpha_dot, theta, theta_dot;
extern volatile PendulumState state;

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

typedef struct {
    float alpha_snap;
    float alpha_dot_snap;
    float theta_snap;
    float theta_dot_snap;
    PendulumState state_snap;
    uint8_t data_ready;
} LCD_Snapshot_t;

static volatile LCD_Snapshot_t lcd_snapshot = {0};

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
        Debug_Printf("LCD_Init: FAILED - Invalid ID\r\n");
        return HAL_ERROR;
    }

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

    hlcd->initialized = 1;

    Debug_Printf("LCD_Init: SUCCESS!\r\n");
    return HAL_OK;
}

void LCD_Display_SnapshotData(LCD_Handle_t *hlcd)
{
    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }

    /* Quick snapshot - called from ISR */
    lcd_snapshot.alpha_snap = alpha;
    lcd_snapshot.alpha_dot_snap = alpha_dot;
    lcd_snapshot.theta_snap = theta;
    lcd_snapshot.theta_dot_snap = theta_dot;
    lcd_snapshot.state_snap = state;
    lcd_snapshot.data_ready = 1;
}

void LCD_Display_Update(LCD_Handle_t *hlcd)
{
    char buffer[32];
    int int_part, frac_part;

    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }

    if (!lcd_snapshot.data_ready) {
        return;
    }

    /* Mark as consumed */
    lcd_snapshot.data_ready = 0;

    /* Use snapshot data - no need to disable interrupts */
    float alpha_val = lcd_snapshot.alpha_snap;
    float alpha_dot_val = lcd_snapshot.alpha_dot_snap;
    float theta_val = lcd_snapshot.theta_snap;
    float theta_dot_val = lcd_snapshot.theta_dot_snap;
    PendulumState state_val = lcd_snapshot.state_snap;

    /* ARM POSITION */
    fillRect(0, Y_ARM_POS_V - 15, 240, 20, COLOR_BG);
    int_part = (int)theta_val;
    frac_part = (int)((theta_val - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d rad", int_part, frac_part);
    setTextSize(1);
    setTextColor(COLOR_VALUE);
    setCursor(10, Y_ARM_POS_V);
    printstr((uint8_t*)buffer);

    /* ARM VELOCITY */
    fillRect(0, Y_ARM_VEL_V - 15, 240, 20, COLOR_BG);
    int_part = (int)theta_dot_val;
    frac_part = (int)((theta_dot_val - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d r/s", int_part, frac_part);
    setTextSize(1);
    setTextColor(COLOR_VALUE);
    setCursor(10, Y_ARM_VEL_V);
    printstr((uint8_t*)buffer);

    /* PENDULUM POSITION */
    fillRect(0, Y_PEN_POS_V - 15, 240, 20, COLOR_BG);
    int_part = (int)alpha_val;
    frac_part = (int)((alpha_val - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d rad", int_part, frac_part);
    setTextSize(1);
    setTextColor(COLOR_VALUE);
    setCursor(10, Y_PEN_POS_V);
    printstr((uint8_t*)buffer);

    /* PENDULUM VELOCITY */
    fillRect(0, Y_PEN_VEL_V - 15, 240, 20, COLOR_BG);
    int_part = (int)alpha_dot_val;
    frac_part = (int)((alpha_dot_val - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    sprintf(buffer, "%d.%02d r/s", int_part, frac_part);
    setTextSize(1);
    setTextColor(COLOR_VALUE);
    setCursor(10, Y_PEN_VEL_V);
    printstr((uint8_t*)buffer);

    /* STATE */
    fillRect(0, Y_STATE_LABEL - 15, 240, 20, COLOR_BG);
    setCursor(0, Y_STATE_LABEL);

    if (state_val == STATE_LQR) {
        setTextColor(COLOR_STATE_STAB);
        printstr((uint8_t*)"STATE:STABILIZE");
    } else if (state_val == STATE_KICK || state_val == STATE_SWINGUP) {
        setTextColor(COLOR_STATE_SWING);
        printstr((uint8_t*)"STATE:SWING UP");
    } else if (state_val == STATE_EMERGENCY) {
        setTextColor(RED);
        printstr((uint8_t*)"STATE:EMERGENCY");
    } else {
        setTextColor(YELLOW);
        printstr((uint8_t*)"STATE:WAITING");
    }
}

uint8_t LCD_Display_ShouldUpdate(LCD_Handle_t *hlcd)
{
    if (hlcd == NULL || !hlcd->initialized) {
        return 0;
    }
    return (hlcd->update_counter == 0 && lcd_snapshot.data_ready);
}

void LCD_Display_IncrementCounter(LCD_Handle_t *hlcd)
{
    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }
    if (++hlcd->update_counter >= hlcd->update_divider) {
        hlcd->update_counter = 0;
        /* Trigger snapshot on counter reset */
        LCD_Display_SnapshotData(hlcd);
    }
}

void LCD_Display_SetUpdateRate(LCD_Handle_t *hlcd, uint16_t divider)
{
    if (hlcd == NULL) {
        return;
    }
    if (divider > 0) {
        hlcd->update_divider = divider;
        hlcd->update_counter = 0;
    }
}

void LCD_Display_Clear(LCD_Handle_t *hlcd)
{
    if (hlcd == NULL || !hlcd->initialized) {
        return;
    }
    fillScreen(COLOR_BG);
    hlcd->initialized = 0;
}

uint8_t LCD_Display_IsInitialized(LCD_Handle_t *hlcd)
{
    if (hlcd == NULL) {
        return 0;
    }
    return hlcd->initialized;
}
