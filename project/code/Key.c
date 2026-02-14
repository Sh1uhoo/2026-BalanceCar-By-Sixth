#include "zf_common_headfile.h"
#include "Key.h"

uint8_t Key_Flag[3];

void Key_Init(void)
{
	gpio_init(E2, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY1 输入 默认高电平 上拉输入
    gpio_init(E3, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY2 输入 默认高电平 上拉输入
    gpio_init(E4, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY3 输入 默认高电平 上拉输入
    gpio_init(E5, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY4 输入 默认高电平 上拉输入
}


uint8_t Key_GetState(int n)
{
	if (n == 0)
	{
		
		if (!gpio_get_level(E2))
			return 1;
		return 0;
	}
	else if (n == 1) 
	{
		
		if (!gpio_get_level(E3))
			return 1;
		return 0;
	}
	else if (n == 2) 
	{
		
		if (!gpio_get_level(E4))
			return 1;
		return 0;
	}
	else if (n == 3) 
	{
		
		if (!gpio_get_level(E5))
			return 1;
		return 0;
	}
	return 0;
}


uint8_t Key_Check(uint8_t Flag,uint8_t n)
{
	
	if (Key_Flag[n] & Flag)
	{
		if (Flag != KEY_HOLD)
		{
			Key_Flag[n] &= ~Flag;
		}
		return 1;
	}
	return 0;
}

void Key_Tick(void)
{
	static uint8_t CurrState[4], PrevState[4];
	static uint8_t S[4]={0};
	static uint16_t Time[4];
	
	for (int i=0;i<4;i++)
	{
		if (Time[i] > 0)
		Time[i]--;

		 PrevState[i] = CurrState[i];
		 CurrState[i] = Key_GetState(i);

		 if (CurrState[i] == 1)
		 {
			Key_Flag[i] |= 0x01;
		 }
		 else
		 {
			Key_Flag[i] &= ~0x01;
		 }

		 if (CurrState[i] == 1 && PrevState[i] == 0)
		 {
			Key_Flag[i] |= 0x02;
		 }
		
		 if (CurrState[i] == 0 && PrevState[i] == 1)
		 {
			Key_Flag[i] |= 0x04;
		 }

		 if (S[i] == 0)
		 {
			if (CurrState[i] == 1)
			{
				Time[i] = 1000 / 10;
				S[i] = 1;
			}
		 }
		 else if (S[i] == 1)
		 {
			if (CurrState[i] == 0)
			{
				Time[i] = 150 / 10;
				S[i] = 2;
			}
			else if (Time[i] == 0)
			{
				Key_Flag[i] |= KEY_LONG;
				S[i] = 4;
			}
		 }
		 else if (S[i] == 2)
		 {
			if (CurrState[i] == 1)
			{
				Key_Flag[i] |= KEY_DOUBLE;
				S[i] = 3;
			}
			else if (Time[i] == 0)
			{
				Key_Flag[i] |= KEY_SINGLE;
				S[i] = 0;
			}
		 }
		 else if (S[i] == 3)
		 {
			if (CurrState[i] == 0)
			{
				S[i] = 0;
			}
		 }
		 else if (S[i] == 4)
		 {
			if (CurrState[i] == 0)
			{
				S[i] = 0;
			}
		 }
	 }
}