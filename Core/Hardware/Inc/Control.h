#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"


#define MAX_GOODS_TYPE 6

/**
 * @brief 马格南盘分类结构体
 * 
 */
typedef struct
{
    uint8_t color;      // 颜色
    uint8_t shape;      // 形状
    uint8_t count;      // 数量
    uint8_t slot;       // 对应马格南盘仓位
    uint8_t used;       // 是否已经分配仓位

}GoodsSlot;



/**
 * @brief 状态枚举
 * 
 */
typedef enum
{
    STATE_INIT = 0,/*初始化*/

    STATE_RELEASE_ONE,/* 释放一个 */

    STATE_WAIT_DISTANCE,/* 等待距离 */

    STATE_WAIT_CLASS,/*等待类别 */

    STATE_WAIT_PHOTO/*确认进入 */

}SystemState_t;



void GoodsTable_Init(void);
int Goods_Find(uint8_t color, uint8_t shape);
int Goods_Add(uint8_t color, uint8_t shape);
void System_StateMachine(void);

#endif
