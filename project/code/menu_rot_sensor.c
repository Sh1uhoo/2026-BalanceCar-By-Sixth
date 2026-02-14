#include "menu_rot_sensor.h"
#include "zf_driver_adc.h"
#include <stdint.h>

RotEntry menu_channels[MENU_ROT_NUM] = {{ADC1_CH10_C0, 0, 0, 0}};

void Menu_RotInit() {
  for (int i = 0; i < MENU_ROT_NUM; ++i)
    adc_init(menu_channels[i].channel, ADC_12BIT);
  Menu_RotReset();
}
void Menu_RotGet() {
  RotEntry *e;
  for (int i = 0; i < MENU_ROT_NUM; ++i) {
    e = menu_channels + i;
    e->actual_value = adc_convert(e->channel);
    e->delta_value = (int16_t)e->actual_value - (int16_t)e->middle_value;
  }
}
void Menu_RotReset() {
  RotEntry *e;
  for (int i = 0; i < MENU_ROT_NUM; ++i) {
    e = menu_channels + i;
    e->actual_value = adc_convert(e->channel);
    e->middle_value = e->actual_value;
  }
}
void Menu_RotResetCurrent() {
  RotEntry *e;
  for (int i = 0; i < MENU_ROT_NUM; ++i) {
    e = menu_channels + i;
    e->middle_value = e->actual_value;
  }
}
int16_t Menu_RotGetDelta(uint8_t pos) { return menu_channels[pos].delta_value; }
