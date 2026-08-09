#include "Motor.h"
#include "pid.h"

CAN_TxHeaderTypeDef CAN1_TxHeader;

extern int16_t torque1;
Motor_Feedback_t Motor1_Feedback;
PID_t PositionPID;
PID_t SpeedPID;

float target_angle=0;




/**
 * @brief 初始化CAN滤波器
 * 
 */
void can_filter_init(void)
{
    CAN_FilterTypeDef can_filter_st;
    can_filter_st.FilterActivation = ENABLE;
    can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
    can_filter_st.FilterIdHigh = 0x0000;
    can_filter_st.FilterIdLow = 0x0000;
    can_filter_st.FilterMaskIdHigh = 0x0000;
    can_filter_st.FilterMaskIdLow = 0x0000;
    can_filter_st.SlaveStartFilterBank = 14;
    //========can1==========
    can_filter_st.FilterBank = 0;
    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
    HAL_CAN_ConfigFilter(&hcan1, &can_filter_st);
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}





/**
 * @brief 初始化CAN发送报文头，主函数只需要调用这个
 * 
 */
void Motor_Init(void)
{
    can_filter_init();
    CAN1_TxHeader.IDE = CAN_ID_STD;
    CAN1_TxHeader.RTR = CAN_RTR_DATA;
    CAN1_TxHeader.DLC = 0x08;
    CAN1_TxHeader.StdId = 0x200;
}






/**
 * @brief 设置马达速度
 * 
 * @param speed
 */
void Motor_SetSpeed(int16_t speed)
{
    torque1 = speed;
    uint8_t TxData[8] = {0};
    TxData[0] = (uint8_t)(torque1 >> 8);
    TxData[1] = (uint8_t)(torque1 & 0xFF);
    TxData[2] = 0;
    TxData[3] = 0;
    TxData[4] = 0;
    TxData[5] = 0;
    TxData[6] = 0;
    TxData[7] = 0;
    HAL_CAN_AddTxMessage(&hcan1, &CAN1_TxHeader, TxData, (uint32_t *)CAN_TX_MAILBOX0);
}





void Motor_UpdateAngle(Motor_Feedback_t *motor, uint16_t encoder)
{
    motor->encoder = encoder;
    /* 第一次收到数据 */
    if(motor->initialized == 0)
    {
        motor->initialized = 1;
        motor->last_encoder = encoder;
        motor->total_encoder = encoder;
        motor->total_angle =
            encoder * 360.0f / 8192.0f;
        return;
    }
    int16_t diff = encoder - motor->last_encoder;
    /* 正向跨越8191->0 */
    if(diff < -4096)
    {
        diff += 8192;
    }
    /* 反向跨越0->8191 */
    else if(diff > 4096)
    {
        diff -= 8192;
    }
    motor->total_encoder += diff;
    motor->last_encoder = encoder;
    motor->total_angle =
        motor->total_encoder * 360.0f / 8192.0f;
}



void Motor_PID_Init(void)//还没调！！！！！！！！！！！！！
{
    PID_Init(&PositionPID,
             6.80f,
             0.0f,
             0.61f,
             2000,
             0);

    PID_Init(&SpeedPID,
             1.25f,
             0.01f,
             0,
             20000,
             3000);
}



/**
 * @brief 设置目标角度
 * 
 * @param angle 角度
 */
void Motor_SetTargetAngle(float angle)
{
    target_angle = angle;
}



/**
 * @brief Pid双环控制
 * 
 */
void Motor_ControlLoop(void)
{
    /* ----------位置环---------- */

    PositionPID.target =
        target_angle;

    PositionPID.feedback =
        Motor1_Feedback.total_angle;

    float target_speed =
        PID_Calculate(&PositionPID);

    /* ----------速度环---------- */

    SpeedPID.target =
        target_speed;

    SpeedPID.feedback =
        Motor1_Feedback.speed;

    int16_t current =
        (int16_t)PID_Calculate(&SpeedPID);

    Motor_SetSpeed(current);
}
