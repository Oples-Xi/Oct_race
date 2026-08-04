#include "Motor.h"

CAN_TxHeaderTypeDef CAN1_TxHeader;

extern int16_t torque1;
Motor_Feedback_t Motor1_Feedback;

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

/**
 * @brief 位置PID
 * 
 * @param pid 
 * @return float 
 */
float Position_PID(PID_t *pid)
{
    pid->err = pid->target - pid->feedback;
    pid->integral += pid->err;
    float derivative =
        pid->err - pid->last_err;
    pid->last_err = pid->err;
    pid->output =
        pid->kp * pid->err +
        pid->ki * pid->integral +
        pid->kd * derivative;
    return pid->output;
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
