#ifndef SENSOR_H_
#define SENSOR_H_
#include "zf_common_headfile.h"

#define SENSOR_NUM (4)

// 黑线吸收红外光，没有回波，输出低电平
#define SENSOR_BLACK 0
#define SENSOR_NONE 1
extern uint8 sensor_value[SENSOR_NUM];

void Sensor_Init();
void Sensor_Update();

#endif // !SENSOR_H_
