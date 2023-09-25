#include "bk_rtos_pub.h"
#include "BkDriverPwm.h"
#include "pwm_pub.h"
#include "error.h"
#include <stdint.h>
#include <stdlib.h>
#include <finsh.h>

#define		CONFIG_PWM_TEST
#ifdef 		CONFIG_PWM_TEST

/*
param:	duty & end-- unit is 1/26MHz = 38ns
   |<---------end-------->|
   |<-------- duty------->|            

    ---------			  |
   |    	 |            |
   |    	 |            |
   |         |            |
	    	  ------------
*/

static void pwm_test(int argc,char *argv[])
{
	UINT32 channel,duty_cycle,cycle;
	
	if(argc != 4)
		return;
		
	channel 	= atoi(argv[1]);
	duty_cycle 	= atoi(argv[2]);
	cycle 		= atoi(argv[3]);

	if(cycle < duty_cycle)
	{
		rt_kprintf("pwm param error: end < duty\r\n");
		return;	
	}
	
	rt_kprintf("---pwm %d test--- \r\n",channel);	

	bk_pwm_initialize(channel, cycle, duty_cycle);		/*pwm 模块初始化，设置对应通道的占空比*/
	bk_pwm_start(channel);								/*启动pwm */
	
	rt_thread_delay(100);	

	bk_pwm_stop(channel);								/*关闭pwm */
	rt_kprintf("---pwm test stop---\r\n");
}
	
MSH_CMD_EXPORT(pwm_test,pwm test);
#endif