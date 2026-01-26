#include "menu_data.h"
#include "Flash.h"
#include "Key.h"
#include "menu_framework.h"
#include "menu_rot_sensor.h"
#include "display/oled_ssd1309.h"
#include "display/oled_text_ui.h"
#include "zf_common_typedef.h"
#include "zf_driver_flash.h"
#include <stdio.h>

#define UNUSED(x) ((void)(x))

OLED_Text_M_MakeHnd(oled_handle, 16, 4, 0, 0);
static int H_CountDigits(int num) {
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
  return 0;
}
static int ProcessInputNormal(Menu_States *self,
                              void (*fn_proc)(struct Menu_States_St *,
                                              int input),
                              void *userp) {
  UNUSED(userp);
  int in = GetInputNormal();
  fn_proc(self, in);
  if (changed_flag || in != 0) {
    changed_flag = 0;
    return 1;
  } else {
    return 0;
  }
}
static int GetInputAdjust() {
  Key_Tick();
  if (Key_Check(KEY_DOUBLE, 2)) {
    return MENU_Input_ResetToZero;
  }
  // TODO: FIX THIS
  if (!Key_Check(KEY_DOWN, 2)) {
    if (Key_Check(KEY_SINGLE, 0)) {
      return MENU_Input_Increase;
    } else if (Key_Check(KEY_SINGLE, 1)) {
      return MENU_Input_Decrease;
    }
  } else {
    if (Key_Check(KEY_SINGLE, 0)) {
      return 0;
      return MENU_Input_FastIncrease;
    } else if (Key_Check(KEY_SINGLE, 1)) {
      return 0;
      return MENU_Input_FastDecrease;
    }
    /*
    Menu_RotGet();
    int delta = (Menu_RotGetDelta(0) / 3) + MENU_Input_ScrollZeroPoint;
    if (delta < MENU_Input_ScrollUnderBound)
      delta = MENU_Input_ScrollUnderBound;
    else if (delta > MENU_Input_ScrollUpperBound)
      delta = MENU_Input_ScrollUpperBound;
    Menu_RotResetCurrent();
    */
  }
  if (Key_Check(KEY_SINGLE, 3)) {
    return MENU_Input_Exit;
  }
  return 0;
}
static int ProcessInputAdjust(Menu_States *self,
                              void (*fn_proc)(struct Menu_States_St *,
                                              int input),
                              void *userp) {
  UNUSED(userp);
  int in = GetInputAdjust();
  fn_proc(self, in);
  if (changed_flag || in != 0) {
    changed_flag = 0;
    return 1;
  } else {
    return 0;
  }
}
static int TransferToChange() { return MENU_Mode_Change; }
static int TransferToChangeWithRotReset() {
  Menu_RotReset();
  return MENU_Mode_Change;
}

extern Menu_Item main_page_entries[2];
Menu_Item main_page = {
    .name = "Main Menu",
    .attributes = MENU_Item_IsTop,
    .entries = main_page_entries,
    .entry_count = sizeof(main_page_entries) / sizeof(Menu_Item),
    .fn_acq_input = ProcessInputNormal,
};

extern Menu_Item pid_page_entries[4];
int process_mode = 0;

static void DisplayMode(uint8 line, uint8 begin, void *userp) {
  int *p = (int *)userp;
  OLED_Text_WriteDigit_R(&oled_handle, line, begin + 2, H_CountDigits(*p), *p,
                         ' ');
}
static void ProcessModeInput(int input, void *userp) {
  UNUSED(userp);
  if (input == MENU_Input_SelectUp) {
    changed_flag = 1;
    process_mode = (process_mode + 1) % 5;
  } else if (input == MENU_Input_SelectDown) {
    changed_flag = 1;
    process_mode = (process_mode - 1) % 5;
  }
}

