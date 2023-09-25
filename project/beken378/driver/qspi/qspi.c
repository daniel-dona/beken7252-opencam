#include "include.h"
#include "arm_arch.h"

#include "drv_model_pub.h"
#include "intc_pub.h"

#include "qspi.h"
#include "qspi_pub.h"
#include "sys_ctrl_pub.h"

#include "drv_model_pub.h"
#include "intc_pub.h"
#include "mcu_ps_pub.h"
#include "icu_pub.h"
#include "gpio_pub.h"
#include "uart_pub.h"


static SDD_OPERATIONS qspi_op = 
{
	qspi_ctrl
};


void qspi_init(void)
{
	os_printf("QSPi_init\r\n");
	intc_service_register(FIQ_PSRAM, PRI_IRQ_QSPI, qspi_isr); 
	sddev_register_dev(QSPI_DEV_NAME, &qspi_op);	
	os_printf("QSPi_init1\r\n");
}

void qspi_exit(void)
{
    sddev_unregister_dev(QSPI_DEV_NAME);
}

static void qspi_psram_set_voltage(UINT32 mode)
{
    UINT32 param;

    param = mode;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_IO_VOLTAGE, &param);
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_VDDRAM_VOLTAGE, &param);
}



static void qspi_gpio_configuration(UINT8 LineMode)
{
    uint32_t val;

	val = GFUNC_MODE_QSPI_CLK;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &val);
	
	val = GFUNC_MODE_QSPI_CSN;
	sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &val);

	if(LineMode == 1)
	{
		val = GFUNC_MODE_QSPI_1LINE;
		sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &val);	
	}
	else if(LineMode == 4)
	{
		val = GFUNC_MODE_QSPI_4LINE;
		sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &val);			
	}

}

static void qspi_icu_configuration(UINT32 enable)
{
    UINT32 param;

    if(enable) 
    {
        param = PWD_QSPI_CLK_BIT;
	    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

        param = (FIQ_PSRAM_BIT);
        sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);

		param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    	sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);
    }
    else
    {
        param = (FIQ_PSRAM_BIT);
        sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);
        
        param = PWD_QSPI_CLK_BIT;
	    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_DOWN, &param);

		param = (~BLK_BIT_MIC_QSPI_RAM_OR_FLASH);
    	sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);
    }
}

static void qspi_clk_set_26M(void)
{		
	UINT32 param;
	param = PCLK_POSI_QSPI_26M;
	sddev_control(ICU_DEV_NAME, CMD_CONF_PCLK_26M, &param);
}

static void qspi_clk_set_dco(void)
{		
	UINT32 param;
	param = PCLK_POSI_QSPI_DCO;
	sddev_control(ICU_DEV_NAME, PCLK_POSI_QSPI_DCO, &param);
}

static void qspi_clk_set_120M(void)
{		
	UINT32 param;
	param = PCLK_POSI_QSPI_120M;
	sddev_control(ICU_DEV_NAME, PCLK_POSI_QSPI_120M, &param);
}

static void qspi_div_clk_set(UINT32 clk)
{
	UINT32 val;

	val = REG_READ(QSPI_CTRL);
	val = (val & (~(0x07 << 8)) | ((clk & 0x07) << 8));
	
	REG_WRITE(QSPI_CTRL, val);
}

static void qspi_dcache_wr_cmd(UINT8 command ,UINT8 linemode)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_WR_CMD);

	if(linemode == 0)
	{
		val = val | (0 << 1) | (8 << 10);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1) | (2 << 10);	
	}
	
	val = val | (command << 2) | (1 << 0); // enable	

	REG_WRITE(QSPI_DCACHE_WR_CMD, val);	
}

static void qspi_dcache_wr_addr(UINT8 linemode)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_WR_ADDR);

	if(linemode == 0)
	{
		val = val | (0 << 1) | ((8*3) << 26);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1) | ((2*3) << 26);	
	}
	
	val = val | (1 << 0); // enable	

	REG_WRITE(QSPI_DCACHE_WR_ADDR, val);	
}

static void qspi_dcache_wr_dum(UINT8 linemode, UINT8 dummy_size)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_WR_DUM);

	if(linemode == 0)
	{
		val = val | (0 << 1);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1);	
	}
	
	val = val | (dummy_size << 2) | ((dummy_size? 1 : 0 )<< 0 );

	REG_WRITE(QSPI_DCACHE_WR_DUM, val);	
}

