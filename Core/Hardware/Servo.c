#include "Servo.h"

void Set_dipan_Duo(int angle)
{
    if(angle>180)
        angle = 180;
        else if(angle<0)
            angle = 0;
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_1, angle * 2000 / 180 + 500);
}


void dipan_duo_init(void)
{
    HAL_TIM_Base_Start(&htim9);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
    Set_dipan_Duo(0);
}
