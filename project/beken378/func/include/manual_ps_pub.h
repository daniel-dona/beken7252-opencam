#ifndef _MANUAL_PS_PUB_H_
#define _MANUAL_PS_PUB_H_

#define BK_DEEP_SLEEP_DEBUG
#ifdef  BK_DEEP_SLEEP_DEBUG
#define BK_DEEP_SLEEP_PRT  rt_kprintf
#endif

typedef enum { 
	PS_DEEP_WAKEUP_GPIO = 0,
	PS_DEEP_WAKEUP_RTC = 1, 
	PS_DEEP_WAKEUP_GPIO_RTC = 2,
} PS_DEEP_WAKEUP_WAY;

typedef struct  ps_deep_ctrl{

	/*deep_sleep 模式下唤醒方式*/
    PS_DEEP_WAKEUP_WAY wake_up_way;
	
	/** @brief	Request deep sleep,and wakeup by gpio.
	 *
	 *	@param	gpio_index_map:The gpio bitmap which set 1 enable wakeup deep sleep.
	 *				gpio_index_map is hex and every bits is map to gpio0-gpio31.
	 *				ps:gpio1 as uart RX pin must be wake up from falling
	 *			gpio_edge_map:The gpio edge bitmap for wakeup gpios,
	 *				gpio_edge_map is hex and every bits is map to gpio0-gpio31.
	 *				0:rising,1:falling.
				gpio_last_index_map:The gpio bitmap which set 1 enable wakeup deep sleep.
	  * 			 gpio_index_map is hex and every bits is map to gpio32-gpio39.
	  * 		gpio_last_edge_map:The gpio edge bitmap for wakeup gpios,
	  * 			 gpio_edge_map is hex and every bits is map to gpio32-gpio39.
	  * 			 0:rising,1:falling.
	 */
	
	UINT32 gpio_index_map;
	UINT32 gpio_edge_map;
	UINT32 gpio_last_index_map;
	UINT32 gpio_last_edge_map;
	
	UINT32 sleep_time;
}PS_DEEP_CTRL_PARAM;

#define     PS_SUPPORT_MANUAL_SLEEP     0
typedef void (*ps_wakeup_cb)(void);
extern void deep_sleep_wakeup_with_gpio(UINT32 gpio_index_map,UINT32 gpio_edge_map);
extern void bk_enter_deep_sleep_mode(PS_DEEP_CTRL_PARAM *deep_param);

#endif

