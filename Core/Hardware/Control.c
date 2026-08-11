#include "Control.h"
#include "Motor.h"
#include "Laser.h"
#include "Servo.h"
#include "In.h"
#include "stdio.h"
#include "command.h"
#include "main.h"
#include "usart.h"
#include "screen.h"

extern Laser_Data_t Laser;          // 测距结构体
extern Motor_Feedback_t Motor1_Feedback; // 马达反馈
extern uint8_t LaserRx[8];          // 测距接收缓冲区

/* ==================== 可调参数（按实际机构调整） ==================== */
#define BELT_CRUISE_STEP_DEG        1.0f    // 巡航：每步推进角度
#define BELT_CRUISE_INTERVAL_MS     20      // 巡航：推进间隔
#define BELT_APPROACH_STEP_DEG      5.0f    // 靠近：每步推进角度
#define BELT_APPROACH_INTERVAL_MS   50      // 靠近：推进间隔
#define DETECT_DISTANCE_CM          15.0f   // 停止测距距离
#define MOTOR_STOP_WAIT_MS          1000    // 电机停稳等待时间
#define RELEASE_TIMEOUT_MS          5000    // 放行后等待货物经过超时
#define GATE_RESET_MS               2000    // 超时后舵机复位持续时间
#define WAIT_CLASS_RETRY_MS         3000    // 等待分类命令时重发 Ready 间隔
#define STATE_TIMEOUT_MS            10000   // 状态机等待超时
#define RELEASE_OPEN_DEG            90      // 放行舵机打开角度
#define RELEASE_CLOSE_DEG           0       // 放行舵机关闭角度

/* ==================== 模块内部变量 ==================== */
static uint8_t command[50];         // 命令解析缓冲
static double Angle = 0;            // 马达角度
static uint8_t stop_flag = 0;       // 电机停稳标志
static uint8_t release_flag = 0;    // 放行标志
static uint32_t release_start = 0;  // 放行计时
static uint32_t stop_start = 0;     // 停稳等待计时
static uint32_t approach_start = 0; // 测距定位超时计时
static uint32_t class_start = 0;    // 分类等待超时计时
static uint32_t photo_start = 0;    // 进洞等待超时计时
static uint32_t belt_last_tick = 0; // 传送带推进节拍

static SystemState_t SystemState = STATE_RELEASE_ONE;  // 状态机结构体
GoodsSlot GoodsTable[MAX_GOODS_TYPE];           // 马格南盘分类结构体

extern const int HoleAngle[];       // 各仓位对应的转盘角度

/**
 * @brief  轮盘分类记录初始化
 */
void GoodsTable_Init(void)
{
    for (int i = 0; i < MAX_GOODS_TYPE; i++)
    {
        GoodsTable[i].used = 0;
        GoodsTable[i].count = 0;
    }
}

/**
 * @brief  查找已有分类
 *
 * @param color 颜色
 * @param shape 形状
 * @return int  找到返回仓位编号，找不到返回-1
 */
int Goods_Find(uint8_t color, uint8_t shape)
{
    for (int i = 0; i < MAX_GOODS_TYPE; i++)
    {
        if (GoodsTable[i].used)
        {
            if (GoodsTable[i].color == color &&
                GoodsTable[i].shape == shape)
            {
                return i;
            }
        }
    }
    return -1;
}

/**
 * @brief 新货物自动编号
 *
 * @param color
 * @param shape
 * @return int
 */
int Goods_Add(uint8_t color, uint8_t shape)
{
    for (int i = 0; i < MAX_GOODS_TYPE; i++)
    {
        if (GoodsTable[i].used == 0)
        {
            GoodsTable[i].color = color;
            GoodsTable[i].shape = shape;
            GoodsTable[i].count = 1;
            GoodsTable[i].used = 1;
            return i;
        }
    }
    return -1;
}

/**
 * @brief 按固定节拍推进传送带，避免主循环频率影响实际速度
 *
 * @param step        每次推进的角度
 * @param interval_ms 推进间隔
 */
static void Belt_Advance(float step, uint32_t interval_ms)
{
    if (HAL_GetTick() - belt_last_tick >= interval_ms)
    {
        belt_last_tick = HAL_GetTick();
        Angle += step;
        Motor_SetTargetAngle(Angle);
    }
}

/**
 * @brief 状态机主函数（while中调用）
 */
