#include "zf_common_headfile.h"
#include <string.h>
#include "OLED.h"
#include "Key.h"
#include "Menu.h"

int sta=0; //0正常模式 1编辑模式
char state[2][15]={"normol","change"};
uint8_t Store[6]={0};
uint8_t KeyNum;

int count_digits(int num) {
    int count = 0;
	if (num == 0) {
        return 2;
    }
      if (num < 0) {
        num = -num;
    }
    while (num > 0) {
        count++;
        num /= 10;
    }
    if (count==1) count++;
    return count;
}

typedef struct  
{
    char name[15];
    int data;
    int bo; // 判断是否可读可写 0都不可 1可读 2可写
} item;

struct system   //系统菜单定义
{
    item Main[2];
    item Mode[1];
    item PID[3];
};



typedef struct
{
    item *ptr;  
    int size;
    char name[15];
    int pre;
    int next[4];
} menu_info;

struct system sys;
menu_info info[] = {
        {sys.Main,2,"Main Menu",-1,{1,2}},
        {sys.Mode,1,"Mode",0,{-1}},
        {sys.PID,3,"PID",0,{-1,-1,-1}}
}; //{头地址，子菜单数，名字，pre地址，next地址数组}

item *p;
menu_info *pi;
uint8_t piju;

void initSystem(struct system *sys) {
    strcpy(sys->Main[0].name, "Mode");
    strcpy(sys->Main[1].name, "PID");

    for (int i=0;i<2;i++)
    {
        sys->Main[i].data=-1;
        sys->Main[i].bo=0;
    }
    
    strcpy(sys->Mode[0].name, "Mode");
    
    for (int i=0;i<1;i++)
    {
        sys->Mode[i].data=0;
        sys->Mode[i].bo=2;
    }
    
    
    strcpy(sys->PID[0].name, "POT");
    strcpy(sys->PID[1].name, "NTC");
	  strcpy(sys->PID[2].name, "LDR");
    for (int i=0;i<3;i++)
    {
        sys->PID[i].data=0;
        sys->PID[i].bo=0;
    }
	
}


void print(void)
{
	
	if (strcmp(pi->name,"Main Menu")!=0) 
    {
    OLED_ShowString(1,1,pi->name);
    if (sta==1) OLED_ShowChar(1,15,'E');
    }
		else {
			OLED_ShowString(1,1,"   == Main ==");
		}

	OLED_ShowChar((p-(pi->ptr))+2,1,'>'); 
	
	for (int i=0;i<pi->size;i++)
	{
            OLED_ShowString(i+2,2,pi->ptr[i].name);
			
			if (pi->ptr[i].bo)
			{
				OLED_ShowNum(i+2,12,pi->ptr[i].data+1,1);
			}
		}
    return;
}

void Menu_Init(void)
{
	
		OLED_Init();
		Key_Init();
	
    initSystem(&sys);

    pi=info; //所在菜单指针
	
    p=info[0].ptr; // 选中子菜单指针
	
    print();

}

void Menu_Loop(void)
{

    Key_Tick();
	
	piju = pi - info;

        if (Key_Check(KEY_SINGLE,0))
		{	
			if (sta==0)
			{
				OLED_ShowChar((p-(pi->ptr))+2,1,' '); 
                p++;
				if (p-pi->ptr>=pi->size) p-=pi->size;
				OLED_ShowChar((p-(pi->ptr))+2,1,'>'); 
			}
			else if (sta==1)
			{
				p->data++;
				if (strcmp(p->name,"Mode")==0) p->data=p->data%5;
				print();
			}
		}
        
        else if (Key_Check(KEY_SINGLE,1))
		{
			if (sta==0)
			{
				OLED_ShowChar((p-(pi->ptr))+2,1,' '); 
                p--;
				if (p-pi->ptr<0) p+=pi->size;
				OLED_ShowChar((p-(pi->ptr))+2,1,'>'); 
			}
			else if (sta==1)
			{
				if (strcmp(p->name,"Mode")==0 && p->data == 0) p->data=p->data+5;
				p->data--;
				print();
			}
			
		}
        
        else if (Key_Check(KEY_SINGLE,2))
        {
            if (pi->next[p-pi->ptr]!=-1) 
            {
								OLED_Clear();
								pi=pi->next[p-pi->ptr]+info;
                p=pi->ptr;
            }
            else if (p->bo==2) 
            {    
                sta=1-sta;
							if (sta == 0) OLED_ShowString(1,15," ");
            }
			
			print();

        }
		
        else if (Key_Check(KEY_SINGLE,3))
        {
            if (pi->pre!=-1)
            {
							OLED_Clear();
               pi=&info[pi->pre];
               p=pi->ptr;
							 sta=0;
            }	
			print();
        }
}

	
