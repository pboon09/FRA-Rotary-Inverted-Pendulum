/*
 * user_setting.h
 *
 *  Modified for STM32G474 with LCD Shield
 *  LCD Pin Mapping based on CubeMX configuration
 */

#ifndef USER_SETTING_H_
#define USER_SETTING_H_

// Control Pins - ตรงกับ main.h
#define RD_PORT LCD_RD_GPIO_Port      // GPIOB
#define RD_PIN  LCD_RD_Pin            // GPIO_PIN_6
#define WR_PORT LCD_WR_GPIO_Port      // GPIOC
#define WR_PIN  LCD_WR_Pin            // GPIO_PIN_7
#define CD_PORT LCD_RS_GPIO_Port      // RS PORT (GPIOB)
#define CD_PIN  LCD_RS_Pin            // RS PIN (GPIO_PIN_10)
#define CS_PORT LCD_CS_GPIO_Port      // GPIOB
#define CS_PIN  LCD_CS_Pin            // GPIO_PIN_4
#define RESET_PORT LCD_RST_GPIO_Port  // GPIOB
#define RESET_PIN  LCD_RST_Pin        // GPIO_PIN_5

// Data Pins D0-D7 - ตรงกับ main.h
#define D0_PORT LCD_D0_GPIO_Port      // GPIOA
#define D0_PIN LCD_D0_Pin             // GPIO_PIN_10
#define D1_PORT LCD_D1_GPIO_Port      // GPIOC
#define D1_PIN LCD_D1_Pin             // GPIO_PIN_4
#define D2_PORT LCD_D2_GPIO_Port      // GPIOC
#define D2_PIN LCD_D2_Pin             // GPIO_PIN_8
#define D3_PORT LCD_D3_GPIO_Port      // GPIOC
#define D3_PIN LCD_D3_Pin             // GPIO_PIN_6
#define D4_PORT LCD_D4_GPIO_Port      // GPIOC
#define D4_PIN LCD_D4_Pin             // GPIO_PIN_5
#define D5_PORT LCD_D5_GPIO_Port      // GPIOB
#define D5_PIN LCD_D5_Pin             // GPIO_PIN_12
#define D6_PORT LCD_D6_GPIO_Port      // GPIOB
#define D6_PIN LCD_D6_Pin             // GPIO_PIN_11
#define D7_PORT LCD_D7_GPIO_Port      // GPIOB
#define D7_PIN LCD_D7_Pin             // GPIO_PIN_2

#define  WIDTH    ((uint16_t)240)
#define  HEIGHT   ((uint16_t)320)

/****************** delay in microseconds ***********************/
extern TIM_HandleTypeDef htim1;

// ใช้ static inline เพื่อป้องกัน multiple definition error
static inline void delay (uint32_t time)
{
	/* change your code here for the delay in microseconds */
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while ((__HAL_TIM_GET_COUNTER(&htim1))<time);
}

// Configure macros for the data pins
/*
 * Data Pin Mapping จาก main.h:
 * D0 -> PA10 (bit 10)
 * D1 -> PC4  (bit 4)
 * D2 -> PC8  (bit 8)
 * D3 -> PC6  (bit 6)
 * D4 -> PC5  (bit 5)
 * D5 -> PB12 (bit 12)
 * D6 -> PB11 (bit 11)
 * D7 -> PB2  (bit 2)
 */

/*
 * write_8(d): เขียนข้อมูล 8-bit ไปยัง data pins
 *
 * การคำนวณ:
 * D0 (bit 0 of d) -> PA10: (d & 0x01) << 10
 * D1 (bit 1 of d) -> PC4:  (d & 0x02) << 3  (เพราะ bit 1 -> bit 4 = shift left 3)
 * D2 (bit 2 of d) -> PC8:  (d & 0x04) << 6  (เพราะ bit 2 -> bit 8 = shift left 6)
 * D3 (bit 3 of d) -> PC6:  (d & 0x08) << 3  (เพราะ bit 3 -> bit 6 = shift left 3)
 * D4 (bit 4 of d) -> PC5:  (d & 0x10) << 1  (เพราะ bit 4 -> bit 5 = shift left 1)
 * D5 (bit 5 of d) -> PB12: (d & 0x20) << 7  (เพราะ bit 5 -> bit 12 = shift left 7)
 * D6 (bit 6 of d) -> PB11: (d & 0x40) << 5  (เพราะ bit 6 -> bit 11 = shift left 5)
 * D7 (bit 7 of d) -> PB2:  (d & 0x80) >> 5  (เพราะ bit 7 -> bit 2 = shift right 5)
 */
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

