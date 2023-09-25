#include "include.h"
#include "arm_arch.h"

#include "spi.h"
#include "spi_pub.h"

#include "drv_model_pub.h"
#include "intc_pub.h"
#include "mcu_ps_pub.h"
#include "icu_pub.h"
#include "gpio_pub.h"
#include "uart_pub.h"

static SDD_OPERATIONS spi_op =
{
    spi_ctrl
};

static void spi_set_clock(UINT32 max_hz);

static void spi_active(BOOLEAN val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~SPIEN;
    }
    else if(val == 1)
    {
        value |= SPIEN;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_set_msten(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~MSTEN;
    }
    else if(val == 1)
    {
        value |= MSTEN;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_set_ckpha(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~CKPHA;
    }
    else if(val == 1)
    {
        value |= CKPHA;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_set_skpol(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~CKPOL;
    }
    else if(val == 1)
    {
        value |= CKPOL;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_set_bit_wdth(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~BIT_WDTH;
    }
    else if(val == 1)
    {
        value |= BIT_WDTH;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_set_nssmd(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    value &= ~(NSSMD_MASK << NSSMD_POSI);
    value |= (val << NSSMD_POSI);
    REG_WRITE(SPI_CTRL, value);
}

static void spi_rxint_enable(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~RXINT_EN;
    }
    else if(val == 1)
    {
        value |= RXINT_EN;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_txint_enable(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~TXINT_EN;
    }
    else if(val == 1)
    {
        value |= TXINT_EN;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_rxovr_enable(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~RXOVR_EN;
    }
    else if(val == 1)
    {
        value |= RXOVR_EN;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_txovr_enable(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);
    if(val == 0)
    {
        value &= ~TXOVR_EN;
    }
    else if(val == 1)
    {
        value |= TXOVR_EN;
    }
    REG_WRITE(SPI_CTRL, value);
}

static void spi_rxint_mode(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);

    value &= ~(RXINT_MODE_MASK << RXINT_MODE_POSI);
    value |= ((val & RXINT_MODE_MASK) << RXINT_MODE_POSI);
    
    REG_WRITE(SPI_CTRL, value);
}

static void spi_txint_mode(UINT8 val)
{
    UINT32 value;

    value = REG_READ(SPI_CTRL);

    value &= ~(TXINT_MODE_MASK << TXINT_MODE_POSI);
    value |= ((val & TXINT_MODE_MASK) << TXINT_MODE_POSI);
    
    REG_WRITE(SPI_CTRL, value);
}

static void spi_slave_set_cs_finish_interrupt(UINT32 enable)
{
    UINT32 value;

    value = REG_READ(SPI_SLAVE_CTRL);
    if(enable)
    {
        value |= SPI_S_CS_UP_INT_EN;
    }
    else
    {
        value &= ~(SPI_S_CS_UP_INT_EN);
    }
    
    // don't clean cs finish status
    value &= ~(SPI_S_CS_UP_INT_STATUS);
    
    REG_WRITE(SPI_SLAVE_CTRL, value);
}

static void spi_gpio_configuration()
{
    uint32_t val;
    
#if (USE_SPI_GPIO_NUM == USE_SPI_GPIO_14_17)
	val = GFUNC_MODE_SPI;
#elif (USE_SPI_GPIO_NUM == USE_SPI_GPIO_30_33)
	val = GFUNC_MODE_SPI1;
#else
    #error "USE_SPI_GPIO_NUM must set to gpio14-17 or gpio30-33"
#endif

	sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &val);
}

static void spi_icu_configuration(UINT32 enable)
{
    UINT32 param;

    if(enable) 
    {
        param = PWD_SPI_CLK_BIT;
	    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

        param = (IRQ_SPI_BIT);
        sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
    }
    else
    {
        param = (IRQ_SPI_BIT);
        sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);
        
        param = PWD_SPI_CLK_BIT;
	    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_DOWN, &param);
    }
}

static void spi_init_msten(UINT8 msten)
{
    UINT32 value = 0;

    value = REG_READ(SPI_CTRL);
    value &= ~((0x03UL << TXINT_MODE_POSI) | (0x03UL << RXINT_MODE_POSI));
    value |= RXOVR_EN
                | TXOVR_EN
                //| RXINT_EN
                //| TXINT_EN
                | (0x3UL << RXINT_MODE_POSI)   // has 1byte 
                | (0x3UL << TXINT_MODE_POSI);   // 12byte left

    REG_WRITE(SPI_CTRL, value);
    if(msten == 0)
    {
        spi_slave_set_cs_finish_interrupt(1);
    }
    else
    {
        spi_slave_set_cs_finish_interrupt(0);
    }
    
    spi_icu_configuration(1);
    spi_gpio_configuration();
}

static void spi_deinit_msten(void)
{
    UINT32 status, slv_status;

    spi_icu_configuration(0);
    
    REG_WRITE(SPI_CTRL, 0);

    status = REG_READ(SPI_STAT);
    slv_status = REG_READ(SPI_SLAVE_CTRL);

    REG_WRITE(SPI_STAT, status);
    REG_WRITE(SPI_SLAVE_CTRL, slv_status);
}

static UINT8 spi_get_busy(void)
{
    UINT32 value;

    value = REG_READ(SPI_STAT);
    if(value & SPIBUSY)
    {
        return 1;
    }

    return 0;
}

static void spi_rxfifo_clr(void)
{
    UINT32 value;
    
    value = REG_READ(SPI_STAT);
    
    while((value & RXFIFO_EMPTY) == 0)
    {
        REG_READ(SPI_DAT);
        value = REG_READ(SPI_STAT);
    } 
}

UINT32 spi_read_rxfifo(UINT8 *data)
{
    UINT32 value;
    
    value = REG_READ(SPI_STAT);
    
    if((value & RXFIFO_EMPTY) == 0)
    {
        value = REG_READ(SPI_DAT);
        if(data)
            *data = value;
        return 1;
    }

    return 0;
}

static void spi_txfifo_fill(void)
{
    UINT32 value;
    
    value = REG_READ(SPI_STAT);
    
    while((value & TXFIFO_FULL) == 0)
    {
        REG_WRITE(SPI_DAT, 0xff);
    } 
}

UINT32 spi_write_txfifo(UINT8 data)
{
    UINT32 value;
    
    value = REG_READ(SPI_STAT);
    
    if((value & TXFIFO_FULL) == 0)
    {
        REG_WRITE(SPI_DAT, data);
        return 1;
    }

    return 0;
}

void spi_init(void)
{
    intc_service_register(IRQ_SPI, PRI_IRQ_SPI, spi_isr);

    sddev_register_dev(SPI_DEV_NAME, &spi_op);
}

void spi_exit(void)
{
    sddev_unregister_dev(SPI_DEV_NAME);
}

static struct spi_callback_des spi_receive_callback = {NULL, NULL};
static struct spi_callback_des spi_txfifo_needwr_callback = {NULL, NULL};
static struct spi_callback_des spi_tx_end_callback = {NULL, NULL};

static void spi_rx_callback_set(uart_callback callback, void *param)
{
    spi_receive_callback.callback = callback;
    spi_receive_callback.param = param;
}

static void spi_tx_fifo_needwr_callback_set(uart_callback callback, void *param)
{
    spi_txfifo_needwr_callback.callback = callback;
    spi_txfifo_needwr_callback.param = param;
}

static void spi_tx_end_callback_set(uart_callback callback, void *param)
{
    spi_tx_end_callback.callback = callback;
    spi_tx_end_callback.param = param;
}


UINT32 spi_ctrl(UINT32 cmd, void *param)
{
    UINT32 ret = SPI_SUCCESS;

    peri_busy_count_add();

    switch(cmd)
    {
    case CMD_SPI_UNIT_ENABLE:
        spi_active(*(UINT8 *)param);
        break;
    case CMD_SPI_SET_MSTEN:
        spi_set_msten(*(UINT8 *)param);
        break;
    case CMD_SPI_SET_CKPHA:
        spi_set_ckpha(*(UINT8 *)param);
        break;
    case CMD_SPI_SET_CKPOL:
        spi_set_skpol(*(UINT8 *)param);
        break;
    case CMD_SPI_SET_BITWIDTH:
        spi_set_bit_wdth(*(UINT8 *)param);
        break;
    case CMD_SPI_SET_NSSID:
        spi_set_nssmd(*(UINT8 *)param);
        break;
    case CMD_SPI_SET_CKR:
        spi_set_clock(*(UINT32 *)param);
        break;
    case CMD_SPI_RXINT_EN:
        spi_rxint_enable(*(UINT8 *)param);
        break;
    case CMD_SPI_TXINT_EN:
        spi_txint_enable(*(UINT8 *)param);
        break;
    case CMD_SPI_RXOVR_EN:
        spi_rxovr_enable(*(UINT8 *)param);
        break;
    case CMD_SPI_TXOVR_EN:
        spi_txovr_enable(*(UINT8 *)param);
        break;
    case CMD_SPI_RXFIFO_CLR:
        spi_rxfifo_clr();
        break;
    case CMD_SPI_RXINT_MODE:
        spi_rxint_mode(*(UINT8 *)param);
        break;
    case CMD_SPI_TXINT_MODE:
        spi_txint_mode(*(UINT8 *)param);
        break;
    case CMD_SPI_INIT_MSTEN:
        spi_init_msten(*(UINT8 *)param);
        break;
    case CMD_SPI_GET_BUSY:
        (*((UINT8 *)param)) = spi_get_busy();
        break;
    case CMD_SPI_SET_RX_CALLBACK:
        {
            struct spi_callback_des *callback = (struct spi_callback_des *)param;
            spi_rx_callback_set(callback->callback, callback->param);
        }
        break;
    case CMD_SPI_SET_TX_NEED_WRITE_CALLBACK:
        {
            struct spi_callback_des *callback = (struct spi_callback_des *)param;
            spi_tx_fifo_needwr_callback_set(callback->callback, callback->param);
        }
        break;
    case CMD_SPI_SET_TX_FINISH_CALLBACK:
        {
            struct spi_callback_des *callback = (struct spi_callback_des *)param;
            spi_tx_end_callback_set(callback->callback, callback->param);
        }
        break;
    case CMD_SPI_DEINIT:
        spi_deinit_msten();
        break;
    default:
        ret = SPI_FAILURE;
        break;
    }

    peri_busy_count_dec();

    return ret;
}

void spi_isr(void)
{
    UINT32 status, slv_status;
    volatile UINT8 fifo_empty_num, data_num, rxfifo_empty;

    //REG_WRITE((0x00802800+(19*4)), 0x02);
    
	data_num = 0; /*fix warning by clang analyzer*/
	fifo_empty_num = 0; /*fix warning by clang analyzer*/
	
    status = REG_READ(SPI_STAT);
    slv_status = REG_READ(SPI_SLAVE_CTRL);

    REG_WRITE(SPI_STAT, status);
    REG_WRITE(SPI_SLAVE_CTRL, slv_status);

    //os_printf("0x%08x, 0x%08x\r\n", status, slv_status);
    //REG_WRITE((0x00802800+(19*4)), 0x00);
    
    if((status & RXINT) || (slv_status & SPI_S_CS_UP_INT_STATUS)) 
    {
        if (spi_receive_callback.callback != 0)
        {
            void *param = spi_receive_callback.param;
            int is_rx_end = (slv_status & SPI_S_CS_UP_INT_STATUS) ? 1 : 0;

            spi_receive_callback.callback(is_rx_end, param);
        }
        else
        {
            /*drop data*/
            spi_rxfifo_clr();
        }
    }

    if(status & TXINT)
    {
        if (spi_txfifo_needwr_callback.callback != 0)
        {
            void *param = spi_txfifo_needwr_callback.param;

            spi_txfifo_needwr_callback.callback(0, param);
        }
        else
        {
            /*fill txfifo with 0xff*/
            //spi_txfifo_fill();
        }
    }

    if(status & TXOVR)
    {
        os_printf("txovr\r\n");
    }

    if(status & RXOVR)
    {
        os_printf("rxovr\r\n");
    }
	
    if(status & MODF)
    {
        os_printf("spi mode error\r\n");
    }
	
    if(status & TXFIFO_EMPTY)
    {
        if (spi_tx_end_callback.callback != 0)
        {
            void *param = spi_tx_end_callback.param;

            spi_tx_end_callback.callback(0, param);
        }
        else
        {
            /*fill txfifo with 0xff*/
            //spi_txfifo_fill();
        }
    }
}

void spi_set_clock(UINT32 max_hz)
{
/*
    spi_clk : 90M/3=30M    - DC0180 - DIV2
    spi_clk : 90M/4=22.5M  - DC0180 - DIV3
    spi_clk : 90M/5=18M    - DC0180 - DIV4
    spi_clk : 90M/6=15M    - DC0180 - DIV5
    spi_clk : 90M/7=12.85M - DC0180 - DIV6
    spi_clk : 90M/8=11.25M - DC0180 - DIV7
    spi_clk : 90M/9=10M    - DC0180 - DIV8
    spi_clk : 90M/10=9M    - DC0180 - DIV9
    spi_clk : 90M/11=8.18M - DC0180 - DIV10
*/
#define SPI_PERI_CLK_26M        (26 * 1000 * 1000)
#define SPI_PERI_CLK_DCO        (180 * 1000 * 1000)
    
    int source_clk = 0;
    int spi_clk = 0;
    int div = 0;
    UINT32 param;

    if (max_hz > 4333000)
    {
        BK_SPI_PRT("config spi clk source DCO\n");

        if(max_hz > 30000000) // 180M/2 / (2 + 1) = 30M
        {
            spi_clk = 30000000; 
            BK_SPI_PRT("input clk > 30MHz, set input clk = 30MHz\n");
        } else {
            spi_clk = max_hz;
        }
        
        source_clk = SPI_PERI_CLK_DCO;
        param = PCLK_POSI_SPI;
    	sddev_control(ICU_DEV_NAME, CMD_CONF_PCLK_DCO, &param);
    }
    else 
    {
        BK_SPI_PRT("config spi clk source 26MHz\n");

        spi_clk = max_hz;
        source_clk = SPI_PERI_CLK_26M;
            
        param = PCLK_POSI_SPI;
	    sddev_control(ICU_DEV_NAME, CMD_CONF_PCLK_26M, &param);
    }

    // spi_clk = in_clk / (2 * (div + 1))
    div = ((source_clk >> 1) / spi_clk) - 1; 

    if (div < 2)
    {
        div = 2; 
    }
    else if (div >= 255)
    {
        div = 255;
    }

    param = REG_READ(SPI_CTRL);
    param &= ~(SPI_CKR_MASK << SPI_CKR_POSI);
    param |= (div << SPI_CKR_POSI);
    REG_WRITE(SPI_CTRL, param);
    
    BK_SPI_PRT("div = %d \n", div);
    BK_SPI_PRT("spi_clk = %d \n", spi_clk);
    BK_SPI_PRT("source_clk = %d \n", source_clk);
    BK_SPI_PRT("target frequency = %d, actual frequency = %d \n", max_hz, source_clk / 2 / (div + 1));
}

// eof 

