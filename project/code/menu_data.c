#include "Key.h"
#include "OLED.h"
#include "menu_framework.h"
#include "zf_common_typedef.h"
#include <stdio.h>

#define UNUSED(x) ((void)(x))

static int changed_flag = 0;
static int GetInputNormal() {
  Key_Tick();
  if (Key_Check(KEY_SINGLE, 0)) {
    return MENU_Input_SelectUp;
  } else if (Key_Check(KEY_SINGLE, 1)) {
    return MENU_Input_SelectDown;
  } else if (Key_Check(KEY_SINGLE, 2)) {
    return MENU_Input_Enter;
  } else if (Key_Check(KEY_SINGLE, 3)) {
    return MENU_Input_Exit;
  }
}
static int ProcessInputNormal(Menu_States *self,
                              void (*fn_proc)(struct Menu_States_St *,
                                              int input),
                              void *userp) {
  UNUSED(userp);
  int in = GetInputNormal();
  fn_proc(self, in);
  if (changed_flag) {
    changed_flag = 0;
    return 1;
  } else {
    return 0;
  }
}
static int TransferToChange() { return MENU_Mode_Change; }

extern Menu_Item main_page_entries[2];
Menu_Item main_page = {
    .name = "Main Menu",
    .attributes = MENU_Item_IsTop,
    .entries = main_page_entries,
    .entry_count = sizeof(main_page_entries) / sizeof(Menu_Item),
    .fn_acq_input = ProcessInputNormal,
};

extern Menu_Item pid_page_entries[4];
Menu_Item main_page_entries[2] = {
    (Menu_Item){
        .name = "Mode",
        .attributes = MENU_Item_NoEnter,
        .entries = NULL,
        .entry_count = 0,
        .fn_acq_input = ProcessInputNormal,
    },
    (Menu_Item){
        .name = "PID",
        .attributes = MENU_Item_NoTitle,
        .entries = pid_page_entries,
        .entry_count = sizeof(pid_page_entries) / sizeof(Menu_Item),
        .userp = NULL,
        .fn_acq_input = ProcessInputNormal,
    },
};

struct PidData {
  int kp, ki, kd;
} bal_pid_data, vel_pid_data, dir_pid_data, pos_pid_data;
static void ChangePidVal(int input, void *userp) {
  int *p = (int *)userp;
  if (input == MENU_Input_SelectUp) {
    changed_flag = 1;
    ++*p;
  } else if (input == MENU_Input_SelectDown) {
    changed_flag = 1;
    --*p;
  }
}
static int count_digits(int num) {
  int count = 0;
  if (num == 0)
    return 2;
  if (num < 0)
    num = -num;
  while (num > 0) {
    count++;
    num /= 10;
  }
  if (count == 1)
    count++;
  return count;
}
static void DisplayPidVal(uint8 line, uint8 begin, void *userp) {
  int *p = (int *)userp;
  OLED_ShowSignedNum(line, begin + 1, *p, count_digits(*p));
}
extern Menu_Item bal_pid_item_entries[3];
extern Menu_Item vel_pid_item_entries[3];
extern Menu_Item dir_pid_item_entries[3];
extern Menu_Item pos_pid_item_entries[3];

Menu_Item pid_page_entries[4] = {
    (Menu_Item){
        .name = "Bal PID",
        .attributes = 0,
        .entries = bal_pid_item_entries,
        .entry_count = 3,
    },
    (Menu_Item){
        .name = "Vel PID",
        .attributes = 0,
        .entries = vel_pid_item_entries,
        .entry_count = 3,
    },
    (Menu_Item){
        .name = "Dir PID",
        .attributes = 0,
        .entries = dir_pid_item_entries,
        .entry_count = 3,
    },
    (Menu_Item){
        .name = "Pos PID",
        .attributes = 0,
        .entries = pos_pid_item_entries,
        .entry_count = 3,
    },
};

#define DEFINE_PID_ENTRY(NAME)                                                 \
  Menu_Item NAME##_pid_item_entries[3] = {                                     \
      (Menu_Item){                                                             \
          .name = "KP=",                                                       \
          .attributes = MENU_Item_NoEnter | MENU_Item_NoTitle,                 \
          .entries = NULL,                                                     \
          .entry_count = 0,                                                    \
          .userp = &NAME##_pid_data.kp,                                        \
          .fn_on_entry = TransferToChange,                                     \
          .fn_proc_input = ChangePidVal,                                       \
          .fn_on_put = DisplayPidVal,                                          \
      },                                                                       \
      (Menu_Item){                                                             \
          .name = "KI=",                                                       \
          .attributes = MENU_Item_NoEnter | MENU_Item_NoTitle,                 \
          .entries = NULL,                                                     \
          .entry_count = 0,                                                    \
          .userp = &NAME##_pid_data.ki,                                        \
          .fn_on_entry = TransferToChange,                                     \
          .fn_proc_input = ChangePidVal,                                       \
          .fn_on_put = DisplayPidVal,                                          \
      },                                                                       \
      (Menu_Item){                                                             \
          .name = "KD=",                                                       \
          .attributes = MENU_Item_NoEnter | MENU_Item_NoTitle,                 \
          .entries = NULL,                                                     \
          .entry_count = 0,                                                    \
          .userp = &NAME##_pid_data.kd,                                        \
          .fn_on_entry = TransferToChange,                                     \
          .fn_proc_input = ChangePidVal,                                       \
          .fn_on_put = DisplayPidVal,                                          \
      },                                                                       \
  };

DEFINE_PID_ENTRY(bal)
DEFINE_PID_ENTRY(vel)
DEFINE_PID_ENTRY(dir)
DEFINE_PID_ENTRY(pos)
