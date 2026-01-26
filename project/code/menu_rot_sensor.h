#ifndef MENU_ROT_SENSOR_H_
#define MENU_ROT_SENSOR_H_
#include "zf_common_headfile.h"
#include <stdint.h>

#define MENU_ROT_NUM 1
typedef struct {
  adc_channel_enum channel;
  uint16_t middle_value;
  uint16_t actual_value;
  int16_t delta_value;
} RotEntry;
extern RotEntry menu_channels[MENU_ROT_NUM];

void Menu_RotInit();
void Menu_RotGet();
void Menu_RotReset();
void Menu_RotResetCurrent();
int16_t Menu_RotGetDelta(uint8_t pos);

#endif // !MENU_ROT_SENSOR_H_
