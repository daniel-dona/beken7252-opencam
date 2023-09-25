#include "error.h"
#include "include.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include <stdlib.h>
#include <finsh.h>
#include <rtdef.h>
#include "include.h"
#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "arm_arch.h"
#include "qspi_pub.h"
#include "BkDriverQspi.h"

#define QSPI_TEST
#ifdef QSPI_TEST

#define QSPI_TEST_LENGTH			( 0x4 * 16 )

static UINT8 DataOffset;

static void qspi_psram_dcache_test(int argc,char *argv[])
{
	UINT32 i,ret;
	UINT32 SetLineMode;
	qspi_dcache_drv_desc qspi_cfg;
		
	UINT32*   p_WRData1;
	UINT32*   p_WRData2;
	UINT32*   p_WRData3;
	UINT32*   p_WRData4;
	UINT32*   p_WRData5;
	
	UINT32*   p_RDData1;
	UINT32*   p_RDData2;
	UINT32*   p_RDData3;
	UINT32*   p_RDData4;
	UINT32*   p_RDData5;

	p_WRData1 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_WRData1[0]));
	if(p_WRData1 == RT_NULL)
	{
		rt_kprintf("p_WRData1 malloc failed\r\n");
	}
	
	p_WRData2 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_WRData2[0]));
	if(p_WRData2 == RT_NULL)
	{
		rt_kprintf("p_WRData2 malloc failed\r\n");
	}
	
	p_WRData3 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_WRData3[0]));
	if(p_WRData3 == RT_NULL)
	{
		rt_kprintf("p_WRData3 malloc failed\r\n");
	}
	
	p_WRData4 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_WRData4[0]));
	if(p_WRData4 == RT_NULL)
	{
		rt_kprintf("p_WRData4 malloc failed\r\n");
	}

	p_WRData5 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_WRData5[0]));
	if(p_WRData5 == RT_NULL)
	{
		rt_kprintf("p_WRData5 malloc failed\r\n");
	}

	p_RDData1 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_RDData1[0]));
	if(p_RDData1 == RT_NULL)
	{
		rt_kprintf("p_RDData1 malloc failed\r\n");
	}
	
	p_RDData2 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_RDData2[0]));
	if(p_RDData2 == RT_NULL)
	{
		rt_kprintf("p_RDData2 malloc failed\r\n");
	}
	
	p_RDData3 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_RDData3[0]));
	if(p_RDData3 == RT_NULL)
	{
		rt_kprintf("p_RDData3 malloc failed\r\n");
	}

	p_RDData4 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_RDData4[0]));
	if(p_RDData4 == RT_NULL)
	{
		rt_kprintf("p_RDData4 malloc failed\r\n");
	}

	p_RDData5 = rt_malloc(QSPI_TEST_LENGTH * sizeof(p_RDData5[0]));
	if(p_RDData5 == RT_NULL)
	{
		rt_kprintf("p_RDData5 malloc failed\r\n");
	}
	

	for(i=0; i<QSPI_TEST_LENGTH; i++)
	{
		p_WRData1[i]= ((i+1)<<24) | ((i+1)<<16)	| ((i+1)<<8) | ((i+1)<<0) |0x70707070;
		p_WRData2[i]= ((i+1)<<24) | ((i+1)<<16)	| ((i+1)<<8) | ((i+1)<<0) |0x80808080;	
		p_WRData3[i]= ((i+1)<<24) | ((i+1)<<16)	| ((i+1)<<8) | ((i+1)<<0) |0x90909090;
		p_WRData4[i]= ((i+1)<<24) | ((i+1)<<16)	| ((i+1)<<8) | ((i+1)<<0) |0xe0e0e0e0;
		p_WRData5[i]= ((i+1)<<24) | ((i+1)<<16)	| ((i+1)<<8) | ((i+1)<<0) |0xf0f0f0f0;		
	}
	
	if(argc == 2)
	{			
		rt_kprintf("[qspi_test]:test_qspi_dcache_write_read_data\r\n");
		
		SetLineMode = atoi(argv[1]);

		qspi_cfg.mode = SetLineMode;		// 0: 1 line mode    3: 4 line mode
		qspi_cfg.clk_set = 0x10;
		qspi_cfg.wr_command = SetLineMode ? 0x38 : 0x02;		//write
		qspi_cfg.rd_command = SetLineMode ? 0xEB : 0x03;		//read
		qspi_cfg.wr_dummy_size = 0;
		qspi_cfg.rd_dummy_size = SetLineMode ? 0x06 : 0x00;

		bk_qspi_dcache_initialize(&qspi_cfg);
		bk_qspi_start();

		bk_qspi_dcache_write_data(0x00000, p_WRData1, QSPI_TEST_LENGTH);
		bk_qspi_dcache_write_data(0x04000, p_WRData2, QSPI_TEST_LENGTH);
		bk_qspi_dcache_write_data(0x08000, p_WRData3, QSPI_TEST_LENGTH);
		bk_qspi_dcache_write_data(0x0C000, p_WRData4, QSPI_TEST_LENGTH);
		bk_qspi_dcache_write_data(0x10000, p_WRData5, QSPI_TEST_LENGTH);

		rt_thread_delay(rt_tick_from_millisecond(100));	

		bk_qspi_dcache_read_data(0x00000, p_RDData1, QSPI_TEST_LENGTH);
		bk_qspi_dcache_read_data(0x04000, p_RDData2, QSPI_TEST_LENGTH);
		bk_qspi_dcache_read_data(0x08000, p_RDData3, QSPI_TEST_LENGTH);
		bk_qspi_dcache_read_data(0x0C000, p_RDData4, QSPI_TEST_LENGTH);
		bk_qspi_dcache_read_data(0x10000, p_RDData5, QSPI_TEST_LENGTH);
		
		if(memcmp(p_WRData1, p_RDData1, QSPI_TEST_LENGTH*4) == 0)
		{
			rt_kprintf("[qspi_test]:qspi read data 1 pass \r\n ");
		}
		else
		{
			rt_kprintf("[qspi_test]:qspi read data 1 error !!! \r\n ");
			
			for (i=0; i<QSPI_TEST_LENGTH; i++)
			{
				rt_kprintf("p_WRData[%d]=0x%lx, p_RDData[%d]=0x%lx\r\n", i, *(p_WRData1 + i), i, *(p_RDData1 + i));
			}
		}
		
		if(memcmp(p_WRData2, p_RDData2, QSPI_TEST_LENGTH*4) == 0)
		{
			rt_kprintf("[qspi_test]:qspi read data 2 pass \r\n ");
		}
		else
		{
			rt_kprintf("[qspi_test]:qspi read data 2 error !!! \r\n ");
			
			for (i=0; i<QSPI_TEST_LENGTH; i++)
			{
				rt_kprintf("p_WRData[%d]=0x%lx, p_RDData[%d]=0x%lx\r\n", i, *(p_WRData2 + i), i, *(p_RDData2 + i));
			}
		}
		
		if(memcmp(p_WRData3, p_RDData3, QSPI_TEST_LENGTH*4) == 0)
		{
			rt_kprintf("[qspi_test]:qspi read data 3 pass \r\n ");
		}
		else
		{
			rt_kprintf("[qspi_test]:qspi read data 3 error !!! \r\n ");			
			for (i=0; i<QSPI_TEST_LENGTH; i++)
			{
				rt_kprintf("p_WRData[%d]=0x%lx, p_RDData[%d]=0x%lx\r\n", i, *(p_WRData3 + i), i, *(p_RDData3 + i));
			}
		}
		if(memcmp(p_WRData4, p_RDData4, QSPI_TEST_LENGTH*4) == 0)
		{
			rt_kprintf("[qspi_test]:qspi read data 4 pass \r\n ");

		}
		else
		{
			rt_kprintf("[qspi_test]:qspi read data 4 error !!! \r\n ");
			
			for (i=0; i<QSPI_TEST_LENGTH; i++)
			{
				rt_kprintf("p_WRData[%d]=0x%lx, p_RDData[%d]=0x%lx\r\n", i, *(p_WRData4 + i), i, *(p_RDData4 + i));
			}
		}

		if(memcmp(p_WRData5, p_RDData5, QSPI_TEST_LENGTH*4) == 0)
		{
			rt_kprintf("[qspi_test]:qspi read data 5 pass \r\n ");
		}
		else
		{
			rt_kprintf("[qspi_test]:qspi read data 5 error !!! \r\n ");
			
			for (i=0; i<QSPI_TEST_LENGTH; i++)
			{
				rt_kprintf("p_WRData[%d]=0x%lx, p_RDData[%d]=0x%lx\r\n", i, *(p_WRData5 + i), i, *(p_RDData5 + i));
			}
		}

		if(p_WRData1 != RT_NULL)
		{
			rt_free(p_WRData1);
			p_WRData1= RT_NULL;
		}
		if(p_WRData2 != RT_NULL)
		{
			rt_free(p_WRData2);
			p_WRData2= RT_NULL;
		}		
		if(p_WRData3 != RT_NULL)
		{
			rt_free(p_WRData3);
			p_WRData3= RT_NULL;
		}
		if(p_WRData4 != RT_NULL)
		{
			rt_free(p_WRData4);
			p_WRData4= RT_NULL;
		}
		if(p_WRData5 != RT_NULL)
		{
			rt_free(p_WRData5);
			p_WRData5= RT_NULL;
		}

		if(p_RDData1 != RT_NULL)
		{
			rt_free(p_RDData1);
			p_RDData1= RT_NULL;
		}
		if(p_RDData2 != RT_NULL)
		{
			rt_free(p_RDData2);
			p_RDData2= RT_NULL;
		}
		if(p_RDData3 != RT_NULL)
		{
			rt_free(p_RDData3);
			p_RDData3= RT_NULL;
		}
		if(p_RDData4 != RT_NULL)
		{
			rt_free(p_RDData4);
			p_RDData4= RT_NULL;
		}
		if(p_RDData5 != RT_NULL)
		{
			rt_free(p_RDData5);
			p_RDData5= RT_NULL;
		}	
	}
	else
	{
		rt_kprintf("[qspi_test]:argc error!!! \r\n");
	}
}

FINSH_FUNCTION_EXPORT_ALIAS(qspi_psram_dcache_test, __cmd_qspi_test, test qspi_psram_dcache mode);
#endif
