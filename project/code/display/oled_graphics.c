#include "oled_graphics.h"
#include "oled_ssd1309.h"
#include <math.h>
#include <stdint.h>

/*
 * *-------OLED_SCREEN_Width------->  X
 * |
 * |
 * OLED_
 * SCREEN_
 * Height
 * |
 * |
 * Y
 */

typedef uint8_t OLED_GRAP_FBbuffer[OLED_SCREEN_Width][OLED_SCREEN_Height / 8];
OLED_GRAP_FBbuffer OLED_GRAP_fbuffer[2] = {0};
OLED_GRAP_FBbuffer *OLED_GRAP_current_buffer = OLED_GRAP_fbuffer + 0;
OLED_GRAP_FBbuffer *OLED_GRAP_last_buffer = OLED_GRAP_fbuffer + 1;

#ifdef OLED_Use_SCREEN_SSD1309
void OLED_GRAP_Put(void) {
  OLED_WriteCmd_2(OLED_CMD_SET_MEM_ADDR_MODE, OLED_CMD_MEM_ADDR_MODE_VADDR);
  OLED_SetColumnAddress(0, 127);
  OLED_SetPageAddress(0, 7);
  OLED_WriteData(&(*OLED_GRAP_current_buffer)[0][0],
                 sizeof(OLED_GRAP_FBbuffer));
  OLED_WriteCmd_2(OLED_CMD_SET_MEM_ADDR_MODE, OLED_CMD_MEM_ADDR_MODE_PADDR);
}
#endif

void OLED_GRAP_SetPixel(int x, int y) {
  if (x >= OLED_SCREEN_Width || y >= OLED_SCREEN_Height)
    return;
  if (x < 0 || y < 0)
    return;
  (*OLED_GRAP_current_buffer)[x][y >> 3] |= 1 << (y & 0x7);
}
void OLED_GRAP_ClearScreen(void) {
  for (int x = 0; x < OLED_SCREEN_Width; ++x)
    for (int y = 0; y < OLED_SCREEN_Height; ++y)
      (*OLED_GRAP_current_buffer)[x][y] = 0x00;
}
void OLED_GRAP_SwapBuffer(void) {
  OLED_GRAP_FBbuffer *temp = OLED_GRAP_current_buffer;
  OLED_GRAP_current_buffer = OLED_GRAP_last_buffer;
  OLED_GRAP_last_buffer = temp;
}
void OLED_GRAP_DrawLine(int x0, int y0, int xe, int ye) {
  int dx = xe - x0;
  int dy = ye - y0;
  int step_x = dx >= 0 ? 1 : -1;
  int step_y = dy >= 0 ? 1 : -1;
  dx = dx < 0 ? -dx : dx;
  dy = dy < 0 ? -dy : dy;

  if (dx > dy) { // |m| < 1
    int p = 2 * dy - dx;
    int y = y0;
    for (int x = x0; x != xe; x += step_x) {
      OLED_GRAP_SetPixel(x, y);
      if (p >= 0) {
        p -= 2 * dx;
        y += step_y;
      }
      p += 2 * dy;
    }
  } else {
    int p = 2 * dx - dy;
    int x = x0;
    for (int y = y0; y != ye; y += step_y) {
      OLED_GRAP_SetPixel(x, y);
      if (p >= 0) {
        p -= 2 * dy;
        x += step_x;
      }
      p += 2 * dx;
    }
  }
}
void OLED_GRAP_DrawTriangle(Point_i p[3]) {
  OLED_GRAP_DrawLine(p[0].x, p[0].y, p[1].x, p[1].y);
  OLED_GRAP_DrawLine(p[1].x, p[1].y, p[2].x, p[2].y);
  OLED_GRAP_DrawLine(p[2].x, p[2].y, p[0].x, p[1].y);
}

void OLED_GRAP_DrawLineGroup(Point_i box[2], float bound, Point_f ps[],
                             int num) {
  int i;
  float x_upper, x_lower, y_upper, y_lower;
  if (num == 0)
    return;
  x_lower = x_upper = ps[0].x;
  y_lower = y_upper = ps[0].y;
  for (i = 1; i < num; ++i) {
    float x, y;
    x = ps[i].x, y = ps[i].y;
    if (x > x_upper)
      x_upper = x;
    if (x < x_lower)
      x_lower = x;
    if (y > y_upper)
      y_upper = y;
    if (y < y_lower)
      y_lower = y;
  }
  x_lower -= bound, x_upper += bound;
  y_lower -= bound, y_upper += bound;

  float x_scale, y_scale;
  x_scale = (box[1].x - box[0].x) / (x_upper - x_lower);
  y_scale = (box[1].x - box[0].x) / (y_upper - y_lower);
  for (i = 0; i < num - 1; ++i) {
    OLED_GRAP_DrawLine(box[0].x + roundf((ps[i].x - x_lower) * x_scale),
                       box[0].y + roundf((ps[i].y - y_lower) * y_scale),
                       box[1].x + roundf((ps[i + 1].x - x_lower) * x_scale),
                       box[1].y + roundf((ps[i + 1].y - y_lower) * y_scale));
  }
}
