	#include "zf_common_headfile.h"
#include "Motor.h"

#define PIT                             (TIM6_PIT )                             // 使用的周期中断编号 如果修改 需要同步对应修改周期中断编号与 isr.c 中的调用
#define PIT_PRIORITY                    (TIM6_IRQn)

extern uint8_t State;
pwm_channel_enum channel_list[2] = {TIM5_PWM_CH2_A1, TIM5_PWM_CH4_A3};

 void Motor_Init(void)
 {
	
 	gpio_init(A0, GPO, GPIO_LOW, GPO_PUSH_PULL);                               // 初始化 MOTOR1_DIR 输出
	gpio_init(A2, GPO, GPIO_LOW, GPO_PUSH_PULL);                               // 初始化 MOTOR2_DIR 输出
	gpio_init(B12, GPO, GPIO_LOW, GPO_PUSH_PULL);                               
	gpio_init(B14, GPO, GPIO_LOW, GPO_PUSH_PULL);
	 gpio_init(B9, GPO, GPIO_LOW, GPO_PUSH_PULL);

	pwm_init(TIM5_PWM_CH2_A1, 17000, 0);                                                // 初始化 PWM 通道 频率 17KHz 初始占空比 0%
    pwm_init(TIM5_PWM_CH4_A3, 17000, 0);
	
	encoder_quad_init(TIM3_ENCODER, TIM3_ENCODER_CH1_B4, TIM3_ENCODER_CH2_B5);    // 初始化编码器模块与引脚 正交解码编码器模式
	encoder_quad_init(TIM4_ENCODER, TIM4_ENCODER_CH1_B6, TIM4_ENCODER_CH2_B7);
	
	pit_ms_init(PIT, 10);                                                      // 初始化 PIT 为周期中断 10ms 周期
	interrupt_set_priority(PIT_PRIORITY, 0);
	
 }
 
 void Motor_Setspeed(int16 Speed,uint8 MotorNum){
	 if (Speed>=0)
	 {
		if (MotorNum == 0)
		{
			gpio_set_level(A0,GPIO_HIGH);
			gpio_set_level(B12,GPIO_LOW);
		}
		else if (MotorNum == 1)
		{
			gpio_set_level(A2,GPIO_HIGH);
			gpio_set_level(B14,GPIO_LOW);
		}
		pwm_set_duty(channel_list[MotorNum], Speed);
	 }
	 else
	 {
		if (MotorNum == 0)
		 {
			gpio_set_level(A0,GPIO_LOW);
			gpio_set_level(B12,GPIO_HIGH);
		 }
		 else if (MotorNum == 1)
		 {
			gpio_set_level(A2,GPIO_LOW);
			gpio_set_level(B14,GPIO_HIGH);
		 }
		 pwm_set_duty(channel_list[MotorNum], -Speed);
	 }
 }
 
int16_t Motor_Get(uint8_t motor_id)
{
    int16_t temp = 0;
    
    switch(motor_id)
    {
        case 0:
            temp = encoder_get_count(TIM3_ENCODER); 
            encoder_clear_count(TIM3_ENCODER);
            break;
            
        case 1:
            temp = encoder_get_count(TIM4_ENCODER);
            encoder_clear_count(TIM4_ENCODER);
            break;

    }
    
    return temp;
}