/*
 * read_8(): อ่านข้อมูล 8-bit จาก data pins
 *
 * การคำนวณ:
 * D0 (bit 0) <- PA10: (IDR & 0x0400) >> 10
 * D1 (bit 1) <- PC4:  (IDR & 0x0010) >> 3
 * D2 (bit 2) <- PC8:  (IDR & 0x0100) >> 6
 * D3 (bit 3) <- PC6:  (IDR & 0x0040) >> 3
 * D4 (bit 4) <- PC5:  (IDR & 0x0020) >> 1
 * D5 (bit 5) <- PB12: (IDR & 0x1000) >> 7
 * D6 (bit 6) <- PB11: (IDR & 0x0800) >> 5
 * D7 (bit 7) <- PB2:  (IDR & 0x0004) << 5
 */
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
/*
 * Arduino 2.4" LCD Shield โดยทั่วไปใช้:
 * - ILI9341 (ยอดนิยม) - ไม่มีใน library นี้
 * - HX8347-D (ทางเลือกที่ 2) - ลองนี้ก่อน
 * - ILI9325/ILI9328 - อาจใช้ SUPPORT_9326_5420
 *
 * วิธีทดสอบ: เปิด SUPPORT_8347D ก่อน ถ้าไม่ได้ลอง SUPPORT_9342 หรือ SUPPORT_9326_5420
 */

//#define SUPPORT_0139              //S6D0139 +280 bytes
//#define SUPPORT_0154              //S6D0154 +320 bytes
//#define SUPPORT_1289              //SSD1289,SSD1297 (ID=0x9797) +626 bytes, 0.03s
//#define SUPPORT_1580              //R61580 Untested
//#define SUPPORT_1963              //only works with 16BIT bus anyway
//#define SUPPORT_4532              //LGDP4532 +120 bytes.  thanks Leodino
//#define SUPPORT_4535              //LGDP4535 +180 bytes
//#define SUPPORT_68140             //RM68140 +52 bytes defaults to PIXFMT=0x55
//#define SUPPORT_7735
//#define SUPPORT_7781              //ST7781 +172 bytes
//#define SUPPORT_8230              //UC8230 +118 bytes

// ลำดับการทดสอบสำหรับ Arduino 2.4" LCD Shield:
#define SUPPORT_8347D             // 1. ทดสอบ HX8347-D ก่อน (ใช้กับ shield บางรุ่น)
//#define SUPPORT_9342              // 2. ถ้าไม่ได้ ลอง ILI9342 (ใกล้เคียง ILI9341)
//#define SUPPORT_9326_5420         // 3. ถ้ายังไม่ได้ ลอง ILI9326

//#define SUPPORT_8347A             //HX8347-A +500 bytes, 0.27s
//#define SUPPORT_8352A             //HX8352A +486 bytes, 0.27s
//#define SUPPORT_8352B             //HX8352B
//#define SUPPORT_8357D_GAMMA       //monster 34 byte
//#define SUPPORT_9163              //
//#define SUPPORT_9225              //ILI9225-B, ILI9225-G ID=0x9225, ID=0x9226, ID=0x6813 +380 bytes
//#define SUPPORT_9806              //UNTESTED
//#define SUPPORT_9488_555          //costs +230 bytes, 0.03s / 0.19s
//#define SUPPORT_B509_7793         //R61509, ST7793 +244 bytes
//#define OFFSET_9327 32            //costs about 103 bytes, 0.08s

#endif /* USER_SETTING_H_ */
