#include "error.h"
#include "include.h"

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <stdint.h>
#include <stdlib.h>
#include <finsh.h>
#include <rtdef.h>

#include "manual_ps_pub.h"
#include "bk_rtos_pub.h"

#include "sys_ctrl_pub.h"
#include "sys_ctrl.h"
#include "target_util_pub.h"

#include "drv_model_pub.h"

#include "uart_pub.h"
#include "flash_pub.h"
#include "power_save_pub.h"
#include "intc_pub.h"
#include "icu_pub.h"
#include "gpio_pub.h"
#include "reg_rc.h"
#include "co_math.h"
#include "rw_pub.h"
#include "manual_ps_pub.h"
#include "mcu_ps_pub.h"
#include "ps_debug_pub.h"

#ifdef TEST_DEEP_SLEEP
static void enter_deep_sleep_test(int argc,char *argv[])
{
	bk_rtos_delay_milliseconds(10);
	PS_DEEP_CTRL_PARAM deep_sleep_param;

	deep_sleep_param.wake_up_way			= 0;
	
	deep_sleep_param.gpio_index_map      	= atoi(argv[1]);
	deep_sleep_param.gpio_edge_map       	= atoi(argv[2]);	
	deep_sleep_param.gpio_last_index_map 	= atoi(argv[3]);
	deep_sleep_param.gpio_last_edge_map  	= atoi(argv[4]);
	deep_sleep_param.sleep_time     		= atoi(argv[5]);
	deep_sleep_param.wake_up_way     		= atoi(argv[6]);

	if(argc == 7)
	{		
		rt_kprintf("---deep sleep test param : 0x%0X 0x%0X 0x%0X 0x%0X %d %d\r\n", 
					deep_sleep_param.gpio_index_map, 
					deep_sleep_param.gpio_edge_map,
					deep_sleep_param.gpio_last_index_map, 
					deep_sleep_param.gpio_last_edge_map,
					deep_sleep_param.sleep_time,
					deep_sleep_param.wake_up_way);

		UINT32 reg;
		reg = (reg & (~(BLOCK_EN_WORD_MASK << 20)) & (~(0x7FFFUL << 5)) & (~(0x01UL << 1)));
		reg = (reg | (BLOCK_EN_WORD_PWD << 20) | BLK_EN_FLASH | BLK_EN_ROSC32K | BLK_EN_26M_XTAL | BLK_EN_32K_XTAL);
		REG_WRITE(SCTRL_BLOCK_EN_CFG, reg); //sys_ctrl : 0x4B;                   //'E'

		REG_WRITE(SCTRL_BLOCK_EN_CFG, (0x15D | (0xA5C << 20)));
		rt_kprintf("VDDRAM set to close\n");
		bk_enter_deep_sleep_mode(&deep_sleep_param);
	}
	else
	{
		rt_kprintf("---argc error!!! \r\n");
	}
}

FINSH_FUNCTION_EXPORT_ALIAS(enter_deep_sleep_test, __cmd_sleep_mode, test sleep mode);

void pwr_enter_sleepmode(void)
{
	
	bk_rtos_delay_milliseconds(10);
	PS_DEEP_CTRL_PARAM deep_sleep_param;

	deep_sleep_param.wake_up_way = 0;

	deep_sleep_param.gpio_index_map = 128;
	deep_sleep_param.gpio_edge_map = 0;
	deep_sleep_param.gpio_last_index_map = 0;
	deep_sleep_param.gpio_last_edge_map = 0;
	deep_sleep_param.sleep_time = 0;
	deep_sleep_param.wake_up_way = 0;

	rt_kprintf("---deep sleep test param : 0x%0X 0x%0X 0x%0X 0x%0X %d %d\r\n",
			   deep_sleep_param.gpio_index_map,
			   deep_sleep_param.gpio_edge_map,
			   deep_sleep_param.gpio_last_index_map,
			   deep_sleep_param.gpio_last_edge_map,
			   deep_sleep_param.sleep_time,
			   deep_sleep_param.wake_up_way);

	UINT32 reg;
	reg = (reg & (~(BLOCK_EN_WORD_MASK << 20)) & (~(0x7FFFUL << 5)) & (~(0x01UL << 1)));
	reg = (reg | (BLOCK_EN_WORD_PWD << 20) | BLK_EN_FLASH | BLK_EN_ROSC32K | BLK_EN_26M_XTAL | BLK_EN_32K_XTAL);
	REG_WRITE(SCTRL_BLOCK_EN_CFG, reg); //sys_ctrl : 0x4B;                   //'E'

	REG_WRITE(SCTRL_BLOCK_EN_CFG, (0x15D | (0xA5C << 20)));
	rt_kprintf("VDDRAM set to close\n");
	bk_enter_deep_sleep_mode(&deep_sleep_param);
}
#endif
