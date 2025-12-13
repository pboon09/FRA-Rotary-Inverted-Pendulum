/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define RCC_OSC_IN_Pin GPIO_PIN_0
#define RCC_OSC_IN_GPIO_Port GPIOF
#define RCC_OSC_OUT_Pin GPIO_PIN_1
#define RCC_OSC_OUT_GPIO_Port GPIOF
#define SDA_Pin GPIO_PIN_0
#define SDA_GPIO_Port GPIOC
#define SCK_Pin GPIO_PIN_1
#define SCK_GPIO_Port GPIOC
#define C7_Pin GPIO_PIN_2
#define C7_GPIO_Port GPIOC
#define C6_Pin GPIO_PIN_3
#define C6_GPIO_Port GPIOC
#define RSTn_Pin GPIO_PIN_0
#define RSTn_GPIO_Port GPIOA
#define LAT_Pin GPIO_PIN_1
#define LAT_GPIO_Port GPIOA
#define LPUART1_TX_Pin GPIO_PIN_2
#define LPUART1_TX_GPIO_Port GPIOA
#define LPUART1_RX_Pin GPIO_PIN_3
#define LPUART1_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define LCD_D1_Pin GPIO_PIN_4
#define LCD_D1_GPIO_Port GPIOC
#define LCD_D4_Pin GPIO_PIN_5
#define LCD_D4_GPIO_Port GPIOC
#define SB_Pin GPIO_PIN_0
#define SB_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_1
#define SD_CS_GPIO_Port GPIOB
#define LCD_D7_Pin GPIO_PIN_2
#define LCD_D7_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_10
#define LCD_RS_GPIO_Port GPIOB
#define LCD_D6_Pin GPIO_PIN_11
#define LCD_D6_GPIO_Port GPIOB
#define LCD_D5_Pin GPIO_PIN_12
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D3_Pin GPIO_PIN_6
#define LCD_D3_GPIO_Port GPIOC
#define LCD_WR_Pin GPIO_PIN_7
#define LCD_WR_GPIO_Port GPIOC
#define LCD_D2_Pin GPIO_PIN_8
#define LCD_D2_GPIO_Port GPIOC
#define emergency_Pin GPIO_PIN_9
#define emergency_GPIO_Port GPIOC
#define LCD_D0_Pin GPIO_PIN_10
#define LCD_D0_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define C1_Pin GPIO_PIN_15
#define C1_GPIO_Port GPIOA
#define C3_Pin GPIO_PIN_10
#define C3_GPIO_Port GPIOC
#define C5_Pin GPIO_PIN_11
#define C5_GPIO_Port GPIOC
#define C2_Pin GPIO_PIN_12
#define C2_GPIO_Port GPIOC
#define C4_Pin GPIO_PIN_2
#define C4_GPIO_Port GPIOD
#define T_SWO_Pin GPIO_PIN_3
#define T_SWO_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_5
#define LCD_RST_GPIO_Port GPIOB
#define LCD_RD_Pin GPIO_PIN_6
#define LCD_RD_GPIO_Port GPIOB
#define C0_Pin GPIO_PIN_7
#define C0_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
