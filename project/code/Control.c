#include "zf_common_headfile.h"
#include "Control.h"
#include "menu_data.h"
#include "MPU6050.h"
#include "Motor.h"
#include "complementary_filter.h"

ComplementaryFilter_t IMU;

int S1,S2,AveOut,DifOut,AngleOut,AveSpd,DifSpd,VelTarget,DirTarget;
int fall=0;

void Control_Bal(void)
{
	
    MPU6050_GetData();
    IMU = CF_Update();
    static int ErrInt,Err0,Err1,Actual,Target=0;

	
	S1 = Motor_Get(0);
	S2 = Motor_Get(1);
    AveSpd = (S1 + S2) / 2;
    DifSpd = S1 - S2;

	Target = VelTarget - AngleOut; 
    Actual = IMU.roll * 100;
    Err1 = Err0;
    Err0 = Target - Actual;
    ErrInt += Err0;

    if (ErrInt > 100000) ErrInt = 100000;
    if (ErrInt < -100000) ErrInt = -100000;

    AveOut = ((bal_pid_data.kp * 2) * Err0 + bal_pid_data.ki * ErrInt / 30 + bal_pid_data.kd * 2 * (Err0 - Err1) + 5)/10;

	int pa=AveOut + DifOut,pb=AveOut - DifOut;
	
	if (pa<0) 
		pa-=400;
	else if (pa>0)
		pa+=400;
	
	if (pb<0)
		pb-=400;
	else if (pb>0)
		pb+=400;

//	if (Actual > 1500 || Actual < - 1500) 
//	{
//		pa *= 1.2;
//		pb *= 1.2;
//	}
	
	if (pa>10000) pa=10000;
	if (pb>10000) pb=10000;
	if (pa<-10000) pa=-10000;
	if (pb<-10000) pb=-10000;


    /*
	if (pa<1200 && pa>300) pa = 1200;
	if (pb<1200 && pb>300) pb = 1200;
	if (pa>-1200 && pa<-300) pa = -1200;
	if (pb>-1200 && pb<-300) pb = -1200;
    */
	
	
	if (IMU.roll <50 && IMU.roll >-50 && !fall)
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
	
	printf("%f,%d,%d,%d,%d\n",IMU.roll,AngleOut,S2,pa);
}

void Control_Vel(void)
{
    static int ErrInt,Err0,Err1,Out,Actual;

    Actual = AveSpd;
    Err1 = Err0;
    Err0 = VelTarget - Actual;
    ErrInt += Err0;

	if (ErrInt > 10000) {ErrInt = 10000;}
	if (ErrInt < -10000) {ErrInt = -10000;}
	
    AngleOut = (vel_pid_data.kp * Err0 * 2
            + (vel_pid_data.ki * 2 * ErrInt) / 200.0f 
            + vel_pid_data.kd * (Err0 - Err1) 
            + 5) / 10;
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