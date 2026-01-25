#include "zf_common_headfile.h"
#include "OLED.h"
#include "Menu.h"
#include "Key.h"
#include "MPU6050.h"
#include "complementary_filter.h"

acc_xyz_value Acc_value;
gyro_xyz_value Gyro_value;				//结构体
										// 三轴陀螺仪数据gyro (陀螺仪)
										// 三轴加速度计数据    acc (accelerometer 加速度计)
										
// 互补滤波器
ComplementaryFilter_t cf_filter;
float roll, pitch, yaw;                 // 融合后的欧拉角
const float CF_ALPHA = 0.05f;           // 加速度计权重，范围0.02-0.1
const float CF_DT = 0.01f;              // 采样周期（秒），10ms

int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();	// 初始化默认 Debug UART
//	Menu_Init();
	OLED_Init();
	Key_Init();
	uint8_t k=MPU6050_Init();
	// 初始化互补滤波器
	CF_Init(&cf_filter, CF_ALPHA, CF_DT);
	
    while(1)
    {	
		OLED_ShowNum(1,12,k,1);
		MPU6050_GetData(&Acc_value,&Gyro_value);
		
		// 更新互补滤波器
		CF_Update(&cf_filter, Acc_value.AX, Acc_value.AY, Acc_value.AZ, Gyro_value.GX, Gyro_value.GY, Gyro_value.GZ);
		
		// 获取融合后的欧拉角
		roll = cf_filter.roll;
		pitch = cf_filter.pitch;
		yaw = cf_filter.yaw;
		
//		OLED_ShowSignedNum(2, 1, Acc_value.AX, 5);					//OLED显示数据
//		OLED_ShowSignedNum(3, 1, Acc_value.AY, 5);
//		OLED_ShowSignedNum(4, 1, Acc_value.AZ, 5);
//		OLED_ShowSignedNum(2, 8, Gyro_value.GX, 5);
//		OLED_ShowSignedNum(3, 8, Gyro_value.GY, 5);
//		OLED_ShowSignedNum(4, 8, Gyro_value.GZ, 5);
		
		 OLED_ShowSignedNum(2, 1, roll, 5);
		 OLED_ShowSignedNum(3, 1, pitch, 5);
		 OLED_ShowSignedNum(4, 1, yaw, 5);
		
//		Menu_Loop();
		system_delay_ms(10);
		
    }
}
