#include "oled_ssd1309.h"
#include "zf_device_oled.h"
#include "zf_driver_delay.h"
#include <stdint.h>

void OLED_Init(void) {
  system_delay_us(10);
#if defined(OLED_Use_SoftwareI2C) || defined(OLED_Use_I2C_DMA)
  OLED_I2C_Init();
#endif // !OLED_Use_SoftwareI2C
  OLED_WriteCmd(OLED_CMD_DISPLAY_OFF);
  OLED_SetDisplayClock(0x0, 0x8);

  OLED_WriteCmd_2(OLED_CMD_SET_MULTIPLEX_RATIO, 0x3F);
  OLED_WriteCmd_2(OLED_CMD_SET_DISPLAY_OFFSET, 0x00);
  OLED_WriteCmd(OLED_CMD_SET_DISPLAY_START_LINE | 0x00);

  OLED_WriteCmd(OLED_CMD_SEG_REMAP_ON);
  OLED_WriteCmd(OLED_CMD_SET_COMOUT_SCAN_DIR_INVERSE);

  OLED_WriteCmd_2(OLED_CMD_SET_COM_PIN_CONFIG, 0x12);

  OLED_WriteCmd_2(OLED_CMD_SET_CONTRAST, 0xCF);

  OLED_SetPrechargePeriod(0x1, 0xF);
  OLED_WriteCmd_2(OLED_CMD_SET_VCOMH_LEVEL, 0x30);

  OLED_WriteCmd(OLED_CMD_ENTIRE_DISPLAY_OFF);

  OLED_WriteCmd(OLED_CMD_NORMAL_DISPLAY);

  OLED_WriteCmd_2(OLED_CMD_SET_CHARGE_PUMP, OLED_CMD_CHARGE_PUMP_ENABLE);

  OLED_WriteCmd(OLED_CMD_DISPLAY_ON);
  OLED_ClearScreen();
#ifdef OLED_Use_BufferedI2C
  OLED_WriteFlush();
#endif
}
void OLED_ClearScreen() {
  OLED_WriteCmd_2(OLED_CMD_SET_MEM_ADDR_MODE, OLED_CMD_MEM_ADDR_MODE_HADDR);
  OLED_SetPageAddress(0, 7);
  OLED_SetColumnAddress(0, OLED_SCREEN_Width - 1);
  OLED_WriteData_cl(0, 128 * 8);
  OLED_WriteCmd_2(OLED_CMD_SET_MEM_ADDR_MODE, OLED_CMD_MEM_ADDR_MODE_PADDR);
#ifdef OLED_Use_BufferedI2C
  OLED_WriteFlush();
#endif
}

#ifdef OLED_Use_SoftwareI2C
#define OLED_Cmd_SendAndCheck(c) OLED_I2C_SendByte((c));
void OLED_WriteCmd(uint8_t cmd) {
  OLED_I2C_Begin();
  OLED_Cmd_SendAndCheck(OLED_ADDR_W);
  OLED_Cmd_SendAndCheck(OLED_CTRLBYTE_COMMANDS);
  OLED_Cmd_SendAndCheck(cmd);
  OLED_I2C_End();
  return;
}
void OLED_WriteCmd_2(uint8_t cmd, uint8_t data) {
  OLED_I2C_Begin();
  OLED_Cmd_SendAndCheck(OLED_ADDR_W);
  OLED_Cmd_SendAndCheck(OLED_CTRLBYTE_COMMANDS);
  OLED_Cmd_SendAndCheck(cmd);
  OLED_Cmd_SendAndCheck(data);
  OLED_I2C_End();
  return;
}
void OLED_WriteCmd_N(uint8_t cmd, const uint8_t *pdata, uint8_t length) {
  OLED_I2C_Begin();
  OLED_Cmd_SendAndCheck(OLED_ADDR_W);
  OLED_Cmd_SendAndCheck(OLED_CTRLBYTE_COMMANDS);
  OLED_Cmd_SendAndCheck(cmd);
  for (; length; --length, ++pdata)
    OLED_Cmd_SendAndCheck(*pdata);
  OLED_I2C_End();
}
void OLED_WriteData(const uint8_t *data, uint32_t length) {
  OLED_I2C_Begin();
  OLED_Cmd_SendAndCheck(OLED_ADDR_W);
  OLED_Cmd_SendAndCheck(OLED_CTRLBYTE_DATA);
  for (; length; --length, ++data)
    OLED_Cmd_SendAndCheck(*data);
  OLED_I2C_End();
}
void OLED_WriteData_fn(uint8_t (*fn)(void *, uint32_t), uint32_t length,
                       void *this) {
  OLED_I2C_Begin();
  OLED_Cmd_SendAndCheck(OLED_ADDR_W);
  OLED_Cmd_SendAndCheck(OLED_CTRLBYTE_DATA);
  for (uint32_t i = 0; i < length; ++i)
    OLED_Cmd_SendAndCheck(fn(this, i));
  OLED_I2C_End();
}
void OLED_WriteData_cl(uint8_t clear_value, uint32_t length) {
  OLED_I2C_Begin();
  OLED_Cmd_SendAndCheck(OLED_ADDR_W);
  OLED_Cmd_SendAndCheck(OLED_CTRLBYTE_DATA);
  for (uint32_t i = 0; i < length; ++i)
    OLED_Cmd_SendAndCheck(clear_value);
  OLED_I2C_End();
}
#undef OLED_Cmd_SendAndCheck
#endif //! OLED_Use_SoftwareI2C

