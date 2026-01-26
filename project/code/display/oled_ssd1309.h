#ifndef __BLE_OLED_SSD1309_H
#define __BLE_OLED_SSD1309_H
#include "oled_config.h"
#include <stdint.h>

#ifdef OLED_Use_SoftwareI2C
void OLED_I2C_Init(void);
void OLED_I2C_Begin(void);
void OLED_I2C_End(void);
void OLED_I2C_SendByte(uint8_t);
#elif defined(OLED_Use_I2C_DMA)
void OLED_I2C_Init(void);
void OLED_WriteFlush(void);
#define OLED_Use_BufferedI2C
#endif // !OLED_Use_SoftwareI2C

// init for ssd1309 & ssd1306
#define OLED_SCREEN_Type_SSD1309
void OLED_Init(void);
void OLED_ClearScreen();

#define OLED_SCREEN_Width (128)
#define OLED_SCREEN_Height (64)

// Error codes
enum {
  OLED_E_OK = 0,
  OLED_E_NOACK = -1,
};

#ifdef OLED_Use_SoftwareI2C
#define OLED_DeviceAddr 0x3c // or 0x3D
#define OLED_ADDR_W ((OLED_DeviceAddr << 1) | 0x0)
#define OLED_ADDR_R ((OLED_DeviceAddr << 1) | 0x1)
#endif // !OLED_Use_SoftwareI2C
#ifdef OLED_Use_I2C_DMA
#define OLED_DeviceAddr 0x3C // or 0x3D
#endif                       // OLED_Use_I2C_DMA

#define OLED_CTRLBYTE_COMMANDS 0x00
#define OLED_CTRLBYTE_DATA 0x40

void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteCmd_2(uint8_t cmd, uint8_t data);
void OLED_WriteCmd_N(uint8_t cmd, const uint8_t *pdata, uint8_t length);

void OLED_WriteData(const uint8_t *data, uint32_t length);
void OLED_WriteData_fn(uint8_t (*fn)(void *, uint32_t), uint32_t length,
                       void *this);
void OLED_WriteData_cl(uint8_t clear_value, uint32_t length);

/* Fundamental Commands
 */
// 1. Set Contrast Control (contrastVal : [0:255])
#define OLED_CMD_SET_CONTRAST 0x81
// 2. Entire Display (NO ARG)
#define OLED_CMD_ENTIRE_DISPLAY_ON 0xA5
#define OLED_CMD_ENTIRE_DISPLAY_OFF 0xA4
// 3. Set Normal/Inverse Display (NO ARG)
#define OLED_CMD_NORMAL_DISPLAY 0xA6
#define OLED_CMD_INVERSE_DISPLAY 0xA7
// 4. Set Display ON/OFF (NO ARG)
#define OLED_CMD_DISPLAY_ON 0xAF
#define OLED_CMD_DISPLAY_OFF 0xAE
// 5. NOP (NO ARG)
#define OLED_CMD_NOP 0xE3
// 6. Set Command Lock (status : OLED_CMD_LOCK)
#define OLED_CMD_LOCK_COMMAND 0xFD
typedef enum {
  OLED_CMD_LOCK_DISENABLE = 0x12,
  OLED_CMD_LOCK_ENABLE = 0x16
} OLED_CMD_LOCK;

/* Scrolling Commands
 */
// 7. Continuous Horizontal Scroll Setup (COMPLEX)
#define OLED_CMD_RHSCROLL_SETUP 0x26
#define OLED_CMD_LHSCROLL_SETUP 0x27
// 8. Continuous Vertical&Horizontal Scroll Setup (COMPLEX)
#define OLED_CMD_VRHSCROLL_SETUP 0x29
#define OLED_CMD_VLHSCROLL_SETUP 0x2A
// 9. Activate/deactivate Scroll (NO ARG)
#define OLED_CMD_SCROLL_DEACTIVATE 0x2E
#define OLED_CMD_SCROLL_ACTIVATE X2F
// 10. Set Vertical Scroll Area (COMPLEX)
#define OLED_CMD_SET_VSCROLL_AREA 0xA3
// 11. Content Scroll Setup (COMPLEX)
#define OLED_CMD_CONTENT_RSCROLL_SETUP 0x2C
#define OLED_CMD_CONTENT_LSCROLL_SETUP 0x2D

