#ifndef __BLE_OLED_TEXT_UI_H
#define __BLE_OLED_TEXT_UI_H
#include "oled_config.h"
#include <stdint.h>

#define OLED_MAX_TScreen_Height (OLED_SCREEN_Height / 16)
#define OLED_MAX_TScreen_Width (OLED_SCREEN_Width / 8)

typedef char CharT;
typedef struct {
  CharT ch;
  uint16_t sty;
} OLED_Text_Block;
typedef enum {
  STY_NONE = 0,
  STY_Underline = 0x1,
  STY_Deleteline = 0x2,
  STY_InverseColor = 0x4,
} OLED_Text_Style;
typedef struct {
  // change virtual screen size in char size
  uint8_t width_offset, height_offset;
  uint8_t width, height;
  OLED_Text_Block *curr_buffer, *last_buffer;
} OLED_Text_Handle;

#define OLED_Text_M_MakeHnd(name, w, h, woff, hoff)                            \
  static OLED_Text_Block(name##_buffers)[2][(h)][(w)] = {0};                   \
  OLED_Text_Handle(name) = {.width_offset = (woff),                            \
                            .height_offset = (hoff),                           \
                            .width = (w),                                      \
                            .height = (h),                                     \
                            .curr_buffer = &(name##_buffers)[0][0][0],         \
                            .last_buffer = &(name##_buffers)[1][0][0]}

void OLED_Text_Fill(OLED_Text_Handle *this, int ch, uint32_t style);
void OLED_Text_Put(OLED_Text_Handle *this);
void OLED_Text_SwapBuffer(OLED_Text_Handle *this);
uint8_t OLED_Text_Write(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                        const char *str, OLED_Text_Style style);
uint8_t OLED_Text_Write2(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                         const char *str, uint8_t max_length,
                         OLED_Text_Style style);
uint8_t OLED_Text_WriteText(OLED_Text_Handle *this, uint8_t line,
                            uint8_t offset, const char *str);
uint8_t OLED_Text_WriteText2(OLED_Text_Handle *this, uint8_t line,
                             uint8_t offset, const char *str,
                             uint8_t max_length);
void OLED_Text_WriteChar(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                         int ch);
uint8_t OLED_Text_WriteDigit_R(OLED_Text_Handle *this, uint8_t line,
                               uint8_t offset, uint8_t length, int x,
                               int fill_char);
uint8_t OLED_Text_WriteDigit_L(OLED_Text_Handle *this, uint8_t line,
                               uint8_t offset, uint8_t length, int x);
void OLED_Text_WriteStyle(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                          OLED_Text_Style style);
void OLED_Text_WriteStyle2(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                           uint8_t length, OLED_Text_Style style);

#endif // !__BLE_OLED_TEXT_UI_H
