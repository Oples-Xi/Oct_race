#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"

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

void System_StateMachine(void);

#endif
