#include "zf_common_headfile.h"
#include "Control.h"
#include "menu_data.h"
#include "MPU6050.h"
#include "Motor.h"
#include "complementary_filter.h"

ComplementaryFilter_t IMU;

int AveOut,DifOut,AveSpd,DifSpd,VelTarget,DirTarget;
int fall=0;

void Control_Bal(void)
{
	
    MPU6050_GetData();
    IMU = CF_Update();
    static int ErrInt,Err0,Err1,Actual,Target=0;

    AveSpd = (Motor_Get(0) + Motor_Get(1)) / 2;
    DifSpd = Motor_Get(0) - Motor_Get(1);

    Actual = IMU.roll * 100;
    Err1 = Err0;
    Err0 = Target - Actual;
    ErrInt += Err0;

    if (ErrInt > 100000) ErrInt = 100000;
    if (ErrInt < -100000) ErrInt = -100000;

    AveOut = ((bal_pid_data.kp / 4 + 40) * Err0 + bal_pid_data.ki * ErrInt / 30 + bal_pid_data.kd * (Err0 - Err1) + 6)/10;

	int pa=AveOut + DifOut,pb=AveOut - DifOut;
	if (pa>10000) pa=10000;
	if (pb>10000) pb=10000;
	if (pa<-10000) pa=-10000;
	if (pb<-10000) pb=-10000;
	
	
	if (IMU.roll <25 && IMU.roll >-25 && !fall)
	{
		Motor_Setspeed(pa, 0);
		Motor_Setspeed(pb, 1);
	}
	else 
	{
		gpio_set_level(A0,GPIO_HIGH);
		gpio_set_level(B12,GPIO_HIGH);
		gpio_set_level(A2,GPIO_HIGH);
		gpio_set_level(B14,GPIO_HIGH);
		
		system_delay_ms(20);
		
		Motor_Setspeed(0,0);
		Motor_Setspeed(0,1);
		
		fall = 1;
	}
	
	printf("%f,%d,%d\n",IMU.roll,pa,pb);
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