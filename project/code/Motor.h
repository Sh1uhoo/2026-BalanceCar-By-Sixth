#ifndef __Motor_H
#define __Motor_H

void Motor_Init(void);
void Motor_Setspeed(int16 Speed,uint8 MotorNum);
int16_t Motor_Get(uint8_t motor_id);

#endif
