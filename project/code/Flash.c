#include "zf_common_headfile.h"
#include "Flash.h"


void Flash_Read(int groupnum)
{
		flash_buffer_clear(); 
		uint8_t sector_num = 127 - groupnum / 3;
		uint8_t page_num = 3 - groupnum % 3;
		flash_read_page_to_buffer(sector_num, page_num);
}

void Flash_Write(int groupnum)
{
		uint8_t sector_num = 127 - groupnum / 3;
		uint8_t page_num = 3 - groupnum % 3;
		if(flash_check(sector_num, page_num))                      // 判断是否有数据
		{
			flash_erase_page(sector_num, page_num);                // 擦除这一页
		}
		flash_write_page_from_buffer(sector_num, page_num);
		flash_buffer_clear(); 
}
