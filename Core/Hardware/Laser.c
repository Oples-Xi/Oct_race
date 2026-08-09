#include "laser.h"
#include "usart.h"
#include "stdio.h"
#include "Kalman.h"

Laser_Data_t Laser;

extern uint8_t LaserRx[8];


/**
 * @brief 串口重接收
 * 
 */
void Laser_UART_Start(void)
{
    HAL_UART_Receive_IT(&huart5, LaserRx, 8);
}

static uint8_t BCC(uint8_t *buf,uint8_t len)
{
    uint8_t bcc=0;

    for(uint8_t i=0;i<len;i++)
        bcc ^= buf[i]; 

    return bcc;
}



/**
 * @brief  单次测量
 * @param None
 * @retval None
 * @note 功能码0x02
 * 
 */
void Laser_StartSingle(void)
{
    uint8_t cmd[5]={0xA5,0x5A,0x02,0x00,0};

    cmd[4]=BCC(cmd,4);

    HAL_UART_Transmit(&huart5,cmd,5,100);
}

/**
 * @brief 连续测量
 * @param None
 * @retval None
 * @note 功能码0x03
 */
void Laser_StartContinuous(void)
{
    uint8_t cmd[5]={0xA5,0x5A,0x03,0x00,0};

    cmd[4]=BCC(cmd,4);

    HAL_UART_Transmit(&huart5,cmd,5,100);
}

/**
 * @brief 快速连续测量
 * @param None
 * @retval None
 * 
 * @note 功能码0x04
 */
void Laser_StartFast(void)
{
    uint8_t cmd[5]={0xA5,0x5A,0x04,0x00,0};

    cmd[4]=BCC(cmd,4);

    HAL_UART_Transmit(&huart5,cmd,5,100);
}

/**
 * @brief 停止测量
 * @param None
 * @retval None
 * @note 功能码0x05
 */
void Laser_Stop(void)
{
    uint8_t cmd[5]={0xA5,0x5A,0x05,0x00,0};

    cmd[4]=BCC(cmd,4);

    HAL_UART_Transmit(&huart5,cmd,5,100);
}

/**
 * @brief 数据解析
 * @param buf 接收数据缓冲区首地址
 */
void Laser_Parse(uint8_t *buf)
{

    if(buf[0]!=0xB4)
    {
        printf("Head1 Err\r\n");
        //HAL_UART_Receive_IT(&huart5, LaserRx, 8);
        return;
    }

    if(buf[1]!=0x69)
    {
        printf("Head2 Err\r\n");
        //HAL_UART_Receive_IT(&huart5, LaserRx, 8);
        return;
    }

    if(BCC(buf,7)!=buf[7])
    {
        printf("BCC Err Calc=%02X Recv=%02X\r\n",
                BCC(buf,7), buf[7]);
                //HAL_UART_Receive_IT(&huart5, LaserRx, 8);
        return;
    }

    Laser.FuncCode=buf[2];

    if(buf[2]&0x80)
    {
        Laser.Error=1;
        printf("Module Error\r\n");
        return;
    }

    Laser.Error=0;

    Laser.Distance =
        ((uint32_t)buf[3] << 24) |
        ((uint32_t)buf[4] << 16) |
        ((uint32_t)buf[5] << 8)  |
        ((uint32_t)buf[6]);
		
		//distance = Laser.Distance / 100.0f;
        //Laser.Distance_cm = KalmanFilter1(distance,0.01,0.15); // 转换为厘米
		Laser.Distance_cm =Laser.Distance / 100.0f;
}
