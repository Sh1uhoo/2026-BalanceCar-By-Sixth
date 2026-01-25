#include "Key.h"
#include "menu_data.h"
#include "zf_common_headfile.h"

#define PIT                     (TIM6_PIT )
#define PIT_PRIORITY            (TIM6_IRQn)

int16_t AX, AY, AZ, GX, GY,
    GZ; // 定义用于存放各个数据的变量
        //  三轴陀螺仪数据gyro (陀螺仪)
        //  三轴加速度计数据    acc (accelerometer 加速度计)

int pit_state = 0;
int main(void) {
  clock_init(SYSTEM_CLOCK_120M); // 初始化芯片时钟 工作频率为 120MHz
  debug_init();                  // 初始化默认 Debug UART
	Key_Init();
  Menu_Start();
	
	pit_ms_init(PIT, 10);
	interrupt_set_priority(PIT_PRIORITY, 0);
  while (1) {
		if (pit_state) {
			Menu_Update();
			pit_state = 0;
		}
  }
}

void pit_handler() {
	pit_state = 1;
}
