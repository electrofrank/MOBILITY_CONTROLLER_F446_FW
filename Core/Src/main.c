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
#include "stdbool.h"
#include <math.h>
#include "Odrive_CanIf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AllSystem_IN_BUSY              1
#define AllSystem_IN_NO_ACTIVE_CHILD   0
#define AllSystem_IN_RISE_INPUT_ERROR2 1
#define AllSystem_IN_SPINNING          2
#define AllSystem_IN_STEERING          3
#define AllSystem_IN_STOP              4
#define AllSystem_IN_STRAIGHT_DRIVE    5
#define AllSystem_IN_Safe_drive        2
#define AllSystem_range                0.01
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

struct Ext_AllSystem_U{
  float sigma_in_deg;               /* '<Root>/sigma_in_deg' */
  float v_in_norm;                  /* '<Root>/v_in_norm' */
  float v_max_ms;                   /* '<Root>/v_max_ms' */
  float encoder_steer_deg[4];       /* '<Root>/alpha_feed_deg' */
} ;

struct Ext_AllSystem_Y{
  float alpha_out_deg[6];           /* '<Root>/alpha_out_deg ' */
  float w_out_rad_s[6];               /* '<Root>/w_out_rad_s ' */
  bool flag_stop;                 /* '<Root>/flag_stop' */
  bool flag_straight;             /* '<Root>/flag_straight' */
  bool flag_steering;             /* '<Root>/flag_steering' */
  bool flag_busy;                 /* '<Root>/flag_busy' */
  bool flag_spinning;             /* '<Root>/flag_spinning' */
  bool input_error_flag;          /* '<Root>/input_error_flag' */
};

struct DW_AllSystem_T{
  long temporalCounter_i1;         /* '<S1>/State_selection' */
  int is_active_c3_AllSystem;      /* '<S1>/State_selection' */
  int is_c3_AllSystem;             /* '<S1>/State_selection' */
  int is_Safe_drive;               /* '<S1>/State_selection' */
};

struct B_AllSystem_T{
  float alpha_out_rad[6];           /* '<S1>/Merge1' */
  float v_out_ms[6];                /* '<S1>/Merge' */
};

struct Ext_AllSystem_U AllSystem_U;
struct Ext_AllSystem_Y AllSystem_Y;
struct DW_AllSystem_T AllSystem_DW;
struct B_AllSystem_T AllSystem_B;
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
	AllSystem_U.sigma_in_deg = 15;
	AllSystem_U.v_in_norm = 0.5;
	AllSystem_U.v_max_ms = 2;
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

	//Odrive_status_check(); //check the status of the odrives before starting to send velocity data


   // start

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
		send_Odrive_vel(5);

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

