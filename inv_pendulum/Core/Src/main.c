/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "iwdg.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rip_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float cmd_energy_norm, cmd_lqr_volt;
int cmd_energy, cmd_lqr;

int emer;

float alpha, alpha_shifted, alpha_dot, theta, theta_dot;

typedef enum {
	STATE_WAIT_BUTTON, STATE_KICK, STATE_SWINGUP, STATE_LQR
} PendulumState;

PendulumState state = STATE_WAIT_BUTTON;
int kick_counter = 0;

int debug;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static inline float wrap_pi(float x);
static inline float wrap_2pi(float x);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	config_begin();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_IWDG_Refresh(&hiwdg);
		emer = HAL_GPIO_ReadPin(emer_GPIO_Port, emer_Pin);
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static inline float wrap_pi(float x) {
	while (x > M_PI)
		x -= 2.0f * M_PI;
	while (x <= -M_PI)
		x += 2.0f * M_PI;
	return x;
}

static inline float wrap_2pi(float x) {
	while (x >= 2.0f * M_PI)
		x -= 2.0f * M_PI;
	while (x < 0.0f)
		x += 2.0f * M_PI;
	return x;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim2) {
		QEI_get_diff_count(&motor_encoder);
		QEI_compute_data(&motor_encoder);
		QEI_get_diff_count(&pendulum_encoder);
		QEI_compute_data(&pendulum_encoder);

		alpha = wrap_2pi(pendulum_encoder.rads);
		alpha_dot = FIR_process(&alpha_dot_filter, pendulum_encoder.radps);
		theta = motor_encoder.rads;
		theta_dot = FIR_process(&theta_dot_filter, motor_encoder.radps);

		alpha_shifted = wrap_pi(alpha - M_PI);

		switch (state) {
		case STATE_WAIT_BUTTON:
			if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
				kick_counter = 0;
				state = STATE_KICK;
				QEI_reset(&pendulum_encoder);
				QEI_reset(&motor_encoder);
			}
			debug = 0;
			MDXX_set_range(&motor, 2000, 0);
			break;
		case STATE_KICK:
			kick_counter++;
			if (kick_counter < 400) {
				MDXX_set_range(&motor, 2000,
						(kick_counter < 200) ? 6000 : -6000);
			} else {
				state = STATE_SWINGUP;
			}
			break;
		case STATE_SWINGUP:
			if (fabsf(alpha_shifted) < 0.5f) {
				debug = 1;
				state = STATE_LQR;
			} else {
				cmd_energy_norm = EnergyCtrl_Update(&swingup, alpha, alpha_dot);
				cmd_energy = (int) (cmd_energy_norm * 6000.0f);
				MDXX_set_range(&motor, 2000, cmd_energy);
			}
			break;
		case STATE_LQR:
			if (fabsf(alpha_shifted) > 0.5f) {
				state = STATE_WAIT_BUTTON;
				MDXX_set_range(&motor, 2000, 0);
			} else {
				LQR_SetState(&lqr_ctrl, theta, alpha_shifted, theta_dot,
						alpha_dot);
				cmd_lqr_volt = LQR_Update(&lqr_ctrl);
				cmd_lqr = (int) (cmd_lqr_volt * 65535.0f / 24.0f);
				MDXX_set_range(&motor, 2000, cmd_lqr);
			}
//			MDXX_set_range(&motor, 2000, 0);
			break;
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
