#include "Control.h"
#include "Motor.h"
#include "Laser.h"

extern Laser_Data_t Laser; //测距结构体
extern Motor_Feedback_t Motor1_Feedback; // 马达反馈


float TargetAngle;

uint8_t GoodsType = 0;

uint8_t UpperReady = 0;

uint8_t PhotoSensor = 0;
SystemState_t SystemState;

void System_StateMachine(void)
{
    switch(SystemState)
    {

    case STATE_RELEASE_ONE:

        /***************
         * 放行一个货物
         ***************/

        //Release_One();放行

        Motor_SetSpeed(1200);//一直转

        SystemState =
            STATE_WAIT_DISTANCE;

        break;

    case STATE_WAIT_DISTANCE:

        /*********************
         * 等待距离到15cm
         *********************/

        if(Laser.Distance_cm <= 15.0f)
        {
            TargetAngle = Motor1_Feedback.total_angle;
            Motor_SetTargetAngle(TargetAngle);

            //发信息给上位机SendToUpper();

            SystemState =
                STATE_WAIT_CLASS;
        }

        break;

    case STATE_WAIT_CLASS:

        /**********************
         * 等待YOLO分类
         **********************/

        if(UpperReady)
        {
            UpperReady = 0;

            //马格南盘转动Magazine_GotoType(GoodsType);

            Motor_SetSpeed(1200);

            SystemState =
                STATE_WAIT_PHOTO;
        }

        break;

    case STATE_WAIT_PHOTO:

        /**********************
         * 光电检测
         **********************/

        if(PhotoSensor)
        {
            Motor_SetSpeed(0);

            SystemState =
                STATE_RELEASE_ONE;
        }

        break;
    }
}

