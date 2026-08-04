#ifndef __LASER_H
#define __LASER_H

#include "main.h"


/**
 * @brief ????
 * 
 */
typedef struct
{
    uint32_t Distance;/*!<??*/
    float Distance_cm;/*!<????*/
    uint8_t  FuncCode;/*!<???*/
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