#ifdef OLED_Use_I2C_DMA
#include "I2C_DMA.h"

enum {
  OLED_TransferBuf_COMMAND = 0x80,
  OLED_TransferBuf_DATA = 0x40,
  OLED_TransferBuf_LEN_MASK = 0x3f,
};
typedef uint8_t OLED_TransferBuf[64];
uint8_t OLED_transfer_buflen = 0;
OLED_TransferBuf OLED_transfer_buffer[2] = {0};
OLED_TransferBuf *OLED_transferbuf_current = OLED_transfer_buffer + 0;
OLED_TransferBuf *OLED_transferbuf_last = OLED_transfer_buffer + 1;

void OLED_WriteFlush(void) {
  if ((OLED_transfer_buflen & OLED_TransferBuf_LEN_MASK) == 0)
    return;
  I2CD_WriteDMA(OLED_DeviceAddr, &(*OLED_transferbuf_current)[0],
                OLED_transfer_buflen & OLED_TransferBuf_LEN_MASK);
  OLED_transfer_buflen = 0x00;

  OLED_TransferBuf *temp = OLED_transferbuf_current;
  OLED_transferbuf_current = OLED_transferbuf_last;
  OLED_transferbuf_last = temp;
}
void OLED_WriteCmd(uint8_t cmd) {
  uint8_t length = OLED_transfer_buflen & OLED_TransferBuf_LEN_MASK;
  if (!(OLED_transfer_buflen & OLED_TransferBuf_COMMAND) || length == 0 ||
      length + 1 > 64) {
    OLED_WriteFlush();
    length = 0;
    (*OLED_transferbuf_current)[length++] = OLED_CTRLBYTE_COMMANDS;
  }
  (*OLED_transferbuf_current)[length++] = cmd;
  OLED_transfer_buflen = OLED_TransferBuf_COMMAND | length;
}
void OLED_WriteCmd_2(uint8_t cmd, uint8_t data) {
  uint8_t length = OLED_transfer_buflen & OLED_TransferBuf_LEN_MASK;
  if (!(OLED_transfer_buflen & OLED_TransferBuf_COMMAND) || length == 0 ||
      length + 2 > 64) {
    OLED_WriteFlush();
    length = 0;
    (*OLED_transferbuf_current)[length++] = OLED_CTRLBYTE_COMMANDS;
  }
  (*OLED_transferbuf_current)[length++] = cmd;
  (*OLED_transferbuf_current)[length++] = data;
  OLED_transfer_buflen = OLED_TransferBuf_COMMAND | length;
}
void OLED_WriteCmd_N(uint8_t cmd, const uint8_t *pdata, uint8_t len) {
  uint8_t length = OLED_transfer_buflen & OLED_TransferBuf_LEN_MASK;
  if (len < 64) {
    if (!(OLED_transfer_buflen & OLED_TransferBuf_COMMAND) || length == 0 ||
        length + len + 1 > 64) {
      OLED_WriteFlush();
      length = 0;
      (*OLED_transferbuf_current)[length++] = OLED_CTRLBYTE_COMMANDS;
    }
    (*OLED_transferbuf_current)[length++] = cmd;
    for (uint32_t l = 0; l < len; ++l)
      (*OLED_transferbuf_current)[length++] = pdata[l];
    OLED_transfer_buflen = OLED_TransferBuf_COMMAND | length;
  } else {
    OLED_WriteFlush();
    I2CD_SetMode(I2CD_MODE_Begin);
    uint8_t buffer[] = {OLED_CTRLBYTE_COMMANDS, cmd};
    I2CD_Write(OLED_DeviceAddr, buffer, sizeof(buffer) / sizeof(uint8_t));
    I2CD_SetMode(I2CD_MODE_End);
    I2CD_Write(OLED_DeviceAddr, pdata, length);
    I2CD_SetMode(I2CD_MODE_Normal);
  }
}
void OLED_WriteData(const uint8_t *pdata, uint32_t len) {
  uint8_t length = OLED_transfer_buflen & OLED_TransferBuf_LEN_MASK;
  if (len < 64) {
    if (!(OLED_transfer_buflen & OLED_TransferBuf_DATA) || length == 0 ||
        length + len > 64) {
      OLED_WriteFlush();
      length = 0;
      (*OLED_transferbuf_current)[length++] = OLED_CTRLBYTE_DATA;
    }
    for (uint32_t l = 0; l < len; ++l)
      (*OLED_transferbuf_current)[length++] = pdata[l];
    OLED_transfer_buflen = OLED_TransferBuf_DATA | length;
  } else {
    OLED_WriteFlush();
    I2CD_SetMode(I2CD_MODE_Begin);
    uint8_t buffer[] = {OLED_CTRLBYTE_DATA};
    I2CD_Write(OLED_DeviceAddr, buffer, sizeof(buffer) / sizeof(uint8_t));
    I2CD_SetMode(I2CD_MODE_End);
    I2CD_Write(OLED_DeviceAddr, pdata, len);
    I2CD_SetMode(I2CD_MODE_Normal);
  }
}
void OLED_WriteData_fn(uint8_t (*fn)(void *, uint32_t), uint32_t length,
                       void *this) {
  uint8_t buffer[length];
  for (uint32_t i = 0; i < length; ++i)
    buffer[i] = fn(this, i);
  OLED_WriteData(buffer, length);
}
void OLED_WriteData_cl(uint8_t clear_value, uint32_t length) {
  uint8_t buffer[length];
  for (uint32_t i = 0; i < length; ++i)
    buffer[i] = clear_value;
  OLED_WriteData(buffer, length);
}
#endif // OLED_Use_I2C_DMA

