#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

#define KEY_HOLD 0x01
#define KEY_DOWN 0x02
#define KEY_UP 0x04
#define KEY_SINGLE 0x08
#define KEY_DOUBLE 0x10
#define KEY_LONG 0x20

void Key_Init(void);
uint8_t Key_Check(uint8_t Flag, uint8_t n);
void Key_Tick(void);

#endif
