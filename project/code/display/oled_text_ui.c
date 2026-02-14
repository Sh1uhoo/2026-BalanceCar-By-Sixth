#include "oled_text_ui.h"
#include "oled_char.h"
#include <stdint.h>

#ifdef OLED_Use_SCREEN_SSD1309
#include "oled_ssd1309.h"
#endif // OLED_Use_SCREEN_SSD1309

#define _OLED_Text_get(buf, h, w) ((buf) + (h) * this->width + (w))

static int IsSameTextBlock(const OLED_Text_Block *a,
                              const OLED_Text_Block *b) {
  return a->ch == b->ch && a->sty == b->sty;
}
#ifdef OLED_Use_SCREEN_SSD1309
struct OLED_FormatterInfo {
  OLED_Text_Block *block;
  const uint8_t *char_image;
};
static uint8_t callback_style_formatter(void *_t, uint32_t index) {
  struct OLED_FormatterInfo *this = (struct OLED_FormatterInfo *)_t;
  uint8_t data = this->char_image[index];
  if ((this->block->sty & STY_Underline) && index >= 8)
    data |= 0x40;
  if ((this->block->sty & STY_Deleteline) && index < 8)
    data |= 0x80;
  if ((this->block->sty & STY_InverseColor))
    data = ~data;
  return data;
}
static void OLED_Text_PutChar(uint8_t page, uint8_t start_column,
                        OLED_Text_Block *block) {
  struct OLED_FormatterInfo this = {
      .block = block,
      .char_image = OLED_Char_GetImage(block->ch),
  };
  OLED_SetColumnAddress(start_column, start_column + 7);
  OLED_SetPageAddress(page, page + 1);
  OLED_WriteData_fn(callback_style_formatter, 16, (void *)&this);
}
void OLED_Text_Put(OLED_Text_Handle *this) {
  OLED_WriteCmd_2(OLED_CMD_SET_MEM_ADDR_MODE, OLED_CMD_MEM_ADDR_MODE_HADDR);
  for (int i = 0; i < this->height; ++i)
    for (int j = 0; j < this->width; ++j)
      if (!IsSameTextBlock(_OLED_Text_get(this->curr_buffer, i, j),
                              _OLED_Text_get(this->last_buffer, i, j)))
        OLED_Text_PutChar((this->height_offset + i) * 2,
                           (this->width_offset + j) * 8,
                           _OLED_Text_get(this->curr_buffer, i, j));
  OLED_WriteCmd_2(OLED_CMD_SET_MEM_ADDR_MODE, OLED_CMD_MEM_ADDR_MODE_PADDR);
#ifdef OLED_Use_BufferedI2C
  OLED_WriteFlush();
#endif // OLED_Use_BufferedI2C
}
#endif // OLED_Use_SCREEN_SSD1309
void OLED_Text_SwapBuffer(OLED_Text_Handle *this) {
  OLED_Text_Block *temp = this->curr_buffer;
  this->curr_buffer = this->last_buffer;
  this->last_buffer = temp;
}

void OLED_Text_Fill(OLED_Text_Handle *this, int ch, uint32_t sty) {
  for (int i = 0, j; i < this->height; ++i)
    for (j = 0; j < this->width; ++j)
      _OLED_Text_get(this->curr_buffer, i, j)
          ->ch = ch,
        _OLED_Text_get(this->curr_buffer, i, j)->sty = sty;
}
uint8_t OLED_Text_Write(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                        const char *str, OLED_Text_Style sty) {
  int i;
  for (i = 0; *str && offset + i < this->width; ++i, ++str) {
    _OLED_Text_get(this->curr_buffer, line, offset + i)->ch = *str;
    _OLED_Text_get(this->curr_buffer, line, offset + i)->sty = sty;
  }
  return i;
}
uint8_t OLED_Text_Write2(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                         const char *str, uint8_t max_length,
                         OLED_Text_Style sty) {
  int i;
  for (i = 0; *str && offset + i < this->width && i < max_length; ++i, ++str) {
    _OLED_Text_get(this->curr_buffer, line, offset + i)->ch = *str;
    _OLED_Text_get(this->curr_buffer, line, offset + i)->sty = sty;
  }
  return i;
}
uint8_t OLED_Text_WriteText(OLED_Text_Handle *this, uint8_t line,
                            uint8_t offset, const char *str) {
  int i;
  for (i = 0; *str && offset + i < this->width; ++i, ++str)
    _OLED_Text_get(this->curr_buffer, line, offset + i)->ch = *str;
  return offset + i;
}
void OLED_Text_WriteChar(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                         int ch) {
  _OLED_Text_get(this->curr_buffer, line, offset)->ch = ch;
}
uint8_t OLED_Text_WriteText2(OLED_Text_Handle *this, uint8_t line,
                             uint8_t offset, const char *str,
                             uint8_t max_length) {
  int i;
  for (i = 0; *str && offset + i < this->width && i < max_length; ++i, ++str)
    _OLED_Text_get(this->curr_buffer, line, offset + i)->ch = *str;
  return offset + i;
}
uint8_t OLED_Text_WriteDigit_R(OLED_Text_Handle *this, uint8_t line,
                               uint8_t offset, uint8_t length, int x,
                               int fill_char) {
  int i = offset + length - 1;
  if (x != 0) {
    int sign;
    if ((sign = x < 0))
      x = -x;
    for (; i >= offset && x; --i, x = x / 10)
      _OLED_Text_get(this->curr_buffer, line, i)->ch = '0' + x % 10;
    if (sign && i >= offset)
      _OLED_Text_get(this->curr_buffer, line, i--)->ch = '-';
  } else {
    _OLED_Text_get(this->curr_buffer, line, i--)->ch = '0';
  }
  int r = i;
  for (; i >= offset; --i)
    _OLED_Text_get(this->curr_buffer, line, i)->ch = fill_char;
  return r;
}
uint8_t OLED_Text_WriteDigit_L(OLED_Text_Handle *this, uint8_t line,
                               uint8_t offset, uint8_t length, int x) {
  int i = offset;
  if (x != 0) {
    int sign;
    if ((sign = x < 0)) {
      x = -x;
      _OLED_Text_get(this->curr_buffer, line, i++)->ch = '-';
    }
    for (; i <= offset + length && x; ++i, x = x / 10)
      _OLED_Text_get(this->curr_buffer, line, i)->ch = '0' + x % 10;
  } else {
    _OLED_Text_get(this->curr_buffer, line, i++)->ch = '0';
  }
  return i;
}
void OLED_Text_WriteStyle(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                          OLED_Text_Style sty) {
  for (int i = 0; offset + i < this->width; ++i)
    _OLED_Text_get(this->curr_buffer, line, offset + i)->sty = sty;
}
void OLED_Text_WriteStyle2(OLED_Text_Handle *this, uint8_t line, uint8_t offset,
                           uint8_t length, OLED_Text_Style sty) {
  for (int i = 0; i < length && offset + i < this->width; ++i)
    _OLED_Text_get(this->curr_buffer, line, offset + i)->sty = sty;
}
