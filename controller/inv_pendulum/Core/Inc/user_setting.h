#ifndef USER_SETTING_H_
#define USER_SETTING_H_

// Control Pins
#define RD_PORT LCD_RD_GPIO_Port
#define RD_PIN  LCD_RD_Pin
#define WR_PORT LCD_WR_GPIO_Port
#define WR_PIN  LCD_WR_Pin
#define CD_PORT LCD_RS_GPIO_Port
#define CD_PIN  LCD_RS_Pin
#define CS_PORT LCD_CS_GPIO_Port
#define CS_PIN  LCD_CS_Pin
#define RESET_PORT LCD_RST_GPIO_Port
#define RESET_PIN  LCD_RST_Pin

// Data Pins D0-D7
#define D0_PORT LCD_D0_GPIO_Port
#define D0_PIN LCD_D0_Pin
#define D1_PORT LCD_D1_GPIO_Port
#define D1_PIN LCD_D1_Pin
#define D2_PORT LCD_D2_GPIO_Port
#define D2_PIN LCD_D2_Pin
#define D3_PORT LCD_D3_GPIO_Port
#define D3_PIN LCD_D3_Pin
#define D4_PORT LCD_D4_GPIO_Port
#define D4_PIN LCD_D4_Pin
#define D5_PORT LCD_D5_GPIO_Port
#define D5_PIN LCD_D5_Pin
#define D6_PORT LCD_D6_GPIO_Port
#define D6_PIN LCD_D6_Pin
#define D7_PORT LCD_D7_GPIO_Port
#define D7_PIN LCD_D7_Pin

#define  WIDTH    ((uint16_t)240)
#define  HEIGHT   ((uint16_t)320)

/****************** delay in microseconds ***********************/
extern TIM_HandleTypeDef htim1;

// EXACTLY like your working code
static inline void delay (uint32_t time)
{
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while ((__HAL_TIM_GET_COUNTER(&htim1))<time);
}

// Configure macros for the data pins
#define write_8(d) { \
   GPIOA->BSRR = 0b0000010000000000 << 16; \
   GPIOB->BSRR = 0b0001100000000100 << 16; \
   GPIOC->BSRR = 0b0000000101110000 << 16; \
   GPIOA->BSRR = (((d) & 0x01) << 10); \
   GPIOB->BSRR = (((d) & 0x20) << 7) \
               | (((d) & 0x40) << 5) \
               | (((d) & 0x80) >> 5); \
   GPIOC->BSRR = (((d) & 0x02) << 3) \
               | (((d) & 0x04) << 6) \
               | (((d) & 0x08) << 3) \
               | (((d) & 0x10) << 1); \
}

#define read_8() (          (((GPIOA->IDR & 0x0400) >> 10) \
                           | ((GPIOC->IDR & 0x0010) >> 3) \
                           | ((GPIOC->IDR & 0x0100) >> 6) \
                           | ((GPIOC->IDR & 0x0040) >> 3) \
                           | ((GPIOC->IDR & 0x0020) >> 1) \
                           | ((GPIOB->IDR & 0x1000) >> 7) \
                           | ((GPIOB->IDR & 0x0800) >> 5) \
                           | ((GPIOB->IDR & 0x0004) << 5)))

/************************** For STM32G474 (170 MHz) ****************************/
#define WRITE_DELAY { WR_ACTIVE8; }
#define READ_DELAY  { RD_ACTIVE8; }

/*****************************  DEFINES FOR DIFFERENT TFTs   ****************************************************/
#define SUPPORT_8347D

#endif /* USER_SETTING_H_ */
