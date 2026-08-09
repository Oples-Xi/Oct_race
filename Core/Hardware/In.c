#include "In.h"


#define IS_Huo_IN()  (HAL_GPIO_ReadPin(fangxing_In_GPIO_Port, fangxing_In_Pin) == GPIO_PIN_RESET)
#define IS_Huo_OUT()  (HAL_GPIO_ReadPin(luoxia_In_GPIO_Port, luoxia_In_Pin) == GPIO_PIN_RESET)
#define IS_CUN()  (HAL_GPIO_ReadPin(cunhuo_In_GPIO_Port, cunhuo_In_Pin) == GPIO_PIN_RESET)

#define KEY_CHECK_INTERVAL 10

#define KEY_DEBOUNCE_TIME 50

#define KEY_DEBOUNCE_COUNT (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)


/**
 * @brief 检测货物是否离开传送带
 * 
 * @return 1 - 是；0 - 否
 */
uint8_t  isHuoIn(void)
{
 static uint8_t state = 0;      // 0=释放态, 1=按下态
    static uint8_t count = 0;
    uint8_t pin_status = IS_Huo_IN(); // 1=物理按下(低电平)

    if (state == 0)  // ---------- 释放状态 ----------
    {
        if (pin_status)  // 检测到低电平（按下）
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT)
            {
                state = 1;      // 切换为按下状态
                count = 0;
                return 1;       // ✅ 只有这里返回1，仅代表“按下动作”
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
                state = 0;      // 切回释放状态
                count = 0;
                // ⚠️ 注意：这里故意不返回任何值，松开绝无触发！
            }
        }
        else
        {
            count = 0;          // 按下期间抖动，清零
        }
    }
    return 0;
}

/**
 * @brief 货物是否被放行
 * 
 * @return 1 - 是；0 - 否
 */
uint8_t  isHuoOut(void)
{
 static uint8_t state = 0;      // 0=释放态, 1=按下态
    static uint8_t count = 0;
    uint8_t pin_status = IS_Huo_OUT(); // 1=物理按下(低电平)

    if (state == 0)  // ---------- 释放状态 ----------
    {
        if (pin_status)  // 检测到低电平（按下）
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT)
            {
                state = 1;      // 切换为按下状态
                count = 0;
                return 1;       // ✅ 只有这里返回1，仅代表“按下动作”
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
                state = 0;      // 切回释放状态
                count = 0;
                // ⚠️ 注意：这里故意不返回任何值，松开绝无触发！
            }
        }
        else
        {
            count = 0;          // 按下期间抖动，清零
        }
    }
    return 0;
}


/**
 * @brief 检测是否有存货
 * 
 * @return 1 - 是；0 - 否
 */
uint8_t  isCunIn(void)
{
 static uint8_t state = 0;      // 0=释放态, 1=按下态
    static uint8_t count = 0;
    uint8_t pin_status = IS_CUN(); // 1=物理按下(低电平)

    if (state == 0)  // ---------- 释放状态 ----------
    {
        if (pin_status)  // 检测到低电平（按下）
        {
            count++;
            if (count >= KEY_DEBOUNCE_COUNT)
            {
                state = 1;      // 切换为按下状态
                count = 0;
                return 1;       // ✅ 只有这里返回1，仅代表“按下动作”
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
                state = 0;      // 切回释放状态
                count = 0;
                // ⚠️ 注意：这里故意不返回任何值，松开绝无触发！
            }
        }
        else
        {
            count = 0;          // 按下期间抖动，清零
        }
    }
    return 0;
}