Menu_Item main_page_entries[2] = {
    (Menu_Item){
        .name = "Mode",
        .attributes = MENU_Item_NoEnter | MENU_Item_NoTitle,
        .entries = NULL,
        .entry_count = 0,
        .userp = &process_mode,
        .fn_acq_input = ProcessInputNormal,
        .fn_on_entry = TransferToChange,
        .fn_on_put = DisplayMode,
        .fn_proc_input = ProcessModeInput,
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

enum {
  PIDDATA_INDEX_BAL = 0,
  PIDDATA_INDEX_VEL = 1,
  PIDDATA_INDEX_DIR = 2,
  PIDDATA_INDEX_POS = 3
};
struct PidData bal_pid_data, vel_pid_data, dir_pid_data, pos_pid_data;
struct PidData *PidGetData(int index) {
  switch (index) {
  case PIDDATA_INDEX_BAL:
    return &bal_pid_data;
  case PIDDATA_INDEX_VEL:
    return &vel_pid_data;
  case PIDDATA_INDEX_DIR:
    return &dir_pid_data;
  case PIDDATA_INDEX_POS:
    return &pos_pid_data;
  default:
    return &bal_pid_data; // Not Result In HardFault
  }
}
static void PidReadFromFlash() {
  struct PidData *pd = &bal_pid_data;
  Flash_Read(PIDDATA_INDEX_BAL);
  pd->kp = flash_union_buffer[0].int32_type;
  pd->ki = flash_union_buffer[1].int32_type;
  pd->kd = flash_union_buffer[2].int32_type;

  pd = &vel_pid_data;
  Flash_Read(PIDDATA_INDEX_VEL);
  pd->kp = flash_union_buffer[0].int32_type;
  pd->ki = flash_union_buffer[1].int32_type;
  pd->kd = flash_union_buffer[2].int32_type;

  pd = &dir_pid_data;
  Flash_Read(PIDDATA_INDEX_DIR);
  pd->kp = flash_union_buffer[0].int32_type;
  pd->ki = flash_union_buffer[1].int32_type;
  pd->kd = flash_union_buffer[2].int32_type;

  pd = &pos_pid_data;
  Flash_Read(PIDDATA_INDEX_POS);
  pd->kp = flash_union_buffer[0].int32_type;
  pd->ki = flash_union_buffer[1].int32_type;
  pd->kd = flash_union_buffer[2].int32_type;
}
static void PidWriteToFlash(int index) {
  struct PidData *pd = PidGetData(index);
  flash_union_buffer[0].int32_type = pd->kp;
  flash_union_buffer[1].int32_type = pd->ki;
  flash_union_buffer[2].int32_type = pd->kd;
  Flash_Write(index);
}

static void ChangePidVal(int input, void *userp) {
  int *p = (int *)userp;
  if (input == MENU_Input_Increase) {
    changed_flag = 1;
    ++*p;
  } else if (input == MENU_Input_Decrease) {
    changed_flag = 1;
    --*p;
  } else if (input == MENU_Input_FastIncrease) {
    changed_flag = 1;
    *p += 10;
  } else if (input == MENU_Input_FastDecrease) {
    changed_flag = 1;
    *p -= 10;
  } else if (input == MENU_Input_ResetToZero) {
    changed_flag = 1;
    *p = 0;
  }
}
static void DisplayPidVal(uint8 line, uint8 begin, void *userp) {
  int *p = (int *)userp;
  OLED_Text_WriteDigit_R(&oled_handle, line, begin + 1, H_CountDigits(*p), *p,
                         ' ');
}
static void ExitPidEditing(void *userp) {
  if (userp == &bal_pid_data) {
    PidWriteToFlash(PIDDATA_INDEX_BAL);
  } else if (userp == &vel_pid_data) {
    PidWriteToFlash(PIDDATA_INDEX_VEL);
  } else if (userp == &dir_pid_data) {
    PidWriteToFlash(PIDDATA_INDEX_DIR);
  } else if (userp == &pos_pid_data) {
    PidWriteToFlash(PIDDATA_INDEX_POS);
  }
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
        .userp = &bal_pid_data,
        .fn_acq_input = ProcessInputNormal,
        .fn_on_exit = ExitPidEditing,
    },
    (Menu_Item){
        .name = "Vel PID",
        .attributes = 0,
        .entries = vel_pid_item_entries,
        .entry_count = 3,
        .userp = &vel_pid_data,
        .fn_acq_input = ProcessInputNormal,
        .fn_on_exit = ExitPidEditing,
    },
    (Menu_Item){
        .name = "Dir PID",
        .attributes = 0,
        .entries = dir_pid_item_entries,
        .entry_count = 3,
        .userp = &dir_pid_data,
        .fn_acq_input = ProcessInputNormal,
        .fn_on_exit = ExitPidEditing,
    },
    (Menu_Item){
        .name = "Pos PID",
        .attributes = 0,
        .entries = pos_pid_item_entries,
        .entry_count = 3,
        .userp = &pos_pid_data,
        .fn_acq_input = ProcessInputNormal,
        .fn_on_exit = ExitPidEditing,
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
          .fn_acq_input = ProcessInputAdjust,                                  \
          .fn_on_entry = TransferToChangeWithRotReset,                         \
          .fn_proc_input = ChangePidVal,                                       \
          .fn_on_put = DisplayPidVal,                                          \
      },                                                                       \
      (Menu_Item){                                                             \
          .name = "KI=",                                                       \
          .attributes = MENU_Item_NoEnter | MENU_Item_NoTitle,                 \
          .entries = NULL,                                                     \
          .entry_count = 0,                                                    \
          .userp = &NAME##_pid_data.ki,                                        \
          .fn_acq_input = ProcessInputAdjust,                                  \
          .fn_on_entry = TransferToChangeWithRotReset,                         \
          .fn_proc_input = ChangePidVal,                                       \
          .fn_on_put = DisplayPidVal,                                          \
      },                                                                       \
      (Menu_Item){                                                             \
          .name = "KD=",                                                       \
          .attributes = MENU_Item_NoEnter | MENU_Item_NoTitle,                 \
          .entries = NULL,                                                     \
          .entry_count = 0,                                                    \
          .userp = &NAME##_pid_data.kd,                                        \
          .fn_acq_input = ProcessInputAdjust,                                  \
          .fn_on_entry = TransferToChangeWithRotReset,                         \
          .fn_proc_input = ChangePidVal,                                       \
          .fn_on_put = DisplayPidVal,                                          \
      },                                                                       \
  }

DEFINE_PID_ENTRY(bal);
DEFINE_PID_ENTRY(vel);
DEFINE_PID_ENTRY(dir);
DEFINE_PID_ENTRY(pos);

Menu_States self;
void Menu_Start() {
  self.handle = &oled_handle;
  Key_Init();
  OLED_Init();
  Menu_RotInit();
  PidReadFromFlash();
  Menu_Switch(&self, &main_page);
  Menu_OLED_WriteText(&self);
  Menu_OLED_Put(&self);
}
void Menu_Update() { Menu_Proc(&self); }