/* Addressing Setting Commands
 */
// 12. Set Column Start Address for Page Addressing Mode
void OLED_PAMSetColumnStartAddress(uint8_t addr);
// 13. Set Memory Addressing Mode (mode)
#define OLED_CMD_SET_MEM_ADDR_MODE 0x20
typedef enum {
  OLED_CMD_MEM_ADDR_MODE_HADDR = 0x00, /* Horizontal Addressing */
  OLED_CMD_MEM_ADDR_MODE_VADDR = 0x01, /* Vertical Addressing */
  OLED_CMD_MEM_ADDR_MODE_PADDR = 0x02  /* Page Addressing */
} OLED_CMD_MEM_ADDR_MODE;
// 14. Set Column Address (COMPLEX)
#define OLED_CMD_SET_COL_ADDR 0x21
void OLED_SetColumnAddress(uint8_t start, uint8_t end);
// 15. Set Page Address (COMPLEX)
#define OLED_CMD_SET_PAGE_ADDR 0x22
void OLED_SetPageAddress(uint8_t start, uint8_t end);
// 16. Set Page Start Address for Page Addressing Mode(PAM)
//     CMD = OLED_CMD_PAM_SET_START_PAGE + page_index([0:7])
#define OLED_CMD_PAM_SET_START_PAGE 0xB0

/* Hardware Configuration Commands
 */
// 17. Set Display Start Line
//     CMD = OLED_CMD_SET_DISPLAY_START_LINE + line_index([0:63])
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
// 18. Set Segment Remap (NO ARG)
#define OLED_CMD_SEG_REMAP_RESET 0xA0
#define OLED_CMD_SEG_REMAP_ON 0xA1
// 19. Set Multiplex Ratio (N : [15:63]) MUX <- N+1
#define OLED_CMD_SET_MULTIPLEX_RATIO 0xA8
// 20. Set COM Output Scan Direction (NO ARG)
#define OLED_CMD_SET_COMOUT_SCAN_DIR_NORMAL 0xC0
#define OLED_CMD_SET_COMOUT_SCAN_DIR_INVERSE 0xC8
// 21. Set Display Offset (shift : [0:63])
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3
// 22. Set COM Pins Hardware Configuration (COMPLEX)
#define OLED_CMD_SET_COM_PIN_CONFIG 0xDA
// 23. Set GPIO (COMPLEX)
// 24. Set Charge Pump (state : OLED_CMD_CHARGE_PUMP_STATE)
#define OLED_CMD_SET_CHARGE_PUMP 0x8D
typedef enum {
  OLED_CMD_CHARGE_PUMP_ENABLE = 0x14,
  OLED_CMD_CHARGE_PUMP_DISABLE = 0x10,
} OLED_CMD_CHARGE_PUMP_STATE;

/* Timing&Driving Scheme Setting Commands
 */
#define OLED_CMD_SET_GPIO 0xDC
// 25. Set Display Clock Divide Ratio (COMPLEX)
#define OLED_CMD_SET_DISPLAY_CLKDIV 0xD5
void OLED_SetDisplayClock(uint8_t clkdiv, uint8_t freq_osc);
// 26. Set Pre-charge Period (COMPLEX)
#define OLED_CMD_SET_PRECHARGE_PERIOD 0xD9
void OLED_SetPrechargePeriod(uint8_t phase1, uint8_t phase2);
// 27. Set V_COMH Deselect Level (vcomh : OLED_CMD_VCOMH_LEVEL)
#define OLED_CMD_SET_VCOMH_LEVEL 0xDB
typedef enum {
  OLED_CMD_VCOMH_0VCC64 = 0x00,
  OLED_CMD_VCOMH_0VCC78 = 0x34,
  OLED_CMD_VCOMH_0VCC84 = 0x3C
} OLED_CMD_VCOMH_LEVEL;

/* Read Command
 */
// 28. Read Command
//     Read command is not supported by I2C.

#endif // !__BLE_OLED_SSD1309_H
