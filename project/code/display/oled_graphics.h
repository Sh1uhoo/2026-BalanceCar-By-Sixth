#ifndef __BLE_OLED_GRAPHICS_H
#define __BLE_OLED_GRAPHICS_H
#include "oled_config.h"

void OLED_GRAP_SetPixel(int x, int y);
void OLED_GRAP_ClearScreen(void);
void OLED_GRAP_SwapBuffer(void);
void OLED_GRAP_Put(void);
void OLED_GRAP_DrawLine(int x0, int y0, int xe, int ye);
typedef struct {
  int x, y;
} Point_i;
void OLED_GRAP_DrawTriangle(Point_i p[3]);

typedef struct {
  float x, y;
} Point_f;
// 画折线图，自动寻找合适的比例
void OLED_GRAP_DrawLineGroup(Point_i box[2], float bound, Point_f ps[],
                             int num);

#endif // !__BLE_OLED_GRAPHICS_H
