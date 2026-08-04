#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"
#include "can.h"

/**
 * @brief 马达反馈
 * 
 */
typedef struct
{
    int16_t angle;      // 角度
    int16_t speed;      // 速度
    int16_t torque;     // 扭矩
    uint8_t temp;       // 温度
} Motor_Feedback_t;

/**
 * @brief 实现累计角度
 * 
 */
typedef struct
{
    uint16_t encoder;
    uint16_t last_encoder;
    int32_t total_encoder;
    float position_deg;
    int16_t speed;
    int16_t torque;
}Motor_t;

/**
 * @brief 位置PID（马达）
 * 
 */
typedef struct
{
    float kp;
    float ki;
    float kd;
    float target;
    float feedback;
    float err;
    float last_err;
    float integral;
    float output;
}PID_t;

void can_filter_init(void);
void Motor_Init(void);

#endif
