#include "zf_common_headfile.h"
#include "Control.h"
#include "menu_data.h"
#include "MPU6050.h"
#include "Motor.h"
#include "complementary_filter.h"

ComplementaryFilter_t IMU;

int AveOut,DifOut,AveSpd,DifSpd,VelTarget,DirTarget;


void Control_Bal(void)
{
	
    MPU6050_GetData();
    IMU = CF_Update();
    static int ErrInt,Err0,Err1,Actual,Target=0;

    AveSpd = (Motor_Get(0) + Motor_Get(1)) / 2;
    DifSpd = Motor_Get(0) - Motor_Get(1);

    Actual = IMU.pitch;
    Err1 = Err0;
    Err0 = Target - Actual;
    ErrInt += Err0;

    AveOut = bal_pid_data.kp * Err0 + bal_pid_data.ki * ErrInt + bal_pid_data.kd * (Err0 - Err1);

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

    Out = vel_pid_data.kp * Err0 + vel_pid_data.ki * ErrInt + vel_pid_data.kd * (Err0 - Err1);
}

void Control_Dir(void)
{
    static int ErrInt,Err0,Err1,Out,Actual;

    Actual = DifSpd;
    Err1 = Err0;
    Err0 = DirTarget - Actual;
    ErrInt += Err0;

    DifOut = dir_pid_data.kp * Err0 + dir_pid_data.ki * ErrInt + dir_pid_data.kd * (Err0 - Err1);
}

void Control_Target(int x, int y)
{
    VelTarget = x;
    DirTarget = y;
}