#include "zf_common_headfile.h"
#include "OLED.h"
#include "Menu.h"
#include "Key.h"
#include "MPU6050.h"
#include "complementary_filter.h"


int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();	// 初始化默认 Debug UART
//	Menu_Init();
	OLED_Init();
	Key_Init();
	MPU6050_Init();
	MPU6050_Check();    			//校正
	// 初始化互补滤波器
	CF_Init();
	
    while(1)
    {	
		MPU6050_GetData();
		
		// 更新互补滤波器
		CF_Update();
		
		// 获取融合后的欧拉角
//		roll = cf_filter.roll;
//		pitch = cf_filter.pitch;
//		yaw = cf_filter.yaw;
		
//		printf("%.5f,%.5f,%.5f\n",roll,pitch,yaw);
//		printf("%.f,%.f,%.f\n",roll,pitch,yaw);
		
//		Menu_Loop();
		system_delay_ms(10);
		
    }
}
