#include "include.h"
#include "arm_arch.h"
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "typedef.h"
#include "icu_pub.h"
#include "i2s.h"
#include "i2s_pub.h"

#include "sys_ctrl_pub.h"

#include "drv_model_pub.h"
#include "mem_pub.h"

#include "sys_config.h"
#include "error.h"
#include "bk_rtos_pub.h"

#define I2S_DATA_LEN       0x100

#ifdef I2S_TEST

extern UINT32 i2s_configure(UINT32 fifo_level, UINT32 sample_rate, UINT32 bits_per_sample, UINT32 mode);

volatile i2s_trans_t i2s_trans;
i2s_level_t  i2s_fifo_level;

int i2s_test(int argc, char** argv)
{
	struct rt_device *i2s_device;
	struct i2s_message msg;	
	uint32 i,rate,bit_length ;
	uint32 i2s_mode = 0;
	if(argc != 4)
	{
		rt_kprintf("---cmd error--\r\n");
		return  RT_ERROR;
	}
	rate		= atoi(argv[2]);
	bit_length	= atoi(argv[3]);
	

	msg.recv_len = I2S_DATA_LEN;
	msg.send_len = I2S_DATA_LEN;

	msg.recv_buf = rt_malloc(I2S_DATA_LEN * sizeof(msg.recv_buf[0]));
	if(msg.recv_buf == RT_NULL)
	{
		rt_kprintf("msg.recv_buf malloc failed\r\n");
	}
	//rt_kprintf("msg.recv_buf=%x\r\n",msg.recv_buf);

	msg.send_buf = rt_malloc(I2S_DATA_LEN * sizeof(msg.send_buf[0]));
	if(msg.send_buf == RT_NULL)
	{
		rt_kprintf("msg.send_buf malloc failed\r\n");
	}
	//rt_kprintf("msg.send_buf=%x\r\n",msg.send_buf);

	/* find device*/
	i2s_device = rt_device_find("i2s");
	if(i2s_device == RT_NULL)
	{
		rt_kprintf("---i2s device find failed---\r\n ");
		return 0 ;
	}

	/* init device*/
	i2s_init(1);

	/* open audio , set fifo level set sample rate/datawidth */
	i2s_mode = i2s_mode| I2S_MODE| I2S_LRCK_NO_TURN| I2S_SCK_NO_TURN| I2S_MSB_FIRST| (0<<I2S_SYNC_LENGTH_BIT)| (0<<I2S_PCM_DATA_LENGTH_BIT);
	
	/* write and recieve */
	if(strcmp(argv[1], "master") == 0)								
	{
		rt_kprintf("---i2s_master_test_start---\r\n");
		
		if(msg.send_buf == NULL)

		{
			rt_kprintf("---msg.send_buf error --\r\n ");
			return 0;
		}
		
		for(i=0; i<I2S_DATA_LEN; i++)
		{
			msg.send_buf[i]= ((i+1)<<24) | ((i+1)<<16)  | ((i+1)<<8) | ((i+1)<<0);
		}
		
		i2s_configure(FIFO_LEVEL_32, rate, bit_length, i2s_mode);
		i2s_transfer(msg.send_buf, msg.recv_buf, I2S_DATA_LEN, MASTER);
	
		for(i=0; i<I2S_DATA_LEN; i++)		
		{
			rt_kprintf("msg.send_buf[%d]=0x%x ---msg.recv_buf[%d]=0x%x \r\n", i, msg.send_buf[i],i, msg.recv_buf[i]);
		}
		
		rt_kprintf("---i2s_master_test_over---\r\n");

	}
	else if(strcmp(argv[1], "slave") == 0) 							//slave
	{		
		rt_kprintf("---i2s_slave_test_start---\r\n");
		
		if(msg.send_buf == NULL)
		{
			rt_kprintf("---msg.send_buf error --\r\n ");
			return 0;
		}
	
		for(i=0; i<I2S_DATA_LEN; i++)
		{
			msg.send_buf[i]= ((i+1)<<24) | ((i+1)<<16)  | ((i+1)<<8) | ((i+1)<<0) |0x80808080;
		}
		
		i2s_configure(FIFO_LEVEL_32, rate, bit_length, i2s_mode);
		i2s_transfer(msg.send_buf, msg.recv_buf, I2S_DATA_LEN, SLAVE);
	
		for(i=0; i<I2S_DATA_LEN; i++)
		{
			rt_kprintf("msg.send_buf[%d]=0x%x , msg.recv_buf[%d]=0x%x \r\n", i, msg.send_buf[i],i, msg.recv_buf[i]);
		}
		
		rt_kprintf("---i2s_slave_test_over---\r\n");
	}
	else
	{
		rt_kprintf("---no test command--\r\n");
	}


	i2s_trans.p_rx_buf = RT_NULL;
	i2s_trans.p_tx_buf = RT_NULL;
	
	if(msg.send_buf != RT_NULL)
	{
		os_free(msg.send_buf);
		msg.send_buf= RT_NULL;
	}

	if(msg.recv_buf != RT_NULL)
	{
		os_free(msg.recv_buf);
		msg.recv_buf= RT_NULL;
	}

	rt_kprintf("---i2s_test_over---\r\n");
	return 0;
}

MSH_CMD_EXPORT(i2s_test, i2s_test);
#endif
