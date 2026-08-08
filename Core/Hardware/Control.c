#include "Control.h"
#include "Motor.h"
#include "Laser.h"
#include "Servo.h"
#include "In.h"
#include "stdio.h"
#include "command.h"
#include "main.h"
#include "usart.h"

extern Laser_Data_t Laser; //测距结构体
extern Motor_Feedback_t Motor1_Feedback; // 马达反馈


//float TargetAngle;

//uint8_t GoodsType = 0;

//接收缓存变量
uint8_t command[50];
int commandLength = 0;

uint8_t UpperReady = 0;

uint8_t PhotoSensor = 0;
SystemState_t SystemState;//状态机
GoodsInfo GoodsTable[MAX_GOODS_TYPE];//马格南盘分类结构体

extern float Angle;//main中马达角度
extern int flag;//main中标志位

int tik = 0;


/**
 * @brief  轮盘分类记录初始化
 * 
 */
void GoodsTable_Init(void)
{
    for(int i=0;i<MAX_GOODS_TYPE;i++)
    {
        GoodsTable[i].valid = 0;
    }
}

/**
 * @brief  查找已有分类
 * 
 * @param color 颜色
 * @param shape 形状
 * @return int  找到返回仓位编号，找不到返回-1
 */
int Goods_Find(uint8_t color,uint8_t shape)
{
    for(int i=0;i<MAX_GOODS_TYPE;i++)
    {
        if(GoodsTable[i].valid)
        {
            if (GoodsTable[i].color == color && GoodsTable[i].shape == shape)
            {
                return GoodsTable[i].slot;
            }
        }
    }

    return -1;
}

/**
 * @brief 状态机主函数（while中调用）
 * 
 */
void System_StateMachine(void)
{
    switch(SystemState)
    {

    case STATE_RELEASE_ONE://放行一个货物
        Set_fangxin_duo(90); //舵机放行

        if (!isHuoIn() && HAL_GetTick() - tik < 5000) // 没经过传感器
        {
            Angle += 50;
            Motor_SetTargetAngle(Angle);//一直转
        }
        else if (!isHuoIn() && HAL_GetTick() - tik > 5000) // 5s还没经过传感器
        {
            Set_fangxin_duo(0); //舵机复位
            HAL_Delay(1000);
            Set_fangxin_duo(90); //舵机放行
            tik = HAL_GetTick();//重新计时
        }
        else
        {
            Set_fangxin_duo(0); //舵机复位
            SystemState =
                STATE_WAIT_DISTANCE;
        }

        break;

    case STATE_WAIT_DISTANCE:/* 等待距离到15cm*/
        if(Laser.Distance_cm>15.0f && flag==0)
        {
            printf("%.2f\r\n", Angle);
            Angle += 50;
            Motor_SetTargetAngle(Angle);
        }
        if (Laser.Distance_cm <= 15.0f && flag != 1)
        {
            Angle = Motor1_Feedback.total_angle;
            Motor_SetTargetAngle(Angle);
            flag = 1;
            printf("Ready\r\n"); // 发信息给上位机
            SystemState = STATE_WAIT_CLASS;
        }

        break;

    case STATE_WAIT_CLASS:/*等待YOLO分类*/
        commandLength = Command_GetCommand(command);
        if (commandLength != 0)
        {
            HAL_UART_Transmit(&huart2, command, commandLength, HAL_MAX_DELAY);
            for (int i = 2; i < commandLength - 1; i += 2)//解包
            {
                //第一位数据
                if (command[i] == 0x00)
                {
                    //红色
                }
                else if (command[i] == 0x01)
                {
                    //橙色
                }
                else if (command[i] == 0x02)
                {
                    //黄色
                }
                else if (command[i] == 0x03)
                {
                    //蓝色
                }
                else if (command[i] == 0x04)
                {
                    //绿色
                }
                else if (command[i] == 0x05)
                {
                    //黑色
                }
                else if (command[i] == 0x06)
                {
                    //白色
                }
                else if (command[i] == 0x07)
                {
                    //紫色
                }

                //第二位数据
                if (command[i + 1] == 0x00)
                {
                    //正方体
                }
                else if (command[i + 1] == 0x01)
                {
                    //五棱柱
                }
                else if (command[i + 1] == 0x02)
                {
                    //四棱锥
                }
                else if (command[i + 1] == 0x03)
                {
                    //三棱锥
                }
                else if (command[i + 1] == 0x04)
                {
                    //圆柱体
                }
                else if (command[i + 1] == 0x05)
                {
                    //四棱柱
                }
                else if (command[i + 1] == 0x06)
                {
                    //球
                }
            }
        }

            if (UpperReady)
            {
                UpperReady = 0;

                // 马格南盘转动Magazine_GotoType(GoodsType);

                Motor_SetSpeed(1200);

                SystemState =
                    STATE_WAIT_PHOTO;
            }

            break;

        case STATE_WAIT_PHOTO:

            /**********************
             * 光电检测
             **********************/

            if (PhotoSensor)
            {
                Motor_SetSpeed(0);

                SystemState =
                    STATE_RELEASE_ONE;
            }

            break;
        }
}
