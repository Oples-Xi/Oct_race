#ifndef _KALMAN_h_
#define _KALMAN_h_


typedef struct
{
  float last_p;      //上次估算的协方差   不可为0
  float now_p;       //当前估算的协方差
  float out;         //卡尔曼滤波输出值
  float kg;          //卡尔曼增益
  float Q;           //过程噪音协方程
  float R;           //观测噪音协方程
}Kalman;

extern  Kalman kfp;

void Kalman_Init(void);
float Kalmanfilter(Kalman*kfp,float intput);
double KalmanFilter1(const double ResrcData, double ProcessNiose_Q, double MeasureNoise_R);

#endif