static void AllSystem_Safe_drive(const bool *bool_i, const float *Product1)
{
  float tmp_0;
  float tmp;

  /* Inport: '<Root>/sigma_in_deg' */
  tmp = fabsf(AllSystem_U.sigma_in_deg);

  /* Constant: '<S1>/sigma_max_deg' */
  if ((tmp > 35.0F) && (fabsf(tmp - 90.0F) >= AllSystem_range)) {
    AllSystem_DW.is_Safe_drive = AllSystem_IN_NO_ACTIVE_CHILD;
    AllSystem_DW.is_c3_AllSystem = AllSystem_IN_RISE_INPUT_ERROR2;

    /* Outport: '<Root>/flag_stop' */
    AllSystem_Y.flag_stop = 0;

    /* Outport: '<Root>/flag_straight' */
    AllSystem_Y.flag_straight = 0;

    /* Outport: '<Root>/flag_steering' */
    AllSystem_Y.flag_steering = 0;

    /* Outport: '<Root>/flag_busy' */
    AllSystem_Y.flag_busy = 0;

    /* Outport: '<Root>/flag_spinning' */
    AllSystem_Y.flag_spinning = 0;

    /* Outport: '<Root>/input_error_flag' */
    AllSystem_Y.input_error_flag = 1;
  } else {
    switch (AllSystem_DW.is_Safe_drive) {
     case AllSystem_IN_BUSY:
      if ((AllSystem_DW.temporalCounter_i1 >= 180000U) && (!*bool_i)) {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 1;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 0;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 0;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 0;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      } else if (!*bool_i) {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_BUSY;
        AllSystem_DW.temporalCounter_i1 = 0U;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 0;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 0;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 1;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 0;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      } else {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_SPINNING;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 0;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 0;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 0;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 1;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      }
      break;

     case AllSystem_IN_SPINNING:
      /* Outport: '<Root>/flag_stop' */
      AllSystem_Y.flag_stop = 0;

      /* Outport: '<Root>/flag_straight' */
      AllSystem_Y.flag_straight = 0;

      /* Outport: '<Root>/flag_steering' */
      AllSystem_Y.flag_steering = 0;

      /* Outport: '<Root>/flag_busy' */
      AllSystem_Y.flag_busy = 0;

      /* Outport: '<Root>/flag_spinning' */
      AllSystem_Y.flag_spinning = 1;

      /* Outport: '<Root>/input_error_flag' */
      AllSystem_Y.input_error_flag = 0;

      /* Inport: '<Root>/sigma_in_deg' */
      if (fabsf(fabsf(AllSystem_U.sigma_in_deg) - 90.0F) >= AllSystem_range) {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 1;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 0;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 0;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 0;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      }
      break;

     case AllSystem_IN_STEERING:
      /* Outport: '<Root>/flag_stop' */
      AllSystem_Y.flag_stop = 0;

      /* Outport: '<Root>/flag_straight' */
      AllSystem_Y.flag_straight = 0;

      /* Outport: '<Root>/flag_steering' */
      AllSystem_Y.flag_steering = 1;

      /* Outport: '<Root>/flag_busy' */
      AllSystem_Y.flag_busy = 0;

      /* Outport: '<Root>/flag_spinning' */
      AllSystem_Y.flag_spinning = 0;

      /* Outport: '<Root>/input_error_flag' */
      AllSystem_Y.input_error_flag = 0;

      /* Inport: '<Root>/sigma_in_deg' */
      if ((tmp <= AllSystem_range) || (tmp > 35.0F)) {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 1;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 0;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 0;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 0;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      } else {
        if (fabsf(AllSystem_U.sigma_in_deg) <= AllSystem_range) {
          AllSystem_DW.is_Safe_drive = AllSystem_IN_STRAIGHT_DRIVE;

          /* Outport: '<Root>/flag_stop' */
          AllSystem_Y.flag_stop = 0;

          /* Outport: '<Root>/flag_straight' */
          AllSystem_Y.flag_straight = 1;

          /* Outport: '<Root>/flag_steering' */
          AllSystem_Y.flag_steering = 0;

          /* Outport: '<Root>/flag_busy' */
          AllSystem_Y.flag_busy = 0;

          /* Outport: '<Root>/flag_spinning' */
          AllSystem_Y.flag_spinning = 0;

          /* Outport: '<Root>/input_error_flag' */
          AllSystem_Y.input_error_flag = 0;
        }
      }
      break;

     case AllSystem_IN_STOP:
      /* Outport: '<Root>/flag_stop' */
      AllSystem_Y.flag_stop = 1;

      /* Outport: '<Root>/flag_straight' */
      AllSystem_Y.flag_straight = 0;

      /* Outport: '<Root>/flag_steering' */
      AllSystem_Y.flag_steering = 0;

      /* Outport: '<Root>/flag_busy' */
      AllSystem_Y.flag_busy = 0;

      /* Outport: '<Root>/flag_spinning' */
      AllSystem_Y.flag_spinning = 0;

      /* Outport: '<Root>/input_error_flag' */
      AllSystem_Y.input_error_flag = 0;
      if (!*bool_i) {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 1;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 0;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 0;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 0;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      } else {
        /* Inport: '<Root>/sigma_in_deg' */
        tmp_0 = fabsf(AllSystem_U.sigma_in_deg);

        /* Inport: '<Root>/sigma_in_deg' */
        if ((tmp_0 >= AllSystem_range) && (tmp < 35.0F)) {
          AllSystem_DW.is_Safe_drive = AllSystem_IN_STEERING;

          /* Outport: '<Root>/flag_stop' */
          AllSystem_Y.flag_stop = 0;

          /* Outport: '<Root>/flag_straight' */
          AllSystem_Y.flag_straight = 0;

          /* Outport: '<Root>/flag_steering' */
          AllSystem_Y.flag_steering = 1;

          /* Outport: '<Root>/flag_busy' */
          AllSystem_Y.flag_busy = 0;

          /* Outport: '<Root>/flag_spinning' */
          AllSystem_Y.flag_spinning = 0;

          /* Outport: '<Root>/input_error_flag' */
          AllSystem_Y.input_error_flag = 0;
        } else if (fabsf(fabsf(AllSystem_U.sigma_in_deg) - 90.0F) <=
                   AllSystem_range) {
          AllSystem_DW.is_Safe_drive = AllSystem_IN_BUSY;
          AllSystem_DW.temporalCounter_i1 = 0U;

          /* Outport: '<Root>/flag_stop' */
          AllSystem_Y.flag_stop = 0;

          /* Outport: '<Root>/flag_straight' */
          AllSystem_Y.flag_straight = 0;

          /* Outport: '<Root>/flag_steering' */
          AllSystem_Y.flag_steering = 0;

          /* Outport: '<Root>/flag_busy' */
          AllSystem_Y.flag_busy = 1;

          /* Outport: '<Root>/flag_spinning' */
          AllSystem_Y.flag_spinning = 0;

          /* Outport: '<Root>/input_error_flag' */
          AllSystem_Y.input_error_flag = 0;
        } else {
          if ((fabsf(*Product1) >= AllSystem_range) && (tmp_0 <= AllSystem_range))
          {
            AllSystem_DW.is_Safe_drive = AllSystem_IN_STRAIGHT_DRIVE;

            /* Outport: '<Root>/flag_stop' */
            AllSystem_Y.flag_stop = 0;

            /* Outport: '<Root>/flag_straight' */
            AllSystem_Y.flag_straight = 1;

            /* Outport: '<Root>/flag_steering' */
            AllSystem_Y.flag_steering = 0;

            /* Outport: '<Root>/flag_busy' */
            AllSystem_Y.flag_busy = 0;

            /* Outport: '<Root>/flag_spinning' */
            AllSystem_Y.flag_spinning = 0;

            /* Outport: '<Root>/input_error_flag' */
            AllSystem_Y.input_error_flag = 0;
          }
        }
      }
      break;

     default:
      /* Outport: '<Root>/flag_stop' */
      /* case IN_STRAIGHT_DRIVE: */
      AllSystem_Y.flag_stop = 0;

      /* Outport: '<Root>/flag_straight' */
      AllSystem_Y.flag_straight = 1;

      /* Outport: '<Root>/flag_steering' */
      AllSystem_Y.flag_steering = 0;

      /* Outport: '<Root>/flag_busy' */
      AllSystem_Y.flag_busy = 0;

      /* Outport: '<Root>/flag_spinning' */
      AllSystem_Y.flag_spinning = 0;

      /* Outport: '<Root>/input_error_flag' */
      AllSystem_Y.input_error_flag = 0;

      /* Inport: '<Root>/sigma_in_deg' */
      tmp_0 = fabsf(AllSystem_U.sigma_in_deg);
      if ((tmp_0 >= AllSystem_range) && (tmp < 35.0F)) {
        AllSystem_DW.is_Safe_drive = AllSystem_IN_STEERING;

        /* Outport: '<Root>/flag_stop' */
        AllSystem_Y.flag_stop = 0;

        /* Outport: '<Root>/flag_straight' */
        AllSystem_Y.flag_straight = 0;

        /* Outport: '<Root>/flag_steering' */
        AllSystem_Y.flag_steering = 1;

        /* Outport: '<Root>/flag_busy' */
        AllSystem_Y.flag_busy = 0;

        /* Outport: '<Root>/flag_spinning' */
        AllSystem_Y.flag_spinning = 0;

        /* Outport: '<Root>/input_error_flag' */
        AllSystem_Y.input_error_flag = 0;
      } else {
        if ((fabsf(*Product1) <= AllSystem_range) || (tmp_0 >= AllSystem_range))
        {
          AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

          /* Outport: '<Root>/flag_stop' */
          AllSystem_Y.flag_stop = 1;

          /* Outport: '<Root>/flag_straight' */
          AllSystem_Y.flag_straight = 0;

          /* Outport: '<Root>/flag_steering' */
          AllSystem_Y.flag_steering = 0;

          /* Outport: '<Root>/flag_busy' */
          AllSystem_Y.flag_busy = 0;

          /* Outport: '<Root>/flag_spinning' */
          AllSystem_Y.flag_spinning = 0;

          /* Outport: '<Root>/input_error_flag' */
          AllSystem_Y.input_error_flag = 0;
        }
      }
      break;
    }
  }

  /* End of Constant: '<S1>/sigma_max_deg' */
}



