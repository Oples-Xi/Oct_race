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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "laser.h"
#include "Motor.h"
#include "stdio.h"
#include "string.h"

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
//uint8_t LaserRx[8];

//测距函数
volatile uint8_t rxReady = 0; //接收完成标志
uint8_t rxBuffer[8];          //接收缓冲区
uint8_t processBuffer[8];     //处理缓冲区
uint8_t LaserRx[8];           // 不知道干嘛的，反正有他能跑

//马达函数
extern Motor_Feedback_t Motor1_Feedback; // 马达反馈
extern Motor_t motor;
uint8_t TxData[8] = {0};                 // 发送缓冲区
int16_t torque1 = 0;                     // 马达1扭矩值

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
extern Laser_Data_t Laser;
extern void Laser_Parse(uint8_t *buf);
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

 void ShowHex(uint8_t *buf,uint8_t len)
{
    uint8_t i;
    printf("hex = ");
    for( i = 0; i < len; i++){
      printf(" %02X", buf[i]);
    }
    printf( "\r\n");
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
  /* USER CODE BEGIN 2 */
  Motor_Init();
  printf("Motor Init\r\n");
  HAL_Delay(1000);
  Laser_UART_Start();
  if (HAL_UART_Receive_IT(&huart5, LaserRx, 8) != HAL_OK)
    Error_Handler();
  HAL_Delay(20);
  Laser_StartContinuous();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (rxReady)
    {
      rxReady = 0;
      Laser_Parse(processBuffer); // 解析激光数据
      printf("%.3F\r\n", Laser.Distance_cm); // 打印距离
    }
    else
    {
      printf("No Data\r\n");
    }
    HAL_Delay(100);   // 实时延时，用于打印频率控制
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
    if (huart == &huart5) {
        /* 清除所有错误标志（关键） */
        if (huart->ErrorCode != HAL_UART_ERROR_NONE) {
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);   // 溢出
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);    // 帧错误
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);    // 噪声
            // 读取数据寄存器以复位
            (void)huart->Instance->DR;
            huart->ErrorCode = HAL_UART_ERROR_NONE;
        }

        /* 拷贝数据并置标志 */
        memcpy(processBuffer, LaserRx, 8);
        rxReady = 1;

        /* 重新启动接收（务必检查返回值） */
        if (HAL_UART_Receive_IT(&huart5, LaserRx, 8) != HAL_OK) {
            // 若启动失败，尝试重新初始化UART或进入错误处理
            Error_Handler();
        }
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];


    if(hcan->Instance == CAN1)
    {
      if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
      {
        // 电机1反馈
        if (RxHeader.StdId == 0x201)
        {
          Motor1_Feedback.angle = (RxData[0] << 8) | RxData[1];
          Motor1_Feedback.speed = (RxData[2] << 8) | RxData[3];
          Motor1_Feedback.torque = (RxData[4] << 8) | RxData[5];
          Motor1_Feedback.temp = RxData[6];

        }
        // 电机2反馈
        // else if(RxHeader.StdId == 0x202)
        // {
        //     Motor2_Feedback.angle =
        //         (RxData[0]<<8)|RxData[1];
        //     Motor2_Feedback.speed =
        //         (RxData[2]<<8)|RxData[3];
        //     Motor2_Feedback.torque =
        //         (RxData[4]<<8)|RxData[5];
        //     Motor2_Feedback.temp =
        //         RxData[6];
        // }
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
