#include "zf_common_headfile.h"
#include "OLED.h"
#include "Menu.h"
#include "Key.h"
#include "MPU6050.h"

int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量
										// 三轴陀螺仪数据gyro (陀螺仪)
										// 三轴加速度计数据    acc (accelerometer 加速度计)

int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();	// 初始化默认 Debug UART
//	Menu_Init();
	OLED_Init();
	Key_Init();
	uint8_t k=MPU6050_Init();
    while(1)
    {	
		OLED_ShowNum(1,12,k,1);
		MPU6050_GetData_Acc(&AX,&AY,&AZ);
		MPU6050_GetData_Gyro(&GX,&GY,&GZ);
		OLED_ShowSignedNum(2, 1, AX, 5);					//OLED显示数据
		OLED_ShowSignedNum(3, 1, AY, 5);
		OLED_ShowSignedNum(4, 1, AZ, 5);
		OLED_ShowSignedNum(2, 8, GX, 5);
		OLED_ShowSignedNum(3, 8, GY, 5);
		OLED_ShowSignedNum(4, 8, GZ, 5);
//		Menu_Loop();
//		system_delay_ms(10);
		system_delay_ms(100);
    }
}
