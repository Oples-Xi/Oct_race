#include "pid.h"

static float Limit(float value,float max)
{
    if(value>max) return max;
    if(value<-max) return -max;
    return value;
}

void PID_Init(PID_t *pid,
              float kp,
              float ki,
              float kd,
              float max_out,
              float max_i)
{
    pid->kp=kp;
    pid->ki=ki;
    pid->kd=kd;

    pid->target=0;
    pid->feedback=0;

    pid->error=0;
    pid->last_error=0;

    pid->integral=0;

    pid->output=0;

    pid->max_output=max_out;
    pid->max_integral=max_i;
}

float PID_Calculate(PID_t *pid)
{
    pid->error =
        pid->target -
        pid->feedback;

    pid->integral += pid->error;

    pid->integral =
        Limit(pid->integral,
              pid->max_integral);

    float derivative =
        pid->error -
        pid->last_error;

    pid->last_error =
        pid->error;

    pid->output =
        pid->kp*pid->error +
        pid->ki*pid->integral +
        pid->kd*derivative;

    pid->output =
        Limit(pid->output,
              pid->max_output);

    return pid->output;
}
