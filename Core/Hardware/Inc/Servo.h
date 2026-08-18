#ifndef __SERVO_H
#define __SERVO_H

#include "gpio.h"
#include "tim.h"
#include "stdlib.h"



void dipan_duo_init(void);
void Set_dipan_duo(int angle);
void fangxin_duo_init(void);
void Set_fangxin_duo(int angle);
void pidai_duo_init(void);
void Set_pidai_zhuan(int speed);

#endif
