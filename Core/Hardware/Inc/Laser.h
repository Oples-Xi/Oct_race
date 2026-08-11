#ifndef __LASER_H
#define __LASER_H

#include "main.h"

/* 无效距离值（测距接收失败时的兜底值） */
#define LASER_INVALID_CM 100.0f

/**
 * @brief 测距结构体
 * 
 */
typedef struct
{
    uint32_t Distance;/*!<毫米距离*/
    float Distance_cm;/*!<厘米距离*/
    uint8_t  FuncCode;/*!<功能码*/
    uint8_t  Error;/*Error*/
}Laser_Data_t;

extern Laser_Data_t Laser;

void Laser_StartSingle(void);
void Laser_StartContinuous(void);
void Laser_StartFast(void);
void Laser_Stop(void);

void Laser_UART_Start(void);
void Laser_Parse(uint8_t *buf);

#endif