static void qspi_dcache_wr_dat(UINT8 linemode)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_WR_DAT);

	if(linemode == 0)
	{
		val = val | (0 << 1) | ((4*8*4) << 2);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1) | ((4*2*4) << 2);	
	}
	
	val = val | (1 << 0) | (1 << 14); // enable	

	REG_WRITE(QSPI_DCACHE_WR_DAT, val);	
}

static void qspi_dcache_rd_cmd(UINT8 command ,UINT8 linemode)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_RD_CMD);

	if(linemode == 0)
	{
		val = val | (0 << 1) | (8 << 10);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1) | (2 << 10);	
	}
	
	val = val | (command << 2) | (1 << 0); // enable	

	REG_WRITE(QSPI_DCACHE_RD_CMD, val);	
}

static void qspi_dcache_rd_addr(UINT8 linemode)
{
	UINT32 val = 0;

	if(linemode == 0)
	{
		val = val | (0 << 1) | ((8*3) << 26);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1) | ((2*3) << 26);	
	}
	
	val = val | (1 << 0); // enable	

	REG_WRITE(QSPI_DCACHE_RD_ADDR, val);	
}

static void qspi_dcache_rd_dum(UINT8 linemode, UINT8 dummy_size)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_RD_DUM);

	if(linemode == 0)
	{
		val = val | (0 << 1);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1);	
	}
	
	val = val | (dummy_size << 2) | ((dummy_size? 1 : 0 ) << 0 );

	REG_WRITE(QSPI_DCACHE_RD_DUM, val);	
}

static void qspi_dcache_rd_dat(UINT8 linemode)
{
	UINT32 val;

	val = REG_READ(QSPI_DCACHE_RD_DAT);

	if(linemode == 0)
	{
		val = val | (0 << 1) | ((4*8*4) << 2);	
	}
	else if(linemode == 3)
	{
		val = val | (1 << 1) | ((4*2*4) << 2);	
	}

	val = val & (~(1 << 14));
	val = val | (1 << 0) ; // enable	

	REG_WRITE(QSPI_DCACHE_RD_DAT, val);	
}

static void qspi_dcache_request(void)
{
	REG_WRITE(QSPI_DCACHE_REQUEST, 1);	
}


static UINT32 qspi_open(UINT32 op_flag)
{
	UINT32 value;
	value = op_flag;
	qspi_icu_configuration(value);
	return QSPI_SUCCESS;
}

static UINT32 qspi_close(void)
{
	qspi_icu_configuration(0);
	return QSPI_SUCCESS;
}

static UINT32 qspi_ctrl(UINT32 cmd, void *param)
{
	UINT32 ret = QSPI_SUCCESS;

	qspi_dcache_drv_desc * p_param;
	
    peri_busy_count_add();

	switch(cmd){
	case QSPI_CMD_SET_VOLTAGE:
		qspi_psram_set_voltage(*(UINT8 *)param);
		break;
	case QSPI_CMD_DCACHE_CONFIG:
		p_param = (qspi_dcache_drv_desc *)param;
		ret = bk_qspi_dcache_configure(p_param);
		break;
	case QSPI_CMD_GPIO_CONFIG:
		qspi_gpio_configuration(*(UINT8 *)param);
		break;		
	case QSPI_CMD_DIV_CLK_SET:
		qspi_div_clk_set(*(UINT8 *)param);
		break;
	case QSPI_CMD_CLK_SET_26M:
		qspi_clk_set_26M();
		break;	
	case QSPI_CMD_CLK_SET_DCO:
		qspi_clk_set_dco();
		break;	
	case QSPI_CMD_CLK_SET_120M:
		qspi_clk_set_120M();
		break;
	case QSPI_DCACHE_CMD_OPEN:	
		qspi_open(1);
		break;	
	case QSPI_DCACHE_CMD_CLOSE:
		qspi_close();
		break;
	
	default:
		ret = QSPI_FAILURE;
		break;
	}

    peri_busy_count_dec();

	return ret;
}

