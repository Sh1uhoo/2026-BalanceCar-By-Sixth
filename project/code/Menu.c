#include "zf_common_headfile.h"
#include <string.h>
#include "OLED.h"
#include "Key.h"
#include "Menu.h"
#include "Flash.h"

int sta=0; //0正常模式 1编辑模式
char state[2][15]={"normol","change"};
uint8_t Store[6]={0};
uint8_t KeyNum;

void flash_data_buffer_printf (void);

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
    item PID[4];
    item Bal[3];
    item Vel[3];    
    item Dir[3];
    item Pos[3];
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
        {sys.PID,4,"PID",0,{3,4,5,6}},
        {sys.Bal,3,"Bal PID",2,{-1,-1,-1}},
        {sys.Vel,3,"Vel PID",2,{-1,-1,-1}},
        {sys.Dir,3,"Dir PID",2,{-1,-1,-1}},
        {sys.Pos,3,"Pos PID",2,{-1,-1,-1}}
}; //{头地址，子菜单数，名字，pre地址，next地址数组}

item *p;
menu_info *pi;
uint8_t piju;
int e=1;

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
    
    
    strcpy(sys->PID[0].name, "Bal");
    strcpy(sys->PID[1].name, "Vel");
	strcpy(sys->PID[2].name, "Dir");
    strcpy(sys->PID[3].name, "Pos");
    for (int i=0;i<4;i++)
    {
        sys->PID[i].data=0;
        sys->PID[i].bo=0;
    }
	
    strcpy(sys->Bal[0].name, "P");
    strcpy(sys->Bal[1].name, "I");  
    strcpy(sys->Bal[2].name, "D");
	Flash_Read(0);
    for (int i=0;i<3;i++)
    {
        sys->Bal[i].data=(int)(flash_union_buffer[i].int16_type);
        sys->Bal[i].bo=2;
    }

    strcpy(sys->Vel[0].name, "P");
    strcpy(sys->Vel[1].name, "I");
    strcpy(sys->Vel[2].name, "D");
	Flash_Read(1);
    for (int i=0;i<3;i++)
    {
        sys->Vel[i].data=(int)(flash_union_buffer[i].int16_type);
        sys->Vel[i].bo=2;
    }

    strcpy(sys->Dir[0].name, "P");
    strcpy(sys->Dir[1].name, "I");
    strcpy(sys->Dir[2].name, "D");
	Flash_Read(2);
    for (int i=0;i<3;i++)
    {
        sys->Dir[i].data=(int)(flash_union_buffer[i].int16_type);
        sys->Dir[i].bo=2;
    }

    strcpy(sys->Pos[0].name, "P");
    strcpy(sys->Pos[1].name, "I");
    strcpy(sys->Pos[2].name, "D");
	Flash_Read(3);
    for (int i=0;i<3;i++)
    {
        sys->Pos[i].data=(int)(flash_union_buffer[i].int16_type);
        sys->Pos[i].bo=2;
    }
}


void print(void)
{
	
	if (strcmp(pi->name,"PID")==0) e=0;
	else e=1;
	
	if (strcmp(pi->name,"PID")!=0) 
    {
		 if (strcmp(pi->name,"Main Menu")==0){
		OLED_ShowString(1,1,"   == Main ==");
		}
		 else OLED_ShowString(1,3,pi->name);
	}

	OLED_ShowChar((p-(pi->ptr))+e+1,1,'>'); 
	
	for (int i=0;i<pi->size;i++)
	{
        
            OLED_ShowString(i+e+1,2,pi->ptr[i].name);
			
			if (pi->ptr[i].bo)
			{
				if (strcmp(p->name,"Mode")==0) OLED_ShowNum(i+2,12,pi->ptr[i].data+1,1);
				else  OLED_ShowNum(i+2,12,pi->ptr[i].data+1,3);
				
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
				OLED_ShowChar((p-(pi->ptr))+e+1,1,' '); 
                p++;
				if (p-pi->ptr>=pi->size) p-=pi->size;
				OLED_ShowChar((p-(pi->ptr))+e+1,1,'>'); 
			}
			else if (sta==1)
			{
				p->data++;
				if (strcmp(p->name,"Mode")==0) p->data=p->data%5;
				print();
			}
		}
        
		else if (Key_Check(KEY_LONG,0) && sta == 1 && strcmp(p->name,"Mode")!=0)
		{
			
			p->data+=10;
				
			print();
			
		}
		
        else if (Key_Check(KEY_SINGLE,1))
		{
			if (sta==0)
			{
				OLED_ShowChar((p-(pi->ptr))+e+1,1,' '); 
                p--;
				if (p-pi->ptr<0) p+=pi->size;
				OLED_ShowChar((p-(pi->ptr))+e+1,1,'>'); 
			}
			else if (sta==1)
			{
				if (strcmp(p->name,"Mode")==0 && p->data == 0) p->data=p->data+5;
				p->data--;
				print();
			}
			
		}

        else if (Key_Check(KEY_LONG,1) && sta == 1 && strcmp(p->name,"Mode")!=0) 
        {
            p->data-=10;
            if (p->data<0) p->data=0;
            print();
        }

        
        else if (Key_Check(KEY_SINGLE,2))
        {
            if (pi->next[p-pi->ptr]!=-1) 
            {
				OLED_Clear();
				pi=pi->next[p-pi->ptr]+info;
                p=pi->ptr;
				print();
            }
            else if (p->bo==2) 
            {    
                sta=1-sta;
				if (sta == 0) 
				{
					OLED_ShowString(1,15," ");
					
					if (strcmp(p->name,"Mode")!=0)
					{
						for (int i=0;i<3;i++)
						{
							flash_union_buffer[i].int16_type = (pi->ptr+i)->data;
						}
							
						Flash_Write(pi - info - 3);
					}
				}
				else 
				{
					OLED_ShowString(1,15,"E");
				}
            }

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

	
