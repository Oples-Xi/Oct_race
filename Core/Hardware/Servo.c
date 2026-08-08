#include "Servo.h"


const int HoleAngle[6] =
{
    0,      // 0号货物
    30,      // 1号货物
    60,      // 2号货物
    90,     // 3号货物
    120,     // 4号货物
    180      // 5号货物
};

/**
 * @brief 马格南盘舵机转动
 * 
 * @param angle 角度
 */
void Set_dipan_duo(int angle)
{
    if(angle>180)
        angle = 180;
        else if(angle<0)
            angle = 0;
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_1, angle * 2000 / 270 + 500);
}

/**
 * @brief 马格南盘舵机初始化
 * 
 */
void dipan_duo_init(void)
{
    HAL_TIM_Base_Start(&htim9);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
    Set_dipan_duo(0);
}

/**
 * @brief 分料机初始化
 * 
 */
void fangxin_duo_init(void)
{
    HAL_TIM_Base_Start(&htim9);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
    Set_fangxin_duo(0);
}

/**
 * @brief 粉料机转动
 * 
 * @param angle 角度
 */
void Set_fangxin_duo(int angle)
{
    if(angle>180)
        angle = 180;
        else if(angle<0)
            angle = 0;
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_2, angle * 2000 / 270 + 500);
}