int bk_qspi_dcache_configure(qspi_dcache_drv_desc *qspi_cfg)
{
	UINT32 line_mode, param;
	UINT32 wr_cmd, rd_cmd, wr_dummy_size, rd_dummy_size;

	ASSERT(qspi_cfg != NULL);
		
	wr_cmd = qspi_cfg->wr_command;
	rd_cmd = qspi_cfg->rd_command;
	wr_dummy_size = qspi_cfg->wr_dummy_size;
	rd_dummy_size = qspi_cfg->rd_dummy_size;	

	if ((qspi_cfg->mode & 0x03) == 0)		// 1 Linemode
	{
		line_mode = 0;
	}
	else if ((qspi_cfg->mode & 0x03) == 3)	//3 linemode
	{
		line_mode = 3;
	}
	else
	{
		os_printf("[QSPI]:line mode error\r\n");
		return QSPI_FAILURE;
	}
	
	param = qspi_cfg->clk_set;
	qspi_div_clk_set(param);

	qspi_dcache_wr_cmd(wr_cmd, line_mode);
	qspi_dcache_wr_addr(line_mode);
	qspi_dcache_wr_dum(line_mode, wr_dummy_size);
	qspi_dcache_wr_dat(line_mode);
	
	qspi_dcache_rd_cmd(rd_cmd, line_mode);
	qspi_dcache_rd_addr(line_mode);
	qspi_dcache_rd_dum(line_mode, rd_dummy_size);
	qspi_dcache_rd_dat(line_mode);

	qspi_dcache_request();	
	
	return QSPI_SUCCESS;
}


void bk_qspi_mode_start(UINT32 mode, UINT32 div)
{
	UINT32 param;
	qspi_open(1);

	param = 2;
	qspi_ctrl(QSPI_CMD_SET_VOLTAGE, (void *)&param);
	
	qspi_ctrl(QSPI_CMD_CLK_SET_26M, NULL);
	
	param = div;
	qspi_ctrl(QSPI_CMD_DIV_CLK_SET,(void *)&param);

	param = mode;
	qspi_ctrl(QSPI_CMD_GPIO_CONFIG, (void *)&param);
	
}



int bk_qspi_dcache_write_data(UINT32 set_addr, UINT32 *wr_data, UINT32 data_length)
{
	UINT32 i ;
	UINT32 value;

	if(wr_data == NULL)
	{
		return QSPI_FAILURE;
	}
	
	for(i = 0; i < data_length; i++)
	{
		value = *wr_data ;
		
		REG_WRITE(QSPI_DCACHE_BASE + set_addr + i*4 , value);
		wr_data	 ++;
	}
	return QSPI_SUCCESS;	
}

int bk_qspi_dcache_read_data(UINT32 set_addr, UINT32 *rd_data, UINT32 data_length)

{
	UINT32 i ;
	UINT32 value;

	if(rd_data == NULL)
	{
		return QSPI_FAILURE;
	}
	
	for(i = 0; i < data_length; i++)
	{		
		value = REG_READ((QSPI_DCACHE_BASE + set_addr + i*4) );
		*rd_data = value;
		rd_data	 ++;
	}
	return QSPI_SUCCESS;	
}


void qspi_printf(void)
{
    os_printf("QSPI_CTRL=0x%lx \r\n", REG_READ(QSPI_CTRL));
    os_printf("QSPI_DCACHE_WR_CMD=0x%lx \r\n", REG_READ(QSPI_DCACHE_WR_CMD));
    os_printf("QSPI_DCACHE_WR_ADDR=0x%lx \r\n", REG_READ(QSPI_DCACHE_WR_ADDR));
    os_printf("QSPI_DCACHE_WR_DUM=0x%lx \r\n", REG_READ(QSPI_DCACHE_WR_DUM));
    os_printf("QSPI_DCACHE_WR_DAT=0x%lx \r\n", REG_READ(QSPI_DCACHE_WR_DAT));
    os_printf("QSPI_DCACHE_RD_CMD=0x%lx \r\n", REG_READ(QSPI_DCACHE_RD_CMD));
    os_printf("QSPI_DCACHE_RD_ADDR=0x%lx \r\n", REG_READ(QSPI_DCACHE_RD_ADDR));
    os_printf("QSPI_DCACHE_RD_DUM=0x%lx \r\n", REG_READ(QSPI_DCACHE_RD_DUM));
    os_printf("QSPI_DCACHE_RD_DAT=0x%lx \r\n", REG_READ(QSPI_DCACHE_RD_DAT));
    os_printf("QSPI_DCACHE_REQUEST=0x%lx \r\n", REG_READ(QSPI_DCACHE_REQUEST));	
}

void qspi_isr(void)
{
    
}


