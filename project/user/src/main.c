#include "zf_common_headfile.h"
#include "Motor.h"
#include "Menu.h"
#include "MPU6050.h"

int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量
										// 三轴陀螺仪数据gyro (陀螺仪)
										// 三轴加速度计数据    acc (accelerometer 加速度计)

int main(void)
{ 
	clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                                         // 初始化默认 Debug UART
	Motor_Init(); 
	Menu_Init();
	uint8_t k=MPU6050_Init();
    while(1)
    {	
		

    }
}
void pit_handler (void)
{
	
	Menu_Loop();
    printf("%d%d\n",Motor_Get(0),Motor_Get(1));
}
