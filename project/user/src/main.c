#include "zf_common_headfile.h"
#include "Motor.h"
int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                                         // 初始化默认 Debug UART
	Motor_Init(); 


    while(1)
    {
        
    }
}
void pit_handler (void)
{
    printf("%d%d\n",Motor_Get(0),Motor_Get(1));
}
