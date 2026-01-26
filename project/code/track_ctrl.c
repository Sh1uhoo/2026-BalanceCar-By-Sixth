#include "track_ctrl.h"
#include "Control.h"
#include "menu_data.h"
#include "sensor.h"

static float CalculateLineOffset() {
  static const int weight[SENSOR_NUM] = {-4, -1, +1, +4};
  float out = 0.0f;
  for (int i = 0; i < SENSOR_NUM; ++i)
    out = weight[i] * !!(sensor_value[i] == SENSOR_BLACK);
}
void Track_Init() { Sensor_Init(); }

extern struct PidData track_pid_data;
int track_base_vel = 100;

void Track_Update() {
  static float error_int, error0, error1, actual;

  Sensor_Update();
  actual = CalculateLineOffset();
  error1 = error0;
  error0 = 0 - actual;
  error_int += error0;

  int out = (track_pid_data.kp * error0 + track_pid_data.ki * error_int +
             track_pid_data.kd * (error0 - error1)) *
            0.1f;
  if (out != 0)
    Control_Target(10 * track_base_vel / out, out);
  else
    Control_Target(10 * track_base_vel, 0);
}
