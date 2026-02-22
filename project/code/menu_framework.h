#ifndef MENU_MODU_H_FILE_
#define MENU_MODU_H_FILE_
#include "display/oled_text_ui.h"
#include "zf_common_headfile.h"
#include <stdint.h>

#define MENU_STACK_MAX 6
enum {
  MENU_Mode_Normal = 0,
  MENU_Mode_Change = 1,
  MENU_Mode_Exit = 2,
};
enum {
  // 标记选项为顶级菜单
  MENU_Item_IsTop = 0x1,
  // 标记选项没有标题
  MENU_Item_NoTitle = 0x2,
  // 标记选项不能进入。例如数据条目显示
  MENU_Item_NoEnter = 0x4,
  MENU_Item_ChangedFlag = 0x8000,
  MENU_Item_NoRender = 0x8,
};
enum {
  MENU_Input_Enter = 'L',
  MENU_Input_Exit = 'H',
  MENU_Input_SelectUp = 'K',
  MENU_Input_SelectDown = 'J',
  MENU_Input_ResetToZero = 'Z',
  MENU_Input_Increase = '+',
  MENU_Input_FastIncrease = 'I',
  MENU_Input_Decrease = '-',
  MENU_Input_FastDecrease = 'D',
  MENU_Input_ScrollZeroPoint = 1024,
  MENU_Input_ScrollUnderBound = MENU_Input_ScrollZeroPoint - 127,
  MENU_Input_ScrollUpperBound = MENU_Input_ScrollZeroPoint + 127,
};
enum { MENU_Render_TextMode = 0, MENU_Render_Otherwise = 2 };
struct Menu_States_St;
struct Menu_Item_St {
  const char *name;
  uint16_t attributes;
  uint16_t entry_count;
  struct Menu_Item_St *entries;

  void *userp;
  /* 以下回调函数可以为NULL */

  // 在进入菜单选项时调用
  // 返回选项进入后所在的模式，MENU_Mode_Exit表示不可进入
  int (*fn_on_entry)(void *userp);
  // 在退出菜单选项时调用
  void (*fn_on_exit)(void *userp);
  // 选项处理输入调用
  void (*fn_proc_input)(int input, void *userp);
  // 在向OLED上输出时调用, 仅对只显示一个数据条目的有用
  void (*fn_on_put)(uint8_t line, uint8_t offset, void *userp);
  // 在ChangeMode下，选项处理输入时调用，不能为空，除非上级菜单选项有处理函数
  // 在获取输入后使用该输入调用fn_proc，返回选项是否被更改
  int (*fn_acq_input)(struct Menu_States_St *,
                      void (*fn_proc)(struct Menu_States_St *, int input),
                      void *userp);
  // 使用此函数来进行整个屏幕的渲染（如果需要），mode
  // 表示模式，0表示渲染，1表示发送（两阶段）
  void (*fn_render)(int mode, struct Menu_States_St *, void *userp);
};
typedef struct Menu_Item_St Menu_Item;
struct Menu_States_St {
  Menu_Item *current;
  int16_t select_index;
  uint16_t cur_mode;
  Menu_Item *ptr_stack[MENU_STACK_MAX];
  int16_t ind_stack[MENU_STACK_MAX];
  int32_t stack_top;

  int render_mode;

  OLED_Text_Handle *handle;
};
typedef struct Menu_States_St Menu_States;
// 切换到以top为初始的菜单
void Menu_Switch(Menu_States *self, Menu_Item *top);
// 处理菜单事项
// 在有必要时自动输出菜单到OLED上
void Menu_Proc(Menu_States *self);

void Menu_OLED_WriteText(Menu_States *self);
void Menu_OLED_Put(Menu_States *self);
#endif // !MENU_MODU_H_FILE_
