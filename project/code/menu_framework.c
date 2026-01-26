#include "menu_framework.h"
#include <stdint.h>
#include <string.h>

void Menu_Switch(Menu_States *self, Menu_Item *top) {
  self->current = top;
  self->select_index = top->entry_count ? 0 : -1;
  self->cur_mode = MENU_Mode_Normal;
  self->stack_top = 0;
}
// NOTE: When stack depth won't increase, return NULL
static Menu_Item *MNormal_Next(Menu_States *, int);
static void MChange_Next(Menu_States *, int);
static void Next(Menu_States *, int input);

static void PushStack(Menu_States *self) {
  self->ptr_stack[self->stack_top] = self->current;
  self->ind_stack[self->stack_top++] = self->select_index;
}
static void PopStack(Menu_States *self) {
  self->current = self->ptr_stack[--self->stack_top];
  self->select_index = self->ind_stack[self->stack_top];
}
#define CheckChangeAndClear(menu)                                              \
  (((menu)->attributes & MENU_Item_ChangedFlag) &&                             \
   ((menu)->attributes &= ~MENU_Item_ChangedFlag, 1))
void Menu_Proc(Menu_States *self) {
  Menu_Item *cursel;
  Menu_Item *current = self->current;
  int changed = 0;
  if (self->select_index >= 0) {
    cursel = &current->entries[self->select_index];
    if (self->cur_mode == MENU_Mode_Change && cursel->fn_acq_input)
      changed = changed | cursel->fn_acq_input(self, Next, cursel->userp);
    else
      goto Input_Unfinished;
  } else {
  Input_Unfinished:
    changed = changed | current->fn_acq_input(self, Next, current->userp);
  }
  if (CheckChangeAndClear(current))
    changed = 1;
  for (int i = 0; i < current->entry_count; ++i)
    if (CheckChangeAndClear(&current->entries[i]))
      changed = 1;
  if (changed)
    Menu_OLED_WriteText(self), Menu_OLED_Put(self);
}
void Next(Menu_States *self, int input) {
  Menu_Item *next;
  switch (self->cur_mode) {
  case MENU_Mode_Normal:
    next = MNormal_Next(self, input);
    break;
  case MENU_Mode_Change:
    MChange_Next(self, input);
    next = NULL;
    break;
  }
  if (next) {
    PushStack(self);
    self->current = next;
    self->select_index = next->entry_count ? 0 : -1;
  }
}
Menu_Item *MNormal_Next(Menu_States *self, int input) {
  Menu_Item *current = self->current;
  int16_t select = self->select_index;
  if (input == MENU_Input_Enter && select >= 0) {
    Menu_Item *cursel = &current->entries[select];
    int mode = self->cur_mode;
    if (cursel->fn_on_entry) {
      mode = cursel->fn_on_entry(cursel->userp);
      if (mode == MENU_Mode_Exit)
        return NULL;
    }
    self->cur_mode = mode;
    if (!(cursel->attributes & MENU_Item_NoEnter))
      return cursel;
  } else if (input == MENU_Input_Exit) {
    if (current->fn_on_exit)
      current->fn_on_exit(current->userp);
    if (self->stack_top > 0)
      PopStack(self);
  } else if (input == MENU_Input_SelectUp) {
    if (select >= 0)
      self->select_index =
          (select - 1 + current->entry_count) % current->entry_count;
    current->attributes |= MENU_Item_ChangedFlag;
  } else if (input == MENU_Input_SelectDown) {
    if (select >= 0)
      self->select_index = (select + 1) % current->entry_count;
    current->attributes |= MENU_Item_ChangedFlag;
  }
  return NULL;
}
void MChange_Next(Menu_States *self, int input) {
  Menu_Item *cursel = &self->current->entries[self->select_index];
  if (input == MENU_Input_Exit) {
    self->cur_mode = MENU_Mode_Normal;
    if (cursel->fn_on_exit)
      cursel->fn_on_exit(cursel->userp);
  } else if (cursel->fn_proc_input)
    cursel->fn_proc_input(input, cursel->userp);
}

void Menu_OLED_WriteText(Menu_States *this) {
  OLED_Text_Handle *hnd = this->handle;
  Menu_Item *current = this->current;
  OLED_Text_Fill(hnd, ' ', STY_NONE);
  uint8_t line = 0;
  if (!(current->attributes & MENU_Item_NoTitle) && hnd->height > 1) {
    OLED_Text_Write2(hnd, line, 0, current->name,
                     hnd->width - (hnd->width > 8 ? 3 : 0), STY_NONE);
    if (hnd->width > 8) {
      const char *mode_str = "[N]";
      if (this->cur_mode == MENU_Mode_Change)
        mode_str = "[C]";
      OLED_Text_Write(hnd, line, hnd->width - 3, mode_str, STY_NONE);
    }
    line++;
  }
  int i, endi;
  if (current->entry_count > (hnd->height - line)) {
    i = this->select_index > 1 ? this->select_index : 0;
    endi = i + (hnd->height - line);
    if (endi > current->entry_count)
      endi = current->entry_count;
  } else if (current->entry_count > 0) {
    i = 0;
    endi = current->entry_count;
  } else
    return;
  for (Menu_Item *cur; i < endi; ++i) {
    cur = &current->entries[i];
    int end = OLED_Text_WriteText(hnd, line, 1, cur->name);
    if (i == this->select_index) {
      OLED_Text_WriteText(hnd, line, 0, ">");
      OLED_Text_WriteStyle(hnd, line, 0, STY_InverseColor);
    }
    if (cur->fn_on_put)
      cur->fn_on_put(line, end, cur->userp);
    line++;
  }
}
void Menu_OLED_Put(Menu_States *self) {
  OLED_Text_Put(self->handle);
  OLED_Text_SwapBuffer(self->handle);
}
