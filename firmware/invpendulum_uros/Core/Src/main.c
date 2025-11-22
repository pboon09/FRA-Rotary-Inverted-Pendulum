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
#include "cmsis_os.h"
#include "dma.h"
#include "iwdg.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rip_config.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>

#include <sensor_msgs/msg/joint_state.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>

#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define RCLSOFTCHECK(fn) if (fn != RCL_RET_OK) {};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_init_options_t init_options;
rclc_executor_t executor;

rcl_timer_t timer;
const int timeout_ms = 10;
const unsigned int timer_period = RCL_MS_TO_NS(10);

rcl_publisher_t publisher;
sensor_msgs__msg__JointState joint_msg;

float cmd_energy_norm, cmd_lqr_volt;
int total_cmd, cmd_energy, cmd_lqr, cmd_kick;

float alpha, alpha_shifted, alpha_dot, theta, theta_dot, voltage_input;

typedef enum {
	STATE_WAIT_BUTTON, STATE_KICK, STATE_SWINGUP, STATE_LQR, STATE_EMERGENCY
} PendulumState;

PendulumState state = STATE_WAIT_BUTTON;
int kick_counter = 0;

int debug, emer, released, led_counter = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
bool cubemx_transport_open(struct uxrCustomTransport *transport);
bool cubemx_transport_close(struct uxrCustomTransport *transport);
size_t cubemx_transport_write(struct uxrCustomTransport *transport,
		const uint8_t *buf, size_t len, uint8_t *err);
size_t cubemx_transport_read(struct uxrCustomTransport *transport, uint8_t *buf,
		size_t len, int timeout, uint8_t *err);

void* microros_allocate(size_t size, void *state);
void microros_deallocate(void *pointer, void *state);
void* microros_reallocate(void *pointer, size_t size, void *state);
void* microros_zero_allocate(size_t number_of_elements, size_t size_of_element,
		void *state);

static inline float wrap_pi(float x);
static inline float wrap_2pi(float x);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
	if (timer != NULL) {
		// Sync micro-ROS session
		rmw_uros_sync_session(timeout_ms);

        int64_t ns = rmw_uros_epoch_nanos();
        if (ns > 0) {
            joint_msg.header.stamp.sec     = (int32_t)(ns / 1000000000LL);
            joint_msg.header.stamp.nanosec = (uint32_t)(ns % 1000000000LL);
        } else {
            joint_msg.header.stamp.sec = 0;
            joint_msg.header.stamp.nanosec = 0;
        }

        // Update joint states
        joint_msg.position.data[0] = theta;
        joint_msg.velocity.data[0] = theta_dot;
        joint_msg.position.data[1] = alpha;
        joint_msg.velocity.data[1] = alpha_dot;

        // Publish the joint state
        RCLSOFTCHECK(rcl_publish(&publisher, &joint_msg, NULL));

        HAL_IWDG_Refresh(&hiwdg);
	}
}

