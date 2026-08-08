/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "laser.h"
#include "Motor.h"
#include "stdio.h"
#include "string.h"
#include "Servo.h"
#include "tjc_usart_hmi.h"
#include "In.h"
#include "command.h"
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

int test=0;//测试专用变量
int flag = 0; //标志位

//测距变量
volatile uint8_t rxReady = 0; //测距接收完成标志
uint8_t rxBuffer[8];          //测距接收缓冲区
uint8_t LaserRx[8];           // 测距处理缓冲区
extern Laser_Data_t Laser; //测距结构体

//马达变量
extern Motor_Feedback_t Motor1_Feedback; // 马达反馈
uint8_t TxData[8] = {0};                 // can发送缓冲区
int16_t torque1 = 0;                     // 马达1扭矩值
float Angle = 0;                     // 马达角度值

//串口屏变量
extern RingBuffer_t ringBuffer;	//创建一个ringBuffer的屏幕串口缓冲区
extern uint8_t tjc_RxBuffer[1];//屏幕串口接收命令位

//状态机变量
extern int tik;//状态机计时

//上位机接收变量
uint8_t readBuffer[10];//上位机接收缓存

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

//extern void Laser_Parse(uint8_t *buf);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//串口重定向（别的地方般的，非常之好用）
#ifdef __cplusplus
extern "C" {
#endif
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return (ch);
}
#ifdef __cplusplus
}
#endif

/**
*@brief 16进制打印，用于串口监控内存
*/
 void ShowHex(uint8_t *buf,uint8_t len)
{
    uint8_t i;
    printf("hex = ");
    for( i = 0; i < len; i++){
      printf(" %02X", buf[i]);
    }
    printf( "\r\n");
}

/**
 * @brief  串口接收空闲回调函数
 * 
 * @param huart 
 * @param Size 
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if (huart == &huart1)
  {
		Command_Write(readBuffer, Size);
		HAL_UARTEx_ReceiveToIdle_IT(&huart2, readBuffer, sizeof(readBuffer));
	}
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
  MX_CAN1_Init();
  MX_USART1_UART_Init();
  MX_UART5_Init();
  MX_TIM9_Init();
  MX_TIM5_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim5);
  fangxin_duo_init();
  dipan_duo_init();
  Motor_Init();
  printf("Motor Init\r\n");
  HAL_Delay(1000);
  Laser_UART_Start();
  HAL_Delay(20);
  Motor_PID_Init();
  Laser_StartContinuous();
  Motor_SetTargetAngle(5000);
  if (HAL_UART_Receive_IT(&huart5, LaserRx, 8) != HAL_OK)
  {
    printf("Receive IT Start Failed!\n");
  }
  else
  {
    printf("Receive IT Start Success!\n");
  }
  Laser.Distance_cm = 100;
  tik = HAL_GetTick();
  HAL_UARTEx_ReceiveToIdle_IT(&huart2, readBuffer, sizeof(readBuffer));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    System_StateMachine();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart5) //测距串口
    {
        /* 清除所有错误标志（关键） */
        if (huart->ErrorCode != HAL_UART_ERROR_NONE) 
        {
            (void)huart->Instance->DR;
            huart->ErrorCode = HAL_UART_ERROR_NONE;
        }
        Laser_Parse(LaserRx); // 解析激光数据

        /* 重新启动接收（务必检查返回值） */
        if (HAL_UART_Receive_IT(&huart5, LaserRx, 8) != HAL_OK) {
            // 若启动失败，尝试重新初始化UART或进入错误处理
            Error_Handler();
        }
    }

    if (huart ==&huart2) // 屏幕串口
    {
      write1ByteToRingBuffer(tjc_RxBuffer[0]);
      HAL_UART_Receive_IT(&TJC_UART, tjc_RxBuffer, 1); // 重新使能串口2接收中断
    }

    return;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];

  if (hcan->Instance == CAN1)
  {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
    {
      // 电机1反馈
      if (RxHeader.StdId == 0x201)
      {
        uint16_t encoder =
            (RxData[0] << 8) | RxData[1];

        Motor_UpdateAngle(&Motor1_Feedback, encoder);

        Motor1_Feedback.speed =
            (RxData[2] << 8) | RxData[3];

        Motor1_Feedback.torque =
            (RxData[4] << 8) | RxData[5];

        Motor1_Feedback.temp =
            RxData[6];
      }
    }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim==&htim5)
    Motor_ControlLoop();
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
  while (1)
  {
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
