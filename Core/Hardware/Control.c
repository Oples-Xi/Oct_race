#include "Control.h"
#include "Motor.h"
#include "Laser.h"
#include "Servo.h"
#include "In.h"
#include "stdio.h"
#include "command.h"
#include "main.h"
#include "usart.h"
#include "screen.h"

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

//放行标志位
int fang = 0;

SystemState_t SystemState =STATE_INIT;//状态机结构体
GoodsSlot GoodsTable[MAX_GOODS_TYPE];//马格南盘分类结构体'

int tik = 0;//全局计时
int motortik = 0;//马达角度累加计时
int try;//分料机尝试次数

extern float Angle;//main中马达角度
extern int flag;//main中标志位
extern const int HoleAngle[];

extern uint8_t LaserRx[8];

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
            if(GoodsTable[i].color == color && GoodsTable[i].shape == shape)
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
        case STATE_INIT://只是初始化位置

            Set_fangxin_duo(0);//放行口默认关闭
            if(HAL_GetTick()-tik<5000)
            {
                Set_pidai_zhuan(100);
                Set_dipan_duo(HoleAngle[0]);//看着像在自检的转动
            }
            if(HAL_GetTick()-tik>5000)
                Set_dipan_duo(0);//看着像在自检的转动
            if(HAL_GetTick()-tik>10000)
            { 
                Angle = -100000;//初始值给这么低是防止超出float的范围
                Motor1_Feedback.total_angle = -100000;
                tik = HAL_GetTick();
                SystemState = STATE_WAIT_DISTANCE;
                try = 0;
            }

            break;

        case STATE_RELEASE_ONE://放行一个货物


            if (fang == 0) // 没放行过
            {
                Set_fangxin_duo(90); // 舵机放行
                if (HAL_GetTick() - tik > 2000)
                {
                    tik = HAL_GetTick();
                    if (isCunIn())
                    {
                        Set_fangxin_duo(0);
                        fang = 1;
                        try = 0;
                    }
                    else
                    {
                        try += 1;
                    }
                }

                if (try == 2)//第二次尝试结束，转动底盘来制造震动
                {
                    if(HAL_GetTick()-tik <4000)
                    {
                        Set_dipan_duo(HoleAngle[5]);
                        HAL_Delay(2000);//这里的不影响程序
                        Set_dipan_duo(HoleAngle[0]);
                        HAL_Delay(1500);
                    }
                }
                else if (try == 3)
                {
                    
                    Set_dipan_duo(270);
                    HAL_Delay(500);
                    Set_dipan_duo(0);
                    HAL_Delay(500);
                }
            }

            if (is_1ji_Luo() && fang == 1)
            {
                Set_pidai_zhuan(0);//一级停转，防止多放
                Set_fangxin_duo(0); // 舵机复位
                flag = 0;
                SystemState =
                    STATE_WAIT_DISTANCE;
                fang = 0;
                try = 0;
            }
        
        break;

        /**
         * 定位到检测区
         */
        case STATE_WAIT_DISTANCE:
            
            if (Laser.Distance_cm == 100.0f) // 测距模块可能接收失败，于是重新接收
            {
                HAL_UART_Receive_IT(&huart5, LaserRx, 8);
            }
        if(Laser.Distance_cm>8.0f && flag==0)/* ！！！！！！！要调！！！！等待距离到8cm*/
        {
            printf("%.2f\r\n", Angle);
            Angle += 50;
            Motor_SetTargetAngle(Angle);
        }
        if ((Laser.Distance_cm <= 8.0f ||isDaowei()) && flag == 0)
        {
            Angle = Motor1_Feedback.total_angle;
            Motor_SetTargetAngle(Angle);
            flag = 1;
            Motor1_Feedback.total_angle = 0;
            HAL_Delay(1000);//等停稳  
            //为什么用hal_delay?因为我懒
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
                //HAL_UART_Transmit(&huart2, command, commandLength, HAL_MAX_DELAY);
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
                    //printf("Color:%d Shape:%d Slot:%d \r\n", color, shape, slot);
                    Display_Goods(slot);
                    Set_dipan_duo(HoleAngle[slot]);// 转到对应马格南仓位
                    flag = 0;
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
            Angle += 1;
            Motor_SetTargetAngle(Motor1_Feedback.total_angle);//一直转
        }
        if(isHuoOut())
        {
            flag = 0;
            fang = 0;
            tik = HAL_GetTick();
            SystemState =STATE_RELEASE_ONE;
            Set_dipan_duo(HoleAngle[0]);
        }
        break;
    }
}
