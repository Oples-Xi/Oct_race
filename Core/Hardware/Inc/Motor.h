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
    uint16_t encoder;          // 当前编码器值(0~8191)
    uint16_t last_encoder;     // 上一次编码器值
    int32_t total_encoder;     // 累计编码器值
    float total_angle;         // 连续角度(°)
    int16_t speed;
    int16_t torque;
    uint8_t temp;
    uint8_t initialized;       // 首次接收标志
}Motor_Feedback_t;

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
void Motor_SetSpeed(int16_t speed);
float Position_PID(PID_t *pid);
void Motor_UpdateAngle(Motor_Feedback_t *motor, uint16_t encoder);

#endif
