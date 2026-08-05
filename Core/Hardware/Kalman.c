#include "Kalman.h"


Kalman kfp;
void Kalman_Init()    //主要调整参数为R和Q
{
  kfp.last_p=1;
  kfp.now_p=0;
  kfp.kg=0;
  kfp.Q=0;
  kfp.R=0.01;
  kfp.out=0;
}
/**
 *卡尔曼滤波
 *@param   Kalman*kfp  卡尔曼结构体参数
 *              float  intput   需要滤波的参数值的测量值（即传感器的采集值）
 *@return  滤波后的参数（最优值）
 */
float Kalmanfilter(Kalman*kfp,float intput)
{
  
	 kfp->now_p = kfp->last_p + kfp->Q;
    //预测协方差方程：k时刻系统估算协方差 = k-1时刻系统协方差 + 过程噪音协方
     kfp->kg =  kfp->now_p / ( kfp->now_p + kfp->R);
    //卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪音协方差）
     kfp->out = kfp->out + kfp->kg * (intput - kfp->out);
    //更新最优值方程：k时刻状态变量最优值 = 状态变量预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
	 kfp->last_p = (1-kfp->kg) * kfp->now_p;
	//更新协方差方程：本次系统的协方差赋给 kfp->last_p 为下一次运算做准备
	 
	
	
	return kfp->out;
}



/*
     *  卡尔曼滤波的五个重要公式
*/
/*
  Q 表示日均体重浮动     R 表示我有多相信体重秤

*/

double KalmanFilter1(const double ResrcData, double ProcessNiose_Q, double MeasureNoise_R)
{

    double R = MeasureNoise_R;
    double Q = ProcessNiose_Q;

    static double x_last;
    double x_mid = x_last;
    double x_now;

    static double p_last;
    double p_mid ;
    double p_now;

    double kg;

    x_mid = x_last;                     //x_last=x(k-1|k-1),x_mid=x(k|k-1)
       // 状态预测：当前状态 = 上一时刻状态 + 影响因素
	
	   //预测体重 = 之前体重 + 影响因素  
    p_mid = p_last + Q;                 //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=噪声
       // 协方差预测：k时刻系统估算协方差 = k-1时刻系统协方差 + 过程噪音协方
	
	   //中午体重有多重 = 吃的东西有多少转化为体重 + 日均体重浮动
    kg = p_mid / (p_mid + R);           //kg为kalman filter，R 为噪声
	   //卡尔曼增益方程：k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪音协方差）
	   
	   //我有多相信秤 = 我估计的误差/我估计的误差 + 秤的误差
    x_now = x_mid + kg * (ResrcData - x_mid); //估计出的最优值
	   //状态更新：k时刻状态变量最优值 = 状态变量预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
	   
	   //我的实际体重 = 预测体重 + 我有多相信秤*( 秤的数值-预测体重 )
    p_now = (1 - kg) * p_mid;           //最优值对应的covariance
	   //协方差更新：本次系统的协方差赋给 kfp->last_p 为下一次运算做准备
	   
    p_last = p_now;                     //更新covariance 值
    x_last = x_now;                     //更新系统状态值

    return x_now;

}

