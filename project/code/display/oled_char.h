#ifndef __BLE_OLED_CHAR_H
#define __BLE_OLED_CHAR_H
#include "oled_config.h"

#include <stdint.h>
#ifdef OLED_CharTbl_ascii
extern const uint8_t OLED_char_tbl_ascii['~'-' '+1][16];
#define OLED_Char_Width (8)
#define OLED_Char_Height (16)
#endif

const uint8_t* OLED_Char_GetImage(int codepoint);

#endif // !__BLE_OLED_CHAR_H
