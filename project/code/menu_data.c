#include "menu_data.h"
#include "Key.h"
#include "display/oled_ssd1309.h"
#include "display/oled_text_ui.h"
#include "menu_framework.h"
#include "menu_rot_sensor.h"
#include "zf_common_typedef.h"
#include "zf_driver_flash.h"
#include <stdio.h>
#include <string.h>

#define UNUSED(x) ((void)(x))

OLED_Text_M_MakeHnd(oled_handle, 16, 4, 0, 0);
OLED_Text_Handle *oled_text_hnd = &oled_handle;
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
static struct PidData back_pid_data[4];

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
static uint32_t write_to_sector = 126, write_to_page = 0, write_head_value = 0;
static void PidReadFromFlash() {
  uint32_t max, headval, headpage, headsector;
  for (uint32_t sector = 126; sector <= 127; ++sector) {
    for (uint32_t i = 0; i < 4; ++i) {
      flash_buffer_clear();
      flash_read_page_to_buffer(126, i);
      headval = flash_union_buffer[0].uint32_type;
      max = headval > max ? (headpage = i, headsector = sector, headval) : max;
    }
  }
  write_to_sector = headsector;
  write_to_page = headpage;
  write_head_value = headval;
  flash_buffer_clear();
  flash_read_page_to_buffer(headsector, headpage);
  for (int j = 0; j < 4; ++j) {
    back_pid_data[j].kp = flash_union_buffer[1 + j * 3 + 0].int32_type;
    back_pid_data[j].ki = flash_union_buffer[1 + j * 3 + 1].int32_type;
    back_pid_data[j].kd = flash_union_buffer[1 + j * 3 + 2].int32_type;
  }
  memcpy(&bal_pid_data, back_pid_data + PIDDATA_INDEX_BAL,
         sizeof(struct PidData));
  memcpy(&vel_pid_data, back_pid_data + PIDDATA_INDEX_VEL,
         sizeof(struct PidData));
  memcpy(&dir_pid_data, back_pid_data + PIDDATA_INDEX_DIR,
         sizeof(struct PidData));
  memcpy(&pos_pid_data, back_pid_data + PIDDATA_INDEX_POS,
         sizeof(struct PidData));
}
static int ComparePidData(const struct PidData *a, const struct PidData *b) {
  return a->kp == b->kp && a->ki == b->ki && a->kd == b->kd;
}
static void PidWriteToFlash() {
  if (ComparePidData(&back_pid_data[PIDDATA_INDEX_VEL], &vel_pid_data) &&
      ComparePidData(&back_pid_data[PIDDATA_INDEX_BAL], &bal_pid_data) &&
      ComparePidData(&back_pid_data[PIDDATA_INDEX_DIR], &dir_pid_data) &&
      ComparePidData(&back_pid_data[PIDDATA_INDEX_POS], &pos_pid_data))
    return;
  if (write_to_page == 3)
    write_to_sector = write_to_sector == 127 ? 126 : 127;
  write_to_page = (write_to_page + 1) % 4;
  write_head_value += 1;
  memcpy(back_pid_data + PIDDATA_INDEX_BAL, &bal_pid_data,
         sizeof(struct PidData));
  memcpy(back_pid_data + PIDDATA_INDEX_DIR, &dir_pid_data,
         sizeof(struct PidData));
  memcpy(back_pid_data + PIDDATA_INDEX_POS, &pos_pid_data,
         sizeof(struct PidData));
  memcpy(back_pid_data + PIDDATA_INDEX_VEL, &vel_pid_data,
         sizeof(struct PidData));

  flash_buffer_clear();
  flash_union_buffer[0].uint32_type = write_head_value;
  for (int j = 0; j < 4; ++j) {
    flash_union_buffer[1 + j * 3 + 0].int32_type = back_pid_data[j].kp;
    flash_union_buffer[1 + j * 3 + 1].int32_type = back_pid_data[j].ki;
    flash_union_buffer[1 + j * 3 + 2].int32_type = back_pid_data[j].kd;
  }
  if (flash_check(write_to_sector, write_to_page))
    flash_erase_page(write_to_sector, write_to_page);
  flash_write_page_from_buffer(write_to_sector, write_to_page);
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
  UNUSED(userp);
  PidWriteToFlash();
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
