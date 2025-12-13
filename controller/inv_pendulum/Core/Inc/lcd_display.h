#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "main.h"

/* LCD Handle Structure */
typedef struct {
    uint16_t lcd_id;
    uint8_t initialized;
    uint16_t update_counter;
    uint16_t update_divider;
} LCD_Handle_t;

/* Function prototypes */
HAL_StatusTypeDef LCD_Display_Init(LCD_Handle_t *hlcd);
void LCD_Display_Update(LCD_Handle_t *hlcd);
uint8_t LCD_Display_ShouldUpdate(LCD_Handle_t *hlcd);
void LCD_Display_IncrementCounter(LCD_Handle_t *hlcd);
void LCD_Display_SetUpdateRate(LCD_Handle_t *hlcd, uint16_t divider);
void LCD_Display_Clear(LCD_Handle_t *hlcd);

#endif /* LCD_DISPLAY_H */
