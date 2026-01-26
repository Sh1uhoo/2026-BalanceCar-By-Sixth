#include "zf_common_headfile.h"
#include "Control.h"
#include "Menu.h"
#include "MPU6050.h"
#include "Motor.h"
#include "complementary_filter.h"

ComplementaryFilter_t IMU;

struct pid
{
    int kp;
    int ki;
    int kd;
} PID_Bal, PID_Vel, PID_Dir, PID_Pos;

int AveOut,DifOut,AveSpd,DifSpd,VelTarget,DirTarget;


void Control_Bal(void)
{
	PID_Bal.kp=1;
	
    MPU6050_GetData();
    IMU = CF_Update();
    static int ErrInt,Err0,Err1,Actual,Target=0;

    AveSpd = (Motor_Get(0) + Motor_Get(1)) / 2;
    DifSpd = Motor_Get(0) - Motor_Get(1);

    Actual = IMU.pitch;
    Err1 = Err0;
    Err0 = Target - Actual;
    ErrInt += Err0;

    AveOut = PID_Bal.kp * Err0 + PID_Bal.ki * ErrInt + PID_Bal.kd * (Err0 - Err1);

    Motor_Setspeed(AveOut + DifOut, 0);
    Motor_Setspeed(AveOut - DifOut, 1);
	
	printf("%f,%d,%d\n",IMU.pitch,AveSpd,AveOut);
}

void Control_Vel(void)
{
    static int ErrInt,Err0,Err1,Out,Actual;

    Actual = AveSpd;
    Err1 = Err0;
    Err0 = VelTarget - Actual;
    ErrInt += Err0;

    Out = PID_Vel.kp * Err0 + PID_Vel.ki * ErrInt + PID_Vel.kd * (Err0 - Err1);
}

void Control_Dir(void)
{
    static int ErrInt,Err0,Err1,Out,Actual;

    Actual = DifSpd;
    Err1 = Err0;
    Err0 = DirTarget - Actual;
    ErrInt += Err0;

    DifOut = PID_Dir.kp * Err0 + PID_Dir.ki * ErrInt + PID_Dir.kd * (Err0 - Err1);
}

void Control_Target(int x, int y)
{
    VelTarget = x;
    DirTarget = y;
}