void System_StateMachine(void)
{
    switch (SystemState)
    {
        case STATE_RELEASE_ONE: // 放行一个货物
        {
            /* 有存货且未放行 → 打开放行舵机 */
            if (isCunIn() && release_flag == 0)
            {
                release_start = HAL_GetTick();
                Set_fangxin_duo(RELEASE_OPEN_DEG);
                release_flag = 1;
            }

            /* 货物经过传感器 → 关舵机，进入测距定位 */
            if (isHuoIn())
            {
                Set_fangxin_duo(RELEASE_CLOSE_DEG);
                release_flag = 0;
                stop_flag = 0;
                approach_start = HAL_GetTick();
                SystemState = STATE_WAIT_DISTANCE;
                break;
            }

            /* 已放行但货物迟迟未到 → 先关舵机，超时后允许重新放行 */
            if (release_flag == 1)
            {
                if (HAL_GetTick() - release_start < RELEASE_TIMEOUT_MS)
                {
                    Belt_Advance(BELT_CRUISE_STEP_DEG, BELT_CRUISE_INTERVAL_MS);
                }
                else if (HAL_GetTick() - release_start < RELEASE_TIMEOUT_MS + GATE_RESET_MS)
                {
                    Set_fangxin_duo(RELEASE_CLOSE_DEG);
                }
                else
                {
                    release_flag = 0;
                }
            }
            break;
        }

        case STATE_WAIT_DISTANCE: // 定位到检测区
        {
            /* 测距数据无效 → 重新发起接收 */
            if (Laser.Distance_cm >= LASER_INVALID_CM)
            {
                HAL_UART_Receive_IT(&huart5, LaserRx, 8);
            }

            /* 距离未到 → 传送带继续送料 */
            if (Laser.Distance_cm > DETECT_DISTANCE_CM && stop_flag == 0)
            {
                Belt_Advance(BELT_APPROACH_STEP_DEG, BELT_APPROACH_INTERVAL_MS);
            }

            /* 距离到位 → 停止，等电机停稳后再清零累计角度 */
            if (Laser.Distance_cm <= DETECT_DISTANCE_CM && stop_flag == 0)
            {
                stop_flag = 1;
                stop_start = HAL_GetTick();
                Motor_SetTargetAngle(Motor1_Feedback.total_angle);
            }

            if (stop_flag == 1 && HAL_GetTick() - stop_start >= MOTOR_STOP_WAIT_MS)
            {
                /* 已停稳（target == feedback），此时清零累计角度不会导致 PID 抽搐 */
                Motor1_Feedback.total_angle = 0;
                Angle = 0;
                stop_flag = 0;
                printf("Ready\r\n"); // 发信息给上位机
                class_start = HAL_GetTick();
                SystemState = STATE_WAIT_CLASS;
            }

            /* 超时保护：迟迟测不到距离，回到放行状态重新开始 */
            if (HAL_GetTick() - approach_start >= STATE_TIMEOUT_MS)
            {
                release_flag = 0;
                stop_flag = 0;
                SystemState = STATE_RELEASE_ONE;
            }
            break;
        }

        case STATE_WAIT_CLASS: // 等待上位机分类
        {
            uint8_t len = Command_GetCommand(command);

            if (len != 0)
            {
                uint8_t color = command[2];
                uint8_t shape = command[3];

                /* 查找是否已经存在，不存在则新建分类 */
                int slot = Goods_Find(color, shape);
                if (slot == -1)
                {
                    slot = Goods_Add(color, shape);
                }

                if (slot != -1)
                {
                    GoodsTable[slot].count++;
                    Display_Goods(slot);
                    Set_dipan_duo(HoleAngle[slot]); // 转到对应马格南仓位
                    stop_flag = 0;
                    photo_start = HAL_GetTick();
                    SystemState = STATE_WAIT_PHOTO;
                }
                break;
            }

            /* 超时保护：上位机未回复，重发 Ready 继续等待 */
            if (HAL_GetTick() - class_start >= WAIT_CLASS_RETRY_MS)
            {
                printf("Ready\r\n");
                class_start = HAL_GetTick();
            }
            break;
        }

        case STATE_WAIT_PHOTO: // 检测进洞
        {
            if (!isHuoOut())
            {
                Belt_Advance(BELT_CRUISE_STEP_DEG, BELT_CRUISE_INTERVAL_MS); // 继续送料
            }

            if (isHuoOut())
            {
                release_flag = 0;
                stop_flag = 0;
                Set_dipan_duo(0);
                SystemState = STATE_RELEASE_ONE;
                break;
            }

            /* 超时保护：货物没进洞，复位后回到放行状态 */
            if (HAL_GetTick() - photo_start >= STATE_TIMEOUT_MS)
            {
                release_flag = 0;
                stop_flag = 0;
                Set_dipan_duo(0);
                SystemState = STATE_RELEASE_ONE;
            }
            break;
        }

        default:
            SystemState = STATE_RELEASE_ONE;
            break;
    }
}
