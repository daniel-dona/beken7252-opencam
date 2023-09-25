#define		EASY_FLASH_TEST
#ifdef 		EASY_FLASH_TEST
#include "rtthread.h"
#include <dfs.h>
#include <dfs_fs.h>
#include "player.h"
#include "include.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "app.h"
#include "ate_app.h"
#include "shell.h"
#include "flash.h" 	
#include <finsh.h>
#include "easyflash.h"

unsigned char read_buff[10*1024];
unsigned char write_buff[10*1024];

#define  test_data "AABBCCDDEEFFGGHHIIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ"     
#define  KEY "temp"

static void Easy_Flash_Write(void)
{
	EfErrCode result = EF_NO_ERR;
	
	easyflash_init();					 			/*初始化 */		
	
    result = ef_set_env(KEY,test_data);				/*将要写入的数据存放到 easy flash  环境变量 */
	if(result != EF_NO_ERR)
	{
		rt_kprintf("easy_flash set error");
		return;
	}
	
    result = ef_save_env();							/*保存数据 */
	if(result != EF_NO_ERR)
	{
		rt_kprintf("easy_flash save error");
		return;
	}
	
    rt_kprintf("---Flash Write over \r\n");	
}

static void Easy_Flash_Read(void)  /*读取easy flash 写入的数据*/
{
    char *p_write_buff;
	
	easyflash_init();
	
	p_write_buff = ef_get_env(KEY);			/*获取easy flash存入的数据*/	
	if( p_write_buff )
	{
		rt_kprintf("%s",p_write_buff);
	}
	else
	{
		rt_kprintf("easy_flash get error\r\n");	
	}
}

	
MSH_CMD_EXPORT(Easy_Flash_Write,set_or_read_Easy_Flash_Write test);
MSH_CMD_EXPORT(Easy_Flash_Read, set_or_read_Easy_Flash_Read       test);

#endif