void AllSystem_step(void) {


  int i;
  float rtb_alpha[6];
  float rtb_alpha_k[6];
  float Product1;
  float rtb_alpha_tmp;
  float rtb_sigma_in_rad;

  bool bool_i;

  /* Product: '<S1>/Product1' incorporates:
   *  Inport: '<Root>/v_in_norm'
   *  Inport: '<Root>/v_max_ms'
   */
  Product1 = AllSystem_U.v_in_norm * AllSystem_U.v_max_ms;

  bool_i = ((fabsf(AllSystem_U.encoder_steer_deg[0] - AllSystem_Y.alpha_out_deg
                   [0]) <= 0.5F) || (fabsf(AllSystem_U.encoder_steer_deg[1] -
              AllSystem_Y.alpha_out_deg[1]) <= 0.5F) || (fabsf
             (AllSystem_U.encoder_steer_deg[2] - AllSystem_Y.alpha_out_deg[4]) <=
             0.5F) || (fabsf(AllSystem_U.encoder_steer_deg[3] -
              AllSystem_Y.alpha_out_deg[5]) <= 0.5F));
  if (AllSystem_DW.temporalCounter_i1 < 262143U) {
    AllSystem_DW.temporalCounter_i1++;
  }

// BYPASS OF ENCODER
bool_i = 1;


  if (AllSystem_DW.is_active_c3_AllSystem == 0U) {
    AllSystem_DW.is_active_c3_AllSystem = 1U;
    AllSystem_DW.is_c3_AllSystem = AllSystem_IN_Safe_drive;
    AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

    /* Outport: '<Root>/flag_stop' */
    AllSystem_Y.flag_stop = 1;

    /* Outport: '<Root>/flag_straight' */
    AllSystem_Y.flag_straight = 0;

    /* Outport: '<Root>/flag_steering' */
    AllSystem_Y.flag_steering = 0;

    /* Outport: '<Root>/flag_busy' */
    AllSystem_Y.flag_busy = 0;

    /* Outport: '<Root>/flag_spinning' */
    AllSystem_Y.flag_spinning = 0;

    /* Outport: '<Root>/input_error_flag' */
    AllSystem_Y.input_error_flag = 0;
  } else if (AllSystem_DW.is_c3_AllSystem == AllSystem_IN_RISE_INPUT_ERROR2) {
    /* Outport: '<Root>/flag_stop' */
    AllSystem_Y.flag_stop = 0;

    /* Outport: '<Root>/flag_straight' */
    AllSystem_Y.flag_straight = 0;

    /* Outport: '<Root>/flag_steering' */
    AllSystem_Y.flag_steering = 0;

    /* Outport: '<Root>/flag_busy' */
    AllSystem_Y.flag_busy = 0;

    /* Outport: '<Root>/flag_spinning' */
    AllSystem_Y.flag_spinning = 0;

    /* Outport: '<Root>/input_error_flag' */
    AllSystem_Y.input_error_flag = 1;

  rtb_sigma_in_rad = fabsf(AllSystem_U.sigma_in_deg);
    if ((rtb_sigma_in_rad <= 35.0F) || (fabsf(rtb_sigma_in_rad - 90.0F) <=
         AllSystem_range)) {
      AllSystem_DW.is_c3_AllSystem = AllSystem_IN_Safe_drive;
      AllSystem_DW.is_Safe_drive = AllSystem_IN_STOP;

      /* Outport: '<Root>/flag_stop' */
      AllSystem_Y.flag_stop = 1;

      /* Outport: '<Root>/flag_straight' */
      AllSystem_Y.flag_straight = 0;

      /* Outport: '<Root>/flag_steering' */
      AllSystem_Y.flag_steering = 0;

      /* Outport: '<Root>/flag_busy' */
      AllSystem_Y.flag_busy = 0;

      /* Outport: '<Root>/flag_spinning' */
      AllSystem_Y.flag_spinning = 0;

      /* Outport: '<Root>/input_error_flag' */
      AllSystem_Y.input_error_flag = 0;
      }
    }

else {
    /* case IN_Safe_drive: */
    AllSystem_Safe_drive(&bool_i, &Product1);
  }

if (AllSystem_Y.flag_straight) {
    /* Merge: '<S1>/Merge1' incorporates:
     *  Constant: '<S6>/Constant'
     *  SignalConversion generated from: '<S6>/alpha_out_rad'
     */
    AllSystem_B.alpha_out_rad[0] = 0.0F;
    AllSystem_B.alpha_out_rad[1] = 0.0F;
    AllSystem_B.alpha_out_rad[2] = 0.0F;
    AllSystem_B.alpha_out_rad[3] = 0.0F;
    AllSystem_B.alpha_out_rad[4] = 0.0F;
    AllSystem_B.alpha_out_rad[5] = 0.0F;

    /* Merge: '<S1>/Merge' incorporates:
     *  Inport: '<S6>/v_in'
     *  SignalConversion generated from: '<S6>/v_out_ms'
     */
    AllSystem_B.v_out_ms[0] = Product1;
    AllSystem_B.v_out_ms[1] = Product1;
    AllSystem_B.v_out_ms[2] = Product1;
    AllSystem_B.v_out_ms[3] = Product1;
    AllSystem_B.v_out_ms[4] = Product1;
    AllSystem_B.v_out_ms[5] = Product1;
  }

  /* End of Outputs for SubSystem: '<S1>/Straight Mode' */

  /* Outputs for Enabled SubSystem: '<S1>/Steering Mode (Ackerman)' incorporates:
   *  EnablePort: '<S4>/Enable'
   */
  /* Outport: '<Root>/flag_steering' */
  if (AllSystem_Y.flag_steering) {
    /* Gain: '<S4>/Gain' incorporates:
     *  Inport: '<Root>/sigma_in_deg'
     */
    rtb_sigma_in_rad = 0.0174532924F * AllSystem_U.sigma_in_deg;

    /* MATLAB Function: '<S4>/Ackerman_angles' incorporates:
     *  Constant: '<S1>/a_m'
     *  Constant: '<S1>/d_m'
     *  Constant: '<S1>/e_m'
     */
    rtb_alpha_tmp = tanf(rtb_sigma_in_rad);
    rtb_alpha[0] = atanf(0.606F / (0.606F / rtb_alpha_tmp + 0.39F));
    rtb_alpha[1] = atanf(0.606F / (0.606F / tanf(rtb_sigma_in_rad) - 0.39F));
    rtb_alpha[2] = 0.0F;
    rtb_alpha[3] = 0.0F;
    rtb_alpha[4] = -atanf(0.593F / (0.593F / rtb_alpha_tmp + 0.39F));
    rtb_alpha[5] = -atanf(0.593F / (0.593F / tanf(rtb_sigma_in_rad) - 0.39F));

    /* MATLAB Function: '<S4>/Ackerman_velocities' incorporates:
     *  Merge: '<S1>/Merge'
     */
    rtb_sigma_in_rad = sinf(rtb_sigma_in_rad);
    AllSystem_B.v_out_ms[0] = fabsf(sinf(rtb_alpha[0]) / rtb_sigma_in_rad) *
      Product1;
    AllSystem_B.v_out_ms[1] = fabsf(sinf(rtb_alpha[1]) / rtb_sigma_in_rad) *
      Product1;
    AllSystem_B.v_out_ms[2] = Product1;
    AllSystem_B.v_out_ms[3] = Product1;
    AllSystem_B.v_out_ms[4] = fabsf(sinf(rtb_alpha[4]) / rtb_sigma_in_rad) *
      Product1;
    AllSystem_B.v_out_ms[5] = fabsf(sinf(rtb_alpha[5]) / rtb_sigma_in_rad) *
      Product1;
    for (i = 0; i < 6; i++) {
      /* Merge: '<S1>/Merge1' incorporates:
       *  SignalConversion generated from: '<S4>/alpha_out_rad'
       */
      AllSystem_B.alpha_out_rad[i] = rtb_alpha[i];
    }
  }

  /* End of Outputs for SubSystem: '<S1>/Steering Mode (Ackerman)' */

  /* Outputs for Enabled SubSystem: '<S1>/Spinning Mode' incorporates:
   *  EnablePort: '<S2>/Enable'
   */
  /* Logic: '<S1>/OR' incorporates:
   *  Outport: '<Root>/flag_busy'
   *  Outport: '<Root>/flag_spinning'
   */
  if (AllSystem_Y.flag_spinning || AllSystem_Y.flag_busy) {
    /* MATLAB Function: '<S2>/MATLAB Function' incorporates:
     *  Constant: '<S1>/a_m'
     *  Constant: '<S1>/d_m'
     *  Constant: '<S1>/e_m'
     */
    for (i = 0; i < 6; i++) {
      AllSystem_B.v_out_ms[i] = 0.0F;
    }

    if (!AllSystem_Y.flag_busy) {
      AllSystem_B.v_out_ms[0] = Product1 * 0.720649719F;
      AllSystem_B.v_out_ms[1] = -AllSystem_B.v_out_ms[0];
      AllSystem_B.v_out_ms[2] = Product1 * 0.39F;
      AllSystem_B.v_out_ms[3] = -AllSystem_B.v_out_ms[2];
      AllSystem_B.v_out_ms[4] = Product1 * 0.709752738F;
      AllSystem_B.v_out_ms[5] = -AllSystem_B.v_out_ms[4];
    }

    /* End of MATLAB Function: '<S2>/MATLAB Function' */

    /* MATLAB Function: '<S2>/Spinning_angles' incorporates:
     *  Constant: '<S1>/a_m'
     *  Constant: '<S1>/d_m'
     *  Constant: '<S1>/e_m'
     */
    rtb_alpha_k[0] = 0.998958647F;
    rtb_alpha_k[1] = -0.998958647F;
    rtb_alpha_k[2] = 0.0F;
    rtb_alpha_k[3] = 0.0F;
    rtb_alpha_k[4] = -0.989046097F;
    rtb_alpha_k[5] = 0.989046097F;
    for (i = 0; i < 6; i++) {
      /* Merge: '<S1>/Merge1' incorporates:
       *  SignalConversion generated from: '<S2>/alpha_out_rad'
       */
      AllSystem_B.alpha_out_rad[i] = rtb_alpha_k[i];
    }
  }

  /* End of Logic: '<S1>/OR' */
  /* End of Outputs for SubSystem: '<S1>/Spinning Mode' */

  /* Outputs for Enabled SubSystem: '<S1>/Stop Mode' incorporates:
   *  EnablePort: '<S5>/Enable'
   */
  /* Outport: '<Root>/flag_stop' */
  if (AllSystem_Y.flag_stop) {
    /* Merge: '<S1>/Merge1' incorporates:
     *  Constant: '<S5>/Constant'
     *  SignalConversion generated from: '<S5>/alpha_out_rad'
     */
    AllSystem_B.alpha_out_rad[0] = 0.0F;
    AllSystem_B.alpha_out_rad[1] = 0.0F;
    AllSystem_B.alpha_out_rad[2] = 0.0F;
    AllSystem_B.alpha_out_rad[3] = 0.0F;
    AllSystem_B.alpha_out_rad[4] = 0.0F;
    AllSystem_B.alpha_out_rad[5] = 0.0F;

    /* Merge: '<S1>/Merge' incorporates:
     *  Constant: '<S5>/Constant1'
     *  SignalConversion generated from: '<S5>/v_out_ms'
     */
    AllSystem_B.v_out_ms[0] = 0.0F;
    AllSystem_B.v_out_ms[1] = 0.0F;
    AllSystem_B.v_out_ms[2] = 0.0F;
    AllSystem_B.v_out_ms[3] = 0.0F;
    AllSystem_B.v_out_ms[4] = 0.0F;
    AllSystem_B.v_out_ms[5] = 0.0F;
  }

  /* End of Outputs for SubSystem: '<S1>/Stop Mode' */
  for (i = 0; i < 6; i++) {
    /* Gain: '<S1>/Gain1' incorporates:
     *  UnitDelay: '<S1>/Unit Delay'
     */
    AllSystem_Y.alpha_out_deg[i] = 57.2957802F * AllSystem_B.alpha_out_rad[i];

    /* Outport: '<Root>/w_out_rad_s ' incorporates:
     *  Constant: '<S1>/R_m '
     *  Product: '<S1>/Product'
     */
    AllSystem_Y.w_out_rad_s[i] = AllSystem_B.v_out_ms[i] / 0.35;
  }

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
