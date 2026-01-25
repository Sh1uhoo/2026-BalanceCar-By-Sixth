#include "sensor.h"

static gpio_pin_enum sensor_enums[SENSOR_NUM] = {E8, E9, E10, E11};
uint8 sensor_value[SENSOR_NUM] = {};

void Sensor_Init() {
  for (uint8_t i = 0; i < SENSOR_NUM; ++i)
    gpio_init(sensor_enums[i], GPI, GPIO_HIGH, GPI_PULL_UP);
}
void Sensor_Update() {
  for (uint8_t i = 0; i < SENSOR_NUM; ++i)
    sensor_value[i] = gpio_get_level(sensor_enums[i]);
}