void OLED_PAMSetColumnStartAddress(uint8_t addr) {
  OLED_WriteCmd_2(addr & 0x0F, (addr >> 4) | 0x10);
}
void OLED_SetColumnAddress(uint8_t start, uint8_t end) {
  uint8_t send[2] = {start, end};
  OLED_WriteCmd_N(OLED_CMD_SET_COL_ADDR, send, 2);
}
void OLED_SetPageAddress(uint8_t start, uint8_t end) {
  uint8_t send[2] = {start, end};
  OLED_WriteCmd_N(OLED_CMD_SET_PAGE_ADDR, send, 2);
}
void OLED_SetDisplayClock(uint8_t clkdiv, uint8_t freq_osc) {
  OLED_WriteCmd_2(OLED_CMD_SET_DISPLAY_CLKDIV, clkdiv | (freq_osc << 4));
}
void OLED_SetPrechargePeriod(uint8_t phase1, uint8_t phase2) {
  OLED_WriteCmd_2(OLED_CMD_SET_PRECHARGE_PERIOD, phase1 | (phase2 << 4));
}

#ifdef OLED_Use_SoftwareI2C
#include "zf_common_headfile.h"
/*引脚配置*/
void OLED_W_SCL(int x) {
  if (x)
    gpio_set_level(A5, GPIO_HIGH);
  else
    gpio_set_level(A5, GPIO_LOW);
}
void OLED_W_SDA(int x) {
  if (x)
    gpio_set_level(A7, GPIO_HIGH);
  else
    gpio_set_level(A7, GPIO_LOW);
}
/*引脚初始化*/
void OLED_I2C_Init(void) {
  gpio_init(A5, GPO, GPIO_HIGH, GPO_OPEN_DTAIN);
  gpio_init(A7, GPO, GPIO_HIGH, GPO_OPEN_DTAIN);
}

/**
 *   * @brief  I2C开始
 *     * @param  无
 *       * @retval 无
 *         */
void OLED_I2C_Begin(void) {
  OLED_W_SDA(1);
  OLED_W_SCL(1);
  OLED_W_SDA(0);
  OLED_W_SCL(0);
}

/**
 *   * @brief  I2C停止
 *     * @param  无
 *       * @retval 无
 *         */
void OLED_I2C_End(void) {
  OLED_W_SDA(0);
  OLED_W_SCL(1);
  OLED_W_SDA(1);
}

/**
 *   * @brief  I2C发送一个字节
 *     * @param  Byte 要发送的一个字节
 *       * @retval 无
 *         */
void OLED_I2C_SendByte(uint8_t Byte) {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    OLED_W_SDA(!!(Byte & (0x80 >> i)));
    OLED_W_SCL(1);
    OLED_W_SCL(0);
  }
  OLED_W_SCL(1); // 额外的一个时钟，不处理应答信号
  OLED_W_SCL(0);
}
#endif
