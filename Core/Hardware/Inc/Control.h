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
    uint8_t color;
    uint8_t shape;
    uint8_t slot;       //对应马格南盘仓位
    uint8_t valid;

}GoodsInfo;


/**
 * @brief 状态枚举
 * 
 */
typedef enum
{
    STATE_RELEASE_ONE = 0,/*< 释放一个 */

    STATE_WAIT_DISTANCE,/*< 等待距离 */

    STATE_WAIT_CLASS,/*< 等待类别 */

    STATE_WAIT_PHOTO/*< 确认进入 */

}SystemState_t;



void GoodsTable_Init(void);
int Goods_Find(uint8_t color, uint8_t shape);
void System_StateMachine(void);

#endif
