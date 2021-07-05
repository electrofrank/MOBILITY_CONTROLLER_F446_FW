/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "crc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "retarget.h"
#include <stdio.h>
#include "Odrive_CanIf.h"
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
uint8_t r[4];
float Vel;
int Steer_deg;

uint8_t cnt_val_8msb = 0;
uint32_t cnt_val = 0, cnt_val_prev = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void send_Steer_Deg(int arg);
void send_Odrive_vel(float arg);
void Odrive_status_check(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_USART2_UART_Init();
	MX_CAN1_Init();
	MX_CRC_Init();
	MX_TIM3_Init();
	/* USER CODE BEGIN 2 */
	RetargetInit(&huart2);
	printf("Configuring Can Register ...\n");

	uint8_t Timeout = 20;
	uint8_t tx_buffer[50];
	uint8_t tx_buff_size;

	tx_buff_size = sprintf((char*) tx_buffer, "HEllo word \n");
	HAL_UART_Transmit(&huart2, tx_buffer, tx_buff_size, Timeout);

	TxHeader_Config();
	CanRxFilterConfig();

	HAL_CAN_Start(&hcan1);
	if ((HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)
			== HAL_OK))
		printf("CAN Configuring: DONE !\n\n");
	else
		printf("Can non parte\n");

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	pTxHeader.IDE = CAN_ID_STD;
	pTxHeader.StdId = CanID_Generate(0x5, ODR_Reboot);

	//  int state = 8;

	r[0] = 0;
	r[1] = 0;
	r[2] = 0;
	r[3] = 0;

	if (HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, r, &TxMailbox) == HAL_OK) {
		HAL_UART_Transmit(&huart2, (uint8_t*) "Odrv_reset\n", 12, 10);
	}

	HAL_Delay(100);

	Odrive_status_check(); //check the status of the odrives before starting to send velocity data






	int i= 0;
	int j = 0;

//quasi sempre parte, ogni tanto un asse si pianta con errore sul controllere SPINOUT_DETECTED

	/*The motor mechanical power and electrical power do not agree.
	 *  This is usually caused by a slipping encoder or incorrect encoder offset calibration.
	 *   Check that your encoder is not slipping on the motor.
	 *   If using an Index pin, check that you are not getting false index pulses caused by noise.
	 *  This can happen if you are using unshielded cable for the encoder signals.
	 *
	 */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//before send the velocity, the actuAL STATUS SHOULD BE CHECKED
		send_Odrive_vel(10);

		send_Steer_Deg(0);

		HAL_Delay(300);

//	  // SWITCH OFF
//	  pTxHeader.IDE = CAN_ID_STD;
//	  pTxHeader.StdId = CanID_Generate(0x5, ODR_Set_InputVel);
//
//	  Vel = 0.0;
//
//	  *(float*)r = Vel;  // float to 4 bytes
//
//	  if(HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, r, &TxMailbox) == HAL_OK)
//	  {
//		HAL_UART_Transmit(&huart2, (uint8_t*)"S1_f - " , 6, 10);
//	  }
//
//	  pTxHeader.StdId = CanID_Generate(0x6, ODR_Set_InputVel);
//	  if(HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, r, &TxMailbox) == HAL_OK)
//	  {
//		HAL_UART_Transmit(&huart2, (uint8_t*)"S2_f\n" , 6, 10);
//	  }
//
//
//	  HAL_Delay(500);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 128;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)

{

	if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pRxHeader, r) == HAL_OK)
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

	if ((pRxHeader.StdId == 0x0A1) && (pRxHeader.IDE == CAN_ID_STD)
			&& (pRxHeader.DLC == 8)) {
		HAL_UART_Transmit(&huart2, (uint8_t*) "Odrv3\n", 7, 10);
	}

	if ((pRxHeader.ExtId == 0x00000154) && (pRxHeader.IDE == CAN_ID_EXT)
			&& (pRxHeader.DLC == 4)) {
		HAL_UART_Transmit(&huart2, (uint8_t*) "StpBrd\n", 8, 10);
	}
}

void send_Steer_Deg(int arg) {
	pTxHeader.IDE = CAN_ID_STD;
	pTxHeader.StdId = 0x140;

	int Steer_deg = arg;

	*(int*) r = Steer_deg;	 // float to bytes

	if (HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, r, &TxMailbox) == HAL_OK) {
		HAL_UART_Transmit(&huart2, (uint8_t*) "steer_to_stepper\n -", 18, 10);
	}
}

void send_Odrive_vel(float arg) {

	pTxHeader.IDE = CAN_ID_STD;
	pTxHeader.StdId = CanID_Generate(0x5, ODR_Set_InputVel);

	Vel = arg;

	*(float*) r = Vel;	 // float to bytes

	if (HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, r, &TxMailbox) == HAL_OK) {
		HAL_UART_Transmit(&huart2, (uint8_t*) "S1_o -", 4, 10);
	}

	pTxHeader.StdId = CanID_Generate(0x6, ODR_Set_InputVel);
	if (HAL_CAN_AddTxMessage(&hcan1, &pTxHeader, r, &TxMailbox) == HAL_OK) {
		HAL_UART_Transmit(&huart2, (uint8_t*) "S2_o\n", 6, 10);
	}
}

void Odrive_status_check(void) {
	printf("Checking Odrive Status...\n");
	pTxHeader.IDE = CAN_ID_STD;
	pTxHeader.StdId = CanID_Generate(0x5, ODR_Set_InputVel);


}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	printf("Someting wrong in Initialization !");
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
