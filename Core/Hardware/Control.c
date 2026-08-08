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

//分类变量
uint8_t color;//颜色
uint8_t shape;//形状

//伪代码替代变量
uint8_t UpperReady = 0;
uint8_t PhotoSensor = 0;



SystemState_t SystemState;//状态机结构体
GoodsSlot GoodsTable[MAX_GOODS_TYPE];//马格南盘分类结构体
int tik = 0;//计时

extern float Angle;//main中马达角度
extern int flag;//main中标志位
extern const int HoleAngle[];

/**
 * @brief  轮盘分类记录初始化
 * 
 */
void GoodsTable_Init(void)
{
    for(int i=0;i<MAX_GOODS_TYPE;i++)
    {
        GoodsTable[i].used = 0;
        GoodsTable[i].count = 0;
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
        if(GoodsTable[i].used)
        {
            if(GoodsTable[i].color == color &&
               GoodsTable[i].shape == shape)
            {
                return i;
            }
        }
    }
    return -1;
}

/**
 * @brief 新货物自动编号
 * 
 * @param color 
 * @param shape 
 * @return int 
 */
int Goods_Add(uint8_t color,uint8_t shape)
{

    for(int i=0;i<MAX_GOODS_TYPE;i++)
    {
        if(GoodsTable[i].used == 0)
        {
            GoodsTable[i].color = color;
            GoodsTable[i].shape = shape;
            GoodsTable[i].count = 1;
            GoodsTable[i].used = 1;
            return i;
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

        /**
         * 定位到检测区
         */
        case STATE_WAIT_DISTANCE:
        if(Laser.Distance_cm>15.0f && flag==0)/* 等待距离到15cm*/
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

        /*
        *分类
        */
        case STATE_WAIT_CLASS:
        {
            commandLength = Command_GetCommand(command);
            if (commandLength != 0)
            {
                HAL_UART_Transmit(&huart2, command, commandLength, HAL_MAX_DELAY);
                uint8_t color = command[2];
                uint8_t shape = command[3];
                // 查找是否已经存在
                int slot = Goods_Find(color, shape);
                // 第一次识别该货物
                if (slot == -1)
                {
                    slot = Goods_Add(color, shape);
                }
                if (slot != -1)
                {
                    GoodsTable[slot].count++;
                    printf("Color:%d Shape:%d Slot:%d \r\n", color, shape, slot);
                    Set_dipan_duo(HoleAngle[slot]);// 转到对应马格南仓位
                    SystemState = STATE_WAIT_PHOTO;
                }
            }
            break;
        }

        /**
         * 检测进洞
         */
        case STATE_WAIT_PHOTO:
        if (!isHuoOut())
        {
            Angle += 50;
            Motor_SetTargetAngle(Angle);//一直转
        }
        if(isHuoOut())
        {
            SystemState =STATE_RELEASE_ONE;
        }
        break;
    }
}