void StartDefaultTask(void *argument) {

	// micro-ROS configuration
	rmw_uros_set_custom_transport(
	true, (void*) &hlpuart1, cubemx_transport_open, cubemx_transport_close,
			cubemx_transport_write, cubemx_transport_read);

	rcl_allocator_t freeRTOS_allocator =
			rcutils_get_zero_initialized_allocator();
	freeRTOS_allocator.allocate = microros_allocate;
	freeRTOS_allocator.deallocate = microros_deallocate;
	freeRTOS_allocator.reallocate = microros_reallocate;
	freeRTOS_allocator.zero_allocate = microros_zero_allocate;

	if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
		printf("Error on default allocators (line %d)\n", __LINE__);
	}

	allocator = rcl_get_default_allocator();

	//create init_options
	init_options = rcl_get_zero_initialized_init_options();
	RCLSOFTCHECK(rcl_init_options_init(&init_options, allocator));
	RCLSOFTCHECK(rcl_init_options_set_domain_id(&init_options, 99));

	rclc_support_init_with_options(&support, 0, NULL, &init_options,
			&allocator);

	// create node
	rclc_node_init_default(&node, "inv_pendulum", "", &support);

    sensor_msgs__msg__JointState__init(&joint_msg);

    // name[]
    rosidl_runtime_c__String__Sequence__init(&joint_msg.name, 2);
    rosidl_runtime_c__String__assign(&joint_msg.name.data[0], "joint_base_to_arm");
    rosidl_runtime_c__String__assign(&joint_msg.name.data[1], "joint_arm_to_pendulum");

    // position[] and velocity[]
    rosidl_runtime_c__double__Sequence__init(&joint_msg.position, 2);
    rosidl_runtime_c__double__Sequence__init(&joint_msg.velocity, 2);

	// create publisher
    RCLSOFTCHECK(rclc_publisher_init_default(
        &publisher, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
        "joint_states_raw"));

	// create timer
	rclc_timer_init_default(&timer, &support, timer_period, timer_callback);

	// create executor
	executor = rclc_executor_get_zero_initialized_executor();
	rclc_executor_init(&executor, &support.context, 1, &allocator);
	rclc_executor_add_timer(&executor, &timer);
	rclc_executor_spin(&executor);
}
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
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  MX_IWDG_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	config_begin();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		emer = HAL_GPIO_ReadPin(emergency_GPIO_Port, emergency_Pin);
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


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == emergency_Pin) {
		state = STATE_EMERGENCY;
		released = 0;
	}
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	if (htim == &htim2) {
		QEI_get_diff_count(&motor_encoder);
		QEI_compute_data(&motor_encoder);
		QEI_get_diff_count(&pendulum_encoder);
		QEI_compute_data(&pendulum_encoder);

		total_cmd = cmd_lqr + cmd_energy + cmd_kick;

		voltage_input = total_cmd * MOTOR_VOLTAGE_LIMIT / 65535.0;

		alpha = wrap_2pi(pendulum_encoder.rads);
		alpha_dot = FIR_process(&alpha_dot_filter, pendulum_encoder.radps);
		theta = motor_encoder.rads;
//		theta_dot = FIR_process(&theta_dot_filter, motor_encoder.radps);
		theta_dot = kf_update(&motor_filter, voltage_input, theta);

		alpha_shifted = wrap_pi(alpha - M_PI);

		switch (state) {
		case STATE_WAIT_BUTTON:
			if (emer) {
				state = STATE_EMERGENCY;
			}

			cmd_lqr = 0;
			if (++led_counter >= 100) {
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				led_counter = 0;
			}
			if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
				kick_counter = 0;
				state = STATE_KICK;
				QEI_reset(&pendulum_encoder);
				QEI_reset(&motor_encoder);
				kf_clear(&motor_filter);
				total_cmd = 0;
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
			}

			MDXX_set_range(&motor, 2000, 0);
			break;
		case STATE_KICK:
			kick_counter++;
			cmd_kick = (kick_counter < 250) ? 6000 : -6000;

			if (kick_counter < 500) {
				MDXX_set_range(&motor, 2000, cmd_kick);
			} else {
				cmd_kick = 0;
				state = STATE_SWINGUP;
			}
			break;
		case STATE_SWINGUP:
			if (fabsf(alpha_shifted) < 0.25) {
				cmd_energy = 0;
				cmd_kick = 0;
				state = STATE_LQR;
			} else {
				cmd_energy_norm = EnergyCtrl_Update(&swingup, alpha, alpha_dot);
				cmd_energy = (int) (cmd_energy_norm * 6000.0f);
				MDXX_set_range(&motor, 2000, cmd_energy);
			}
			break;
		case STATE_LQR:
			if (fabsf(alpha_shifted) > 0.5f) {
				cmd_energy = 0;
				state = STATE_WAIT_BUTTON;
				MDXX_set_range(&motor, 2000, 0);
			} else {
				LQR_SetState(&lqr_ctrl, theta, alpha_shifted, theta_dot,
						alpha_dot);
				cmd_lqr_volt = LQR_Update(&lqr_ctrl);
				cmd_lqr = (int) (cmd_lqr_volt * 65535.0f / MOTOR_VOLTAGE_LIMIT);
				MDXX_set_range(&motor, 2000, cmd_lqr);
			}
			break;
		case STATE_EMERGENCY:
			cmd_lqr = 0;
			MDXX_set_range(&motor, 2000, 0);
			QEI_reset(&motor_encoder);
			QEI_reset(&pendulum_encoder);
			kf_clear(&motor_filter);
			total_cmd = 0;
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

			if (emer == 0) {
				released = 1;
			}

			if (released
					&& HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
				state = STATE_WAIT_BUTTON;
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				led_counter = 0;
				released = 0;
				debug = 0;
			}
			break;
		}
	}
  /* USER CODE END Callback 1 */
}

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
