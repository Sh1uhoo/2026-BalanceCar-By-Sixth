#include "zf_common_headfile.h"
#include "Key.h"
#include "menu_data.h"
#include "Motor.h"
#include "MPU6050.h"
#include "sensor.h"
#include "complementary_filter.h"


int main(void)
{ 
	clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                                         // 初始化默认 Debug UART
	
	Motor_Init(); 
	Menu_Start();
	pit_ms_init(TIM6_PIT, 1);                  
	interrupt_set_priority(TIM6_IRQn, 1);
	MPU6050_Init();
	CF_Init();
	
	gpio_init(D3, GPI, GPIO_HIGH, GPI_FLOATING_IN);

    while(1)
    {	
		if (!gpio_get_level(D3))
		{
			Menu_Update();
			system_delay_ms(10);
			Motor_Setspeed(0,0);
			Motor_Setspeed(0,1);
			
		}
		else 
		{
			
		}
	}

}
