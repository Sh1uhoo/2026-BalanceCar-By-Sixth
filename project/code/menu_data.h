#ifndef MENU_DATA_H_
#define MENU_DATA_H_
#include "menu_framework.h"


struct PidData {
  int kp, ki, kd;
};
extern struct PidData bal_pid_data, vel_pid_data, dir_pid_data, pos_pid_data;
extern int process_mode;

void Menu_Start();
void Menu_Update();

#endif // MENU_DATA_H_
