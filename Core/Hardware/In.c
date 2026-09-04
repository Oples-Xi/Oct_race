#include "In.h"

#define Is_1ji_luo() ((HAL_GPIO_ReadPin(R_1ji_In_GPIO_Port, R_1ji_In_Pin) == GPIO_PIN_RESET) || (HAL_GPIO_ReadPin(L_1ji_In_GPIO_Port, L_1ji_In_Pin) == GPIO_PIN_RESET))

#define IS_2ji_Luo() ((HAL_GPIO_ReadPin(R_2ji_In_GPIO_Port, R_2ji_In_Pin) == GPIO_PIN_RESET) || (HAL_GPIO_ReadPin(L_2ji_In_GPIO_Port, L_2ji_In_Pin) == GPIO_PIN_RESET));

#define IS_CUN()  (HAL_GPIO_ReadPin(cunhuo_In_GPIO_Port, cunhuo_In_Pin) == GPIO_PIN_RESET)

#define IS_daowei()  (HAL_GPIO_ReadPin(dingwei_In_GPIO_Port, dingwei_In_Pin) == GPIO_PIN_RESET)

#define KEY_CHECK_INTERVAL 10

#define KEY_DEBOUNCE_TIME 30

#define KEY_DEBOUNCE_COUNT (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)


/**
 * @brief 检测货物是否离开一级传送带
 * 
 * @return 按下触发1 - 是；0 - 否
 */
uint8_t  is_1ji_Luo(void)
{
 static uint8_t state = 0;      // 0=释放态, 1=按下态
    static uint8_t count = 0;
    uint8_t pin_status = Is_1ji_luo(); // 1=物理按下(低电平)

    if (state == 0)  // ---------- 释放状态 ----------
    {
        if (pin_status)  // 检测到低电平（按下）
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT)
            {
                state = 1;      // 切换为按下状态
                count = 0;
                return 1; // 松开触发
            }
        }
        else
        {
            count = 0;          // 未按下，清零
        }
    }
    else  // ---------- 按下状态 ----------
    {
        if (!pin_status)  // 检测到高电平（松开）
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT) // 释放去抖完成
            {
                state = 0; // 切回释放状态
                count = 0;
            }
        }
        else
        {
            count = 0; // 按下期间抖动，清零
        }
    }
    return 0;
}

/**
 * @brief 货物是否离开二级传送带
 * 
 * @return 松开触发1 - 是；0 - 否
 */
uint8_t  isHuoOut(void)
{
 static uint8_t state = 0;
    static uint8_t count = 0;
    uint8_t pin_status = IS_2ji_Luo();

    if (state == 0)
    {
        if (pin_status)
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT)
            {
                state = 1;
                count = 0;
            }
        }
        else
        {
            count = 0;
        }
    }
    else
    {
        if (!pin_status)
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT)
            {
                state = 0;
                count = 0;
                return 1;
            }
        }
        else
        {
            count = 0;
        }
    }
    return 0;
}


/**
 * @brief 检测是否有存货
 * 
 * @return 1 - 是；0 - 否
 */
uint8_t isCunIn(void)
{
    static uint8_t stable_state = 0;
    static uint8_t count = 0;

    uint8_t pin_status = IS_CUN();  // 1=有货，0=无货


    if(pin_status != stable_state)
    {
        count++;
        if(count >= KEY_DEBOUNCE_COUNT)
        {
            stable_state = pin_status;
            count = 0;
        }
    }
    else
    {
        count = 0;
    }
    return stable_state;
}

/**
 * @brief 检测货物是否到位
 * 
 * @return 1 - 是；0 - 否
 */
uint8_t isDaowei(void)
{
    static uint8_t stable_state = 0;
    static uint8_t count = 0;
    uint8_t pin_status = IS_daowei();  // 1=有货，0=无货
    if(pin_status != stable_state)
    {
        count++;
        
        if(count >= KEY_DEBOUNCE_COUNT)
        {
            stable_state = pin_status;
            count = 0;
        }
    }
    else
    {
        count = 0;
    }


    return stable_state;
}
