#ifndef __SCREEN_H
#define __SCREEN_H

#include <stdint.h>

const char *GetColorName(uint8_t color);
const char *GetShapeName(uint8_t shape);

void Display_Goods(uint8_t slot);

#endif
