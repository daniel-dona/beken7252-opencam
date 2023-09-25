#include "include.h"
#include "arm_arch.h"

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

#define DPLL_DIV                0x0
#define DCO_CALIB_26M           0x1
#define DCO_CALIB_60M           0x2
#define DCO_CALIB_80M           0x3
#define DCO_CALIB_120M          0x4
#define DCO_CALIB_180M          0x5

#if (CFG_SOC_NAME == SOC_BK7221U)  
#define DCO_CLK_SELECT          DCO_CALIB_180M
#define USE_DCO_CLK_POWON       1

UINT8  calib_charger[3] = {
    0x23,   //vlcf
    0x15,   //icp
    0x1b    //vcv
    };
#else
#define DCO_CLK_SELECT          DCO_CALIB_120M
#define USE_DCO_CLK_POWON       0
#endif

static SCTRL_PS_SAVE_VALUES ps_saves[2];
static UINT32 ps_saves_gpio_cfgs[32];
static SCTRL_MCU_PS_INFO sctrl_mcu_ps_info =
{
    .hw_sleep = 0,
    .mcu_use_dco = 0,
    .first_sleep = 1,
};

 UINT32 rf_sleeped = 0;

static SDD_OPERATIONS sctrl_op =
{
    sctrl_ctrl
};

extern void WFI( void );
/**********************************************************************/
#if DPLL_DELAY_EN
void sctrl_dpll_delay10us(void)
{
    volatile UINT32 i = 0;
	
    for(i = 0; i < DPLL_DELAY_TIME_10US; i ++)
    {
        ;
    }
}

void sctrl_dpll_delay200us(void)
{
    volatile UINT32 i = 0;
	
    for(i = 0; i < DPLL_DELAY_TIME_200US; i ++)
    {
        ;
    }
}

void sctrl_ps_dpll_delay(UINT32 time)
{
    volatile UINT32 i = 0;

    for(i = 0; i < time; i ++)
    {
        ;
    }
}
#endif

void sctrl_cali_dpll(UINT8 flag)
{
    UINT32 param;
   
    param = sctrl_analog_get(SCTRL_ANALOG_CTRL0);
    param &= ~(SPI_TRIG_BIT);
    sctrl_analog_set(SCTRL_ANALOG_CTRL0, param);
#if DPLL_DELAY_EN

    if(!flag)
        sctrl_dpll_delay10us();
    else
        sctrl_ps_dpll_delay(60);

#endif
    param |= (SPI_TRIG_BIT);
    sctrl_analog_set(SCTRL_ANALOG_CTRL0, param);   
    
    param = sctrl_analog_get(SCTRL_ANALOG_CTRL0);
    param &= ~(SPI_DET_EN);
    sctrl_analog_set(SCTRL_ANALOG_CTRL0, param);
#if DPLL_DELAY_EN

    if(!flag)
        sctrl_dpll_delay200us();
    else
        sctrl_ps_dpll_delay(340);
#endif
    param = sctrl_analog_get(SCTRL_ANALOG_CTRL0);
    param |= (SPI_DET_EN);
    sctrl_analog_set(SCTRL_ANALOG_CTRL0, param);
}

void sctrl_dpll_isr(void)
{
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CLR_DPLL_UNLOOK_INT_BIT, NULL);    
    sctrl_cali_dpll(0);

    os_printf("DPLL Unlock\r\n");
}

void sctrl_dpll_int_open(void)
{
    UINT32 param;
    
    param = (FIQ_DPLL_UNLOCK_BIT);
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);

    #if (CFG_SOC_NAME != SOC_BK7231)
    param = 1;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_EN_DPLL_UNLOOK_INT, &param);
    #endif
}

void sctrl_dpll_int_close(void)
{
    UINT32 param;
    
    #if (CFG_SOC_NAME != SOC_BK7231)
    param = 0;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_EN_DPLL_UNLOOK_INT, &param);
    #endif
    
    param = (FIQ_DPLL_UNLOCK_BIT);
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);
}

void sctrl_dco_cali(UINT32 speed)
{
    UINT32 reg_val;
    
    switch(speed) 
    {
        case DCO_CALIB_180M: 
        reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
        reg_val &= ~((DCO_CNTI_MASK << DCO_CNTI_POSI) | (DCO_DIV_MASK << DCO_DIV_POSI));
        reg_val |= ((0xDD & DCO_CNTI_MASK) << DCO_CNTI_POSI);
        reg_val |= DIV_BYPASS_BIT;
        sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val);   
        break;
        
        case DCO_CALIB_120M:             
        reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
        reg_val &= ~((DCO_CNTI_MASK << DCO_CNTI_POSI) | (DCO_DIV_MASK << DCO_DIV_POSI));
        reg_val |= ((0x127 & DCO_CNTI_MASK) << DCO_CNTI_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val);   
        break;

        case DCO_CALIB_80M:             
        reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
        reg_val &= ~((DCO_CNTI_MASK << DCO_CNTI_POSI) | (DCO_DIV_MASK << DCO_DIV_POSI));
        reg_val |= ((0x0C5 & DCO_CNTI_MASK) << DCO_CNTI_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val);   
        break;

        case DCO_CALIB_60M:             
        reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
        reg_val &= ~((DCO_CNTI_MASK << DCO_CNTI_POSI) | (DCO_DIV_MASK << DCO_DIV_POSI));
        reg_val |= ((0x127 & DCO_CNTI_MASK) << DCO_CNTI_POSI);
        reg_val |= ((0x02 & DCO_DIV_MASK) << DCO_DIV_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val);   
        break;

        default:
        reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
        reg_val &= ~((DCO_CNTI_MASK << DCO_CNTI_POSI) | (DCO_DIV_MASK << DCO_DIV_POSI));
        reg_val |= ((0xC0 & DCO_CNTI_MASK) << DCO_CNTI_POSI);
        reg_val |= ((0x03 & DCO_DIV_MASK) << DCO_DIV_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val);  
        break;
    }

    reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
    reg_val &= ~(SPI_RST_BIT);
    sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val); 

    reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
    reg_val |= SPI_RST_BIT;
    sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val); 

    reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
    reg_val |= DCO_TRIG_BIT;
    sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val); 
    
    reg_val = sctrl_analog_get(SCTRL_ANALOG_CTRL1);
    reg_val &= ~(DCO_TRIG_BIT);
    sctrl_analog_set(SCTRL_ANALOG_CTRL1, reg_val); 
}

void sctrl_set_cpu_clk_dco(void)
{
    UINT32 reg_val;

    reg_val = REG_READ(SCTRL_CONTROL);
    reg_val &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    reg_val &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
    
    reg_val |= ((MCLK_FIELD_DCO&MCLK_MUX_MASK) << MCLK_MUX_POSI);
    reg_val |= HCLK_DIV2_EN_BIT;    
    delay(10);
    REG_WRITE(SCTRL_CONTROL, reg_val); 
}

#if CFG_USE_STA_PS

void sctrl_flash_select_dco(void)
{
    UINT32 reg;
    DD_HANDLE flash_hdl;
    UINT32 status;

    /* Flash 26MHz clock select dco clock*/
    flash_hdl = ddev_open(FLASH_DEV_NAME, &status, 0);
    ASSERT(DD_HANDLE_UNVALID != flash_hdl);
    ddev_control(flash_hdl, CMD_FLASH_SET_DCO, 0);
    //flash get id  shouldn't remove
    ddev_control(flash_hdl, CMD_FLASH_GET_ID, &reg);
}

void sctrl_sta_ps_init(void)
{
    UINT32 reg;
    extern void power_save_wakeup_isr(void);

    reg = REG_READ(SCTRL_LOW_PWR_CLK);
    reg &=~(LPO_CLK_MUX_MASK);
#if (CFG_SOC_NAME == SOC_BK7231)
    reg |=(LPO_SRC_32K_DIV << LPO_CLK_MUX_POSI);
#else
    reg |=(LPO_SRC_ROSC << LPO_CLK_MUX_POSI);
#endif
    REG_WRITE(SCTRL_LOW_PWR_CLK, reg);

#if (CFG_SOC_NAME != SOC_BK7231)
    reg = REG_READ(SCTRL_BLOCK_EN_MUX);
    reg &=~(0x1FF);
    reg |=(0x40);
    REG_WRITE(SCTRL_BLOCK_EN_MUX, reg);
#endif

    #if PS_WAKEUP_MOTHOD_RW
    intc_service_register(FIQ_MAC_WAKEUP, PRI_FIQ_MAC_WAKEUP, power_save_wakeup_isr);
    nxmac_enable_lp_clk_switch_setf(0x01);
    os_printf("sctrl_sta_ps_init\r\n");
    #endif

    sctrl_flash_select_dco();

    power_save_rf_ps_wkup_semlist_init();
    
}
#endif

void sctrl_ble_ps_init(void)
{
    UINT32 reg;

    reg = REG_READ(SCTRL_LOW_PWR_CLK);
    reg &=~(LPO_CLK_MUX_MASK);
#if (CFG_SOC_NAME == SOC_BK7231)
    reg |=(LPO_SRC_32K_DIV << LPO_CLK_MUX_POSI);
#else
    reg |=(LPO_SRC_ROSC << LPO_CLK_MUX_POSI);
#endif
    REG_WRITE(SCTRL_LOW_PWR_CLK, reg);

}

void sctrl_init(void)
{
    UINT32 param,reg;

    sddev_register_dev(SCTRL_DEV_NAME, &sctrl_op);

    /*enable blk clk
      Attention: ENABLE 26m xtal block(BLK_BIT_26M_XTAL), for protect 32k circuit
     */
    param = BLK_BIT_26M_XTAL | BLK_BIT_DPLL_480M | BLK_BIT_XTAL2RF | BLK_BIT_DCO;
    sctrl_ctrl(CMD_SCTRL_BLK_ENABLE, &param);

    /*config main clk*/
    #if !USE_DCO_CLK_POWON
    param = REG_READ(SCTRL_CONTROL);
    param &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    param &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
    #if (CFG_SOC_NAME == SOC_BK7221U)  
    /* BK7221U ahb bus max rate is 90MHZ, so ahb bus need div 2 from MCU clock */
    /* AHB bus is very import to AUDIO and DMA */
    param |= HCLK_DIV2_EN_BIT;
    #endif // (CFG_SOC_NAME == SOC_BK7221U)
    #if CFG_SYS_REDUCE_NORMAL_POWER
    param |= ((MCLK_DIV_7 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
    #else // CFG_SYS_REDUCE_NORMAL_POWER 
    param |= ((MCLK_DIV_3 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
    #endif // CFG_SYS_REDUCE_NORMAL_POWER
    param |= ((MCLK_FIELD_DPLL & MCLK_MUX_MASK) << MCLK_MUX_POSI);
    REG_WRITE(SCTRL_CONTROL, param);
    #endif // (!USE_DCO_CLK_POWON)

    /*sys_ctrl <0x4c> */
    param = 0x00171710;//0x00151510;    LDO BIAS CALIBRATION
    REG_WRITE(SCTRL_BIAS, param);

    /*mac & modem power up */
    sctrl_ctrl(CMD_SCTRL_MAC_POWERUP, NULL);
    sctrl_ctrl(CMD_SCTRL_MODEM_POWERUP, NULL);

    param = 0x819A59B;
    sctrl_analog_set(SCTRL_ANALOG_CTRL0, param);
    
    sctrl_cali_dpll(0);
    
    param = 0x6AC03102;
    sctrl_analog_set(SCTRL_ANALOG_CTRL1, param);
    /*do dco Calibration*/
    sctrl_dco_cali(DCO_CLK_SELECT);
    #if USE_DCO_CLK_POWON
    sctrl_set_cpu_clk_dco();
    #endif

    #if (CFG_SOC_NAME == SOC_BK7231)
    param = 0x24006000;
    #else
    param = 0x24026080;   // xtalh_ctune   // 24006080
    param &= ~(XTALH_CTUNE_MASK<< XTALH_CTUNE_POSI);
    param |= ((0x10&XTALH_CTUNE_MASK) << XTALH_CTUNE_POSI);
    #endif // (CFG_SOC_NAME == SOC_BK7231)
    sctrl_analog_set(SCTRL_ANALOG_CTRL2, param);
    
    param = 0x4FE06C50;
    sctrl_analog_set(SCTRL_ANALOG_CTRL3, param);

    /*sys_ctrl <0x1a> */
    #if (CFG_SOC_NAME == SOC_BK7231)
    param = 0x59E04520;
    #else	
    param = 0x59C04520;  // 0x59E04520 
    #endif // (CFG_SOC_NAME == SOC_BK7231)
    sctrl_analog_set(SCTRL_ANALOG_CTRL4, param);

    /*regist intteruppt handler for Dpll unlock*/
    intc_service_register(FIQ_DPLL_UNLOCK, PRI_FIQ_DPLL_UNLOCK, sctrl_dpll_isr);

    sctrl_sub_reset();

	/*sys ctrl clk gating, for rx dma dead*/
	REG_WRITE(SCTRL_CLK_GATING, 0x3f);

	/* increase VDD voltage*/
    #if CFG_SYS_REDUCE_NORMAL_POWER
	param = 4;
    #else
	param = 5;
    #endif
    sctrl_ctrl(CMD_SCTRL_SET_VDD_VALUE, &param);

	#if CFG_USE_STA_PS
	/*32K Rosc calib*/
    REG_WRITE(SCTRL_ROSC_CAL, 0x7);
	#endif

    #if (CFG_SOC_NAME == SOC_BK7221U)
    #if (CFG_USE_AUDIO)
    sctrl_analog_set(SCTRL_ANALOG_CTRL8, 0x0033587C);
    sctrl_analog_set(SCTRL_ANALOG_CTRL9, 0x82204607);
    sctrl_analog_set(SCTRL_ANALOG_CTRL10, 0x80801027);
    #endif // CFG_USE_AUDIO
    #endif // (CFG_SOC_NAME == SOC_BK7221U)

	#if (RHINO_CONFIG_CPU_PWR_MGMT & CFG_USE_STA_PS)
	sctrl_mcu_init();
	#endif
}

void sctrl_exit(void)
{
    sddev_unregister_dev(SCTRL_DEV_NAME);
}

void sctrl_modem_core_reset(void)
{
    sctrl_ctrl(CMD_SCTRL_MODEM_CORE_RESET, 0);
}

void sctrl_sub_reset(void)
{
    sctrl_ctrl(CMD_SCTRL_MPIF_CLK_INVERT, 0);
    sctrl_ctrl(CMD_SCTRL_MODEM_CORE_RESET, 0);
    sctrl_ctrl(CMD_SCTRL_MODEM_SUBCHIP_RESET, 0);
    sctrl_ctrl(CMD_SCTRL_MAC_SUBSYS_RESET, 0);
    sctrl_ctrl(CMD_SCTRL_USB_SUBSYS_RESET, 0);
}

void ps_delay(volatile UINT16 times)
{
	UINT32	delay = times;
    while(delay--) ;
}

void sctrl_ps_dump()
{
    UINT32 i;
	
    os_printf("reg dump\r\n");
    os_printf("sys\r\n0x%8x:0x%8x\r\n", SCTRL_CONTROL, REG_READ(SCTRL_CONTROL));
    os_printf("0x%8x:0x%8x\r\n", SCTRL_MODEM_CORE_RESET_PHY_HCLK, REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK));
    os_printf("0x%8x:0x%8x\r\n", SCTRL_BLOCK_EN_CFG, REG_READ(SCTRL_BLOCK_EN_CFG));
    os_printf("0x%8x:0x%8x\r\n", SCTRL_ROSC_CAL, REG_READ(SCTRL_ROSC_CAL));
    os_printf("0x%8x:0x%8x\r\n", SCTRL_ANALOG_CTRL2, sctrl_analog_get(SCTRL_ANALOG_CTRL2));
    os_printf("0x%8x:0x%8x\r\n", ICU_INTERRUPT_ENABLE, REG_READ(ICU_INTERRUPT_ENABLE));
    os_printf("0x%8x:0x%8x\r\n", ICU_PERI_CLK_PWD, REG_READ(ICU_PERI_CLK_PWD));
    os_printf("0x%8x:0x%8x\r\n", SCTRL_SLEEP, REG_READ(SCTRL_SLEEP));
    os_printf("0x%8x:0x%8x\r\n", ICU_ARM_WAKEUP_EN, REG_READ(ICU_ARM_WAKEUP_EN));
    os_printf("mac\r\n0x%8x:0x%8x\r\n", NXMAC_TIMERS_INT_UN_MASK_ADDR, nxmac_timers_int_un_mask_get());
    os_printf("0x%8x:0x%8x\r\n", NXMAC_DOZE_CNTRL_1_ADDR, nxmac_doze_cntrl_1_get());
    os_printf("0x%8x:0x%8x\r\n", NXMAC_DOZE_CNTRL_2_ADDR, nxmac_doze_cntrl_2_get());
    os_printf("0x%8x:0x%8x\r\n", NXMAC_BCN_CNTRL_1_ADDR, nxmac_bcn_cntrl_1_get());
    os_printf("saves dump\r\n");
    for(i = 0; i < (3 * (sizeof(SCTRL_PS_SAVE_VALUES) / 4)); i++)
        os_printf(" %d 0x%x\r\n", i, *((UINT32 *)(&ps_saves) + i));   
}

void sctrl_hw_sleep(UINT32 peri_clk)
{    
    UINT32 reg;
    DD_HANDLE flash_hdl;
    UINT32 status;
    PS_DEBUG_DOWN_TRIGER;

    if(4 == flash_get_line_mode())
    {
        flash_set_line_mode(2);
    }


    if(power_save_if_rf_sleep())
    {
        reg = REG_READ(ICU_ARM_WAKEUP_EN); 
        reg |= (MAC_ARM_WAKEUP_EN_BIT);
    	REG_WRITE(ICU_ARM_WAKEUP_EN, reg);      
    }
    PS_DEBUG_DOWN_TRIGER;

#if (CFG_SOC_NAME == SOC_BK7231)
    REG_WRITE(SCTRL_ROSC_CAL, 0x35);
    REG_WRITE(SCTRL_ROSC_CAL, 0x37);
#endif

    if(sctrl_mcu_ps_info.mcu_use_dco == 0)
    {
        /* MCLK(main clock) select:dco*/
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);       
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);
        PS_DEBUG_DOWN_TRIGER;
    }

    /*close 32K Rosc calib*/
#if (CFG_SOC_NAME == SOC_BK7231)
    REG_WRITE(SCTRL_ROSC_CAL, 0x36);
#endif
    PS_DEBUG_DOWN_TRIGER;
    /* dpll division reset*/
    reg = REG_READ(SCTRL_CONTROL);
    reg |= (0x1 << 14);
    REG_WRITE(SCTRL_CONTROL, reg);
    PS_DEBUG_DOWN_TRIGER;
    /* dpll (480m) & xtal2rf  disable*/
    reg = REG_READ(SCTRL_BLOCK_EN_CFG);
    reg &= ~(BLOCK_EN_WORD_MASK << BLOCK_EN_WORD_POSI);
    reg = reg | (BLOCK_EN_WORD_PWD << BLOCK_EN_WORD_POSI);
    reg &= ~(BLK_EN_DPLL_480M | BLK_EN_XTAL2RF );
    REG_WRITE(SCTRL_BLOCK_EN_CFG, reg);
    PS_DEBUG_DOWN_TRIGER;
    /* center bias power down*/
    reg = sctrl_analog_get(SCTRL_ANALOG_CTRL2);
    reg &= (~(1 << 13));
    sctrl_analog_set(SCTRL_ANALOG_CTRL2, reg);
    sctrl_mcu_ps_info.hw_sleep = 1;

    while(sctrl_analog_get(SCTRL_ANALOG_CTRL2) & (1 << 13));

    PS_DEBUG_DOWN_TRIGER;
#if PS_CLOSE_PERI_CLK
    /* close all peri clock*/
    ps_saves[0].peri_clk_cfg= REG_READ(ICU_PERI_CLK_PWD);    
    REG_WRITE(ICU_PERI_CLK_PWD, peri_clk);
#endif
    /* arm clock disable */
    reg = REG_READ(SCTRL_SLEEP);
    reg &= ~(SLEEP_MODE_MASK << SLEEP_MODE_POSI);
    reg = reg | SLEEP_MODE_CFG_NORMAL_VOL_WORD;
    REG_WRITE(SCTRL_SLEEP, reg); 
    ps_delay(1);//5
    	
}

void sctrl_hw_wakeup()
{
    UINT32 reg;
    
	PS_DEBUG_BCN_TRIGER;   
    /* center bias power on*/ 
    reg = sctrl_analog_get(SCTRL_ANALOG_CTRL2);
    reg |= (1<<13);
    sctrl_analog_set(SCTRL_ANALOG_CTRL2, reg); 
    
    while((sctrl_analog_get(SCTRL_ANALOG_CTRL2) & (1<<13))  == 0);
    
    
    /*dpll(480m)  & xtal2rf enable*/
    reg = REG_READ(SCTRL_BLOCK_EN_CFG);
    reg &= ~(BLOCK_EN_WORD_MASK << BLOCK_EN_WORD_POSI);
    reg |= (BLOCK_EN_WORD_PWD << BLOCK_EN_WORD_POSI);
    reg |= ( BLK_EN_DPLL_480M | BLK_EN_XTAL2RF );
    REG_WRITE(SCTRL_BLOCK_EN_CFG, reg);
    ps_delay(10);
    PS_DEBUG_BCN_TRIGER;

    if(sctrl_mcu_ps_info.mcu_use_dco == 0)
    {
        /* MCLK(main clock) select:26M*/
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);    
        reg |= ((MCLK_FIELD_26M_XTAL & MCLK_MUX_MASK) << MCLK_MUX_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);

        ps_delay(500);
	    PS_DEBUG_BCN_TRIGER;   
        /* dpll division reset release*/
        reg = REG_READ(SCTRL_CONTROL); 
        reg &= ~(0x1<<14);     
        REG_WRITE(SCTRL_CONTROL, reg); 
      
        /* MCLK(main clock) select:dpll*//* MCLK division*/
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
        reg |= ((MCLK_DIV_7 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);

        reg = REG_READ(SCTRL_CONTROL);    
        reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);    
        reg |= ((MCLK_FIELD_DPLL & MCLK_MUX_MASK) << MCLK_MUX_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);
	    PS_DEBUG_BCN_TRIGER;   

    }
    else
    {
        /* dpll division reset release*/
        reg = REG_READ(SCTRL_CONTROL); 
        reg &= ~(0x1<<14);     
        REG_WRITE(SCTRL_CONTROL, reg); 
	    PS_DEBUG_BCN_TRIGER;   
        
    }

    sctrl_mcu_ps_info.hw_sleep = 0;
    sctrl_cali_dpll(1);
    PS_DEBUG_BCN_TRIGER;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CLR_DPLL_UNLOOK_INT_BIT, NULL);

    /*open 32K Rosc calib*/
#if (CFG_SOC_NAME == SOC_BK7231)
    REG_WRITE(SCTRL_ROSC_CAL, 0x35);
    REG_WRITE(SCTRL_ROSC_CAL, 0x37);
#endif
    if(4 == flash_get_line_mode())
    {
        flash_set_line_mode(4);
    }
	PS_DEBUG_BCN_TRIGER;       
}

UINT8 sctrl_if_rf_sleep(void)
{
    uint32_t value;
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    value =  rf_sleeped;
    GLOBAL_INT_RESTORE();
    return value;
}
#if CFG_USE_STA_PS
void sctrl_rf_sleep(void)
{
    UINT32 reg;
    power_save_sleep_status_set();
#if (CFG_SOC_NAME == SOC_BK7231)
    REG_WRITE(SCTRL_ROSC_CAL, 0x35);
    REG_WRITE(SCTRL_ROSC_CAL, 0x37);
#endif

	/*Disable BK7011*/
    rc_cntl_stat_set(0x0);
    /* MAC AHB slave clock disable */
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~MAC_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);
    /* Mac Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MAC_CLK480M_PWD_BIT);
    //PS_DEBUG_CK_TRIGER;

	
    /* Modem AHB clock disable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~PHY_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);
    /* Modem Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MODEM_CLK480M_PWD_BIT);
	//PS_DEBUG_CK_TRIGER;


    reg = REG_READ(ICU_INTERRUPT_ENABLE);
    reg |= (CO_BIT(FIQ_MAC_WAKEUP));
    REG_WRITE(ICU_INTERRUPT_ENABLE, reg);


}
void sctrl_rf_wakeup(void)
{
    UINT32 reg;
    PS_DEBUG_UP_TRIGER;

    if(sctrl_mcu_ps_info.hw_sleep == 1)
    {
        //if rf add mcu up meanwhile
        os_printf("err, hw not up\r\n");
    }
    
    /* MAC AHB slave clock enable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg | MAC_HCLK_EN_BIT);
    
    /* Mac Subsystem clock 480m enable*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~MAC_CLK480M_PWD_BIT;
    REG_WRITE(SCTRL_CONTROL, reg);
	PS_DEBUG_UP_TRIGER;

    /* Modem AHB clock enable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg | PHY_HCLK_EN_BIT);
    
    /* Modem Subsystem clock 480m enable*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~MODEM_CLK480M_PWD_BIT;
    REG_WRITE(SCTRL_CONTROL, reg);
	PS_DEBUG_UP_TRIGER;

	/*Enable BK7011:rc_en,ch0_en*/
    rc_cntl_stat_set(0x09);
        
    reg = REG_READ(ICU_ARM_WAKEUP_EN); 
    reg &= ~(MAC_ARM_WAKEUP_EN_BIT);
	REG_WRITE(ICU_ARM_WAKEUP_EN, reg);      

}
#endif

#if CFG_USE_MCU_PS
void sctrl_mcu_sleep(UINT32 peri_clk)
{
    UINT32 reg;

    if((power_save_if_rf_sleep()) && (sctrl_mcu_ps_info.hw_sleep == 0))
    {
        reg = REG_READ(ICU_ARM_WAKEUP_EN);

        if(sctrl_mcu_ps_info.first_sleep == 1)
        {
            reg = 0x0;
            sctrl_mcu_ps_info.first_sleep = 0;
        }
#if (CHIP_U_MCU_WKUP_USE_TIMER && (CFG_SOC_NAME != SOC_BK7231))
        reg |= (TIMER_ARM_WAKEUP_EN_BIT | UART2_ARM_WAKEUP_EN_BIT
            #if (!PS_NO_USE_UART1_WAKE)
            | UART1_ARM_WAKEUP_EN_BIT
            #endif
            );
#else
        reg |= (PWM_ARM_WAKEUP_EN_BIT | UART2_ARM_WAKEUP_EN_BIT
            #if (!PS_NO_USE_UART1_WAKE)
            | UART1_ARM_WAKEUP_EN_BIT
            #endif
            );
#endif

    	REG_WRITE(ICU_ARM_WAKEUP_EN, reg);  

        sctrl_hw_sleep(peri_clk);     
    }
    else
    {
        PS_DEBUG_DOWN_TRIGER;
        delay(1);
#if PS_CLOSE_PERI_CLK
        /* close all peri clock*/
        ps_saves[0].peri_clk_cfg= REG_READ(ICU_PERI_CLK_PWD);        
        REG_WRITE(ICU_PERI_CLK_PWD, peri_clk);
#endif
        
        PS_DEBUG_DOWN_TRIGER;
#if (CFG_SOC_NAME == SOC_BK7231)
        REG_WRITE(SCTRL_ROSC_CAL, 0x35);
        REG_WRITE(SCTRL_ROSC_CAL, 0x37);
#endif
        WFI();              
    }
    delay(5);
}
UINT32 sctrl_mcu_wakeup(void)
{
    UINT32 reg;
    UINT32 wkup_type;

    if(sctrl_mcu_ps_info.hw_sleep == 1)
    {
        sctrl_hw_wakeup();

        #if PS_CLOSE_PERI_CLK
        REG_WRITE(ICU_PERI_CLK_PWD, ps_saves[0].peri_clk_cfg);
        #endif
        
        reg = REG_READ(ICU_ARM_WAKEUP_EN); 
        reg &= ~(PWM_ARM_WAKEUP_EN_BIT);
    	REG_WRITE(ICU_ARM_WAKEUP_EN, reg);   

        wkup_type = 1;
    }
    else
    {
    	PS_DEBUG_BCN_TRIGER;   
        delay(2);
    	PS_DEBUG_BCN_TRIGER;   
    
        #if PS_CLOSE_PERI_CLK
        /* recovery periphral unit clock config*/        
        REG_WRITE(ICU_PERI_CLK_PWD, ps_saves[0].peri_clk_cfg);  
        #endif
        wkup_type = 0;
     }
    return wkup_type;
}


void sctrl_mcu_init(void)
{
    UINT32 reg;

#if (USE_DCO_CLK_POWON )
    sctrl_mcu_ps_info.mcu_use_dco = 1;
#else
#if CFG_MCU_PS_SELECT_120M
    if(REG_READ(SCTRL_CONTROL) & (MCLK_MUX_MASK << MCLK_MUX_POSI))
    {
        ps_delay(5);
        /* MCLK(main clock) select:dco*/
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
        #if (CFG_SOC_NAME != SOC_BK7221U)
        reg |= ((MCLK_DIV_1 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
        #endif
        REG_WRITE(SCTRL_CONTROL, reg);       
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);
        sctrl_mcu_ps_info.mcu_use_dco = 1;
        ps_delay(5);
    }
#endif
#endif
}

void sctrl_mcu_exit(void)
{
    UINT32 reg;

#if (USE_DCO_CLK_POWON )
#else
#if CFG_MCU_PS_SELECT_120M
    /*config main clk*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
    #if CFG_SYS_REDUCE_NORMAL_POWER
    reg |= ((MCLK_DIV_7 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
    #else
    reg |= ((MCLK_DIV_3 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
    #endif
    reg |= ((MCLK_FIELD_DPLL & MCLK_MUX_MASK) << MCLK_MUX_POSI);
    REG_WRITE(SCTRL_CONTROL, reg);
    sctrl_mcu_ps_info.mcu_use_dco = 0;
#endif
#endif
}
#endif

void sctrl_subsys_power(UINT32 cmd)
{
    UINT32 reg = 0;
    UINT32 reg_val;
    UINT32 reg_word = 0;

    switch(cmd)
    {
    case CMD_SCTRL_DSP_POWERDOWN:
        reg = SCTRL_DSP_PWR;
        reg_word = DSP_PWD;
        break;

    case CMD_SCTRL_DSP_POWERUP:
        reg = SCTRL_DSP_PWR;
        reg_word = DSP_PWU;
        break;

    case CMD_SCTRL_USB_POWERDOWN:
        reg = SCTRL_USB_PWR;
        reg_val = REG_READ(SCTRL_USB_PWR);
        reg_val &= ~(USB_PWD_MASK << USB_PWD_POSI);
        reg_val |= USB_PWD << USB_PWD_POSI;
        reg_word = reg_val;
        break;

    case CMD_SCTRL_USB_POWERUP:
        reg = SCTRL_USB_PWR;
        reg_val = REG_READ(SCTRL_USB_PWR);
        reg_val &= ~(USB_PWD_MASK << USB_PWD_POSI);
        reg_val |= USB_PWU << USB_PWD_POSI;
        reg_word = reg_val;
        break;

    case CMD_SCTRL_MAC_POWERDOWN:
        reg = SCTRL_PWR_MAC_MODEM;
        reg_val = REG_READ(SCTRL_PWR_MAC_MODEM);
        reg_val &= ~(MAC_PWD_MASK << MAC_PWD_POSI);
        reg_val |= MAC_PWD << MAC_PWD_POSI;
        reg_word = reg_val;
        break;

    case CMD_SCTRL_MAC_POWERUP:
        reg = SCTRL_PWR_MAC_MODEM;
        reg_val = REG_READ(SCTRL_PWR_MAC_MODEM);
        reg_val &= ~(MAC_PWD_MASK << MAC_PWD_POSI);
        reg_val |= MAC_PWU << MAC_PWD_POSI;
        reg_word = reg_val;
        break;

    case CMD_SCTRL_MODEM_POWERDOWN:
        reg = SCTRL_PWR_MAC_MODEM;
        reg_val = REG_READ(SCTRL_PWR_MAC_MODEM);
        reg_val &= ~(MODEM_PWD_MASK << MODEM_PWD_POSI);
        reg_val |= MODEM_PWD << MODEM_PWD_POSI;
        reg_word = reg_val;
        break;

    case CMD_SCTRL_BLE_POWERDOWN:
        reg = SCTRL_USB_PWR;
        reg_val = REG_READ(SCTRL_USB_PWR);
        reg_val &= ~(BLE_PWD_MASK << BLE_PWD_POSI);
        reg_val |= BLE_PWD << BLE_PWD_POSI;
        reg_word = reg_val;
        break;        

    case CMD_SCTRL_MODEM_POWERUP:
        reg = SCTRL_PWR_MAC_MODEM;
        reg_val = REG_READ(SCTRL_PWR_MAC_MODEM);
        reg_val &= ~(MODEM_PWD_MASK << MODEM_PWD_POSI);
        reg_val |= MODEM_PWU << MODEM_PWD_POSI;
        reg_word = reg_val;
        break;

     case CMD_SCTRL_BLE_POWERUP:
        reg = SCTRL_USB_PWR;
        reg_val = REG_READ(SCTRL_USB_PWR);
        reg_val &= ~(BLE_PWD_MASK << BLE_PWD_POSI);
        reg_val |= BLE_PWU << BLE_PWD_POSI;
        reg_word = reg_val;
        break;         

    default:
        break;
    }

    if(reg)
    {
        REG_WRITE(reg, reg_word);
    }
}

void sctrl_subsys_reset(UINT32 cmd)
{
    UINT32 reg = 0;
    UINT32 reset_word = 0;

    switch(cmd)
    {
    case CMD_SCTRL_MODEM_SUBCHIP_RESET:
        reg = SCTRL_MODEM_SUBCHIP_RESET_REQ;
        reset_word = MODEM_SUBCHIP_RESET_WORD;
        break;

    case CMD_SCTRL_MAC_SUBSYS_RESET:
        reg = SCTRL_MAC_SUBSYS_RESET_REQ;
        reset_word = MAC_SUBSYS_RESET_WORD;
        break;

    case CMD_SCTRL_USB_SUBSYS_RESET:
        reg = SCTRL_USB_SUBSYS_RESET_REQ;
        reset_word = USB_SUBSYS_RESET_WORD;
        break;

    case CMD_SCTRL_DSP_SUBSYS_RESET:
        reg = SCTRL_DSP_SUBSYS_RESET_REQ;
        reset_word = DSP_SUBSYS_RESET_WORD;
        break;

    default:
        break;
    }

    if(reg)
    {
        REG_WRITE(reg, reset_word);
        delay(10);
        REG_WRITE(reg, 0);
    }

    return;
}

#if CFG_USE_DEEP_PS
void sctrl_enter_rtos_idle_sleep(UINT32 peri_clk)
{
    DD_HANDLE flash_hdl;
    UINT32 status;
    UINT32 param;
    UINT32 reg;
    int     i;

	uart_wait_tx_over();
	
    /* close all peri int*/
    ps_saves[1].int_enable_cfg = REG_READ(ICU_INTERRUPT_ENABLE);
    REG_WRITE(ICU_INTERRUPT_ENABLE, 0);
    /* MAC AHB slave clock disable */
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~MAC_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);

    /* Mac Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MAC_CLK480M_PWD_BIT);

    /* Modem pwd*/
    REG_WRITE(SCTRL_PWR_MAC_MODEM, MODEM_PWD); 
	
    /* Modem AHB clock disable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~PHY_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);
    
    /* Modem Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MODEM_CLK480M_PWD_BIT);
	
    /* Flash 26MHz clock select dco clock*/
    flash_hdl = ddev_open(FLASH_DEV_NAME, &status, 0);
    ASSERT(DD_HANDLE_UNVALID != flash_hdl);
    ddev_control(flash_hdl, CMD_FLASH_SET_DCO, 0);
    
    /* MCLK(main clock) select:dco*/ /* MCLK division*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
    reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    REG_WRITE(SCTRL_CONTROL, reg);    

    /*close 32K Rosc calib*/
    REG_WRITE(SCTRL_ROSC_CAL, 0x36);
	
    /* dpll division reset*/
    reg = REG_READ(SCTRL_CONTROL); 
    reg |= (0x1<<14);     
    REG_WRITE(SCTRL_CONTROL, reg); 

    /* dpll (480m) & 26m xtal & xtal2rf & Vsys disable*/
    reg = REG_READ(SCTRL_BLOCK_EN_CFG);
    reg &= ~(BLOCK_EN_WORD_MASK << BLOCK_EN_WORD_POSI);
    reg = reg | (BLOCK_EN_WORD_PWD << BLOCK_EN_WORD_POSI);
    reg &= ~(BLK_EN_DPLL_480M | BLK_EN_26M_XTAL | BLK_EN_XTAL2RF | BLK_EN_ANALOG_SYS_LDO);
    REG_WRITE(SCTRL_BLOCK_EN_CFG, reg); 
    reg = REG_READ(SCTRL_BLOCK_EN_MUX);
    reg = reg | (0x1 << 4);
    REG_WRITE(SCTRL_BLOCK_EN_MUX, reg);
	
    /* close all peri clock*/
    ps_saves[1].peri_clk_cfg= REG_READ(ICU_PERI_CLK_PWD);
    REG_WRITE(ICU_PERI_CLK_PWD, peri_clk);

    /* vdddig decrease to 0.9v*/
    REG_WRITE(SCTRL_DIGTAL_VDD, 0x20);
	
    /* set all gpio to high-z state*/
    for(i =2; i < 32; i ++)
    {
        ps_saves_gpio_cfgs[i] = REG_READ(0x00802800+i*4);
        REG_WRITE(0x00802800+i*4, 0x2c);
    }
    
    /* arm clock disable */
    reg = REG_READ(SCTRL_SLEEP);
    reg &= ~(SLEEP_MODE_MASK << SLEEP_MODE_POSI);
    reg = reg | SLEEP_MODE_CFG_NORMAL_VOL_WORD;
    REG_WRITE(SCTRL_SLEEP, reg);  
    delay(5);
}

void sctrl_exit_rtos_idle_sleep(void)
{
    UINT32 reg;
    int    i;

    /* vdddig increase to 1.2v*/
    REG_WRITE(SCTRL_DIGTAL_VDD, 0x40);
    PS_DEBUG_UP_OUT;

    /*dpll(480m) & 26m xtal & xtal2rf enable*/
    reg = REG_READ(SCTRL_BLOCK_EN_MUX);
    reg &= ~(0x1 << 4);
    REG_WRITE(SCTRL_BLOCK_EN_MUX, reg);
    reg = REG_READ(SCTRL_BLOCK_EN_CFG);
    reg &= ~(BLOCK_EN_WORD_MASK << BLOCK_EN_WORD_POSI);
    reg |= (BLOCK_EN_WORD_PWD << BLOCK_EN_WORD_POSI);
    reg |= (BLK_EN_26M_XTAL | BLK_EN_DPLL_480M | BLK_EN_XTAL2RF | BLK_EN_ANALOG_SYS_LDO);
    REG_WRITE(SCTRL_BLOCK_EN_CFG, reg); 	
	
    delay(1);  //delay 1.1ms for 26MHz DCO clock. need change for other dco clock frequency
    sctrl_cali_dpll(0);
    /* dpll division reset release*/
    reg = REG_READ(SCTRL_CONTROL); 
    reg &= ~(0x1<<14);     
    REG_WRITE(SCTRL_CONTROL, reg); 
    
    /* MCLK(main clock) select:dpll*//* MCLK division*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);    
    reg |= ((MCLK_DIV_7 & MCLK_DIV_MASK) << MCLK_DIV_POSI);
    reg |= ((MCLK_FIELD_DPLL & MCLK_MUX_MASK) << MCLK_MUX_POSI);
    REG_WRITE(SCTRL_CONTROL, reg);

    /* Flash clock select dpll clock*/ 
    {
        DD_HANDLE flash_hdl;
        UINT32 status;
        flash_hdl = ddev_open(FLASH_DEV_NAME, &status, 0);
        ASSERT(DD_HANDLE_UNVALID != flash_hdl);
        ddev_control(flash_hdl, CMD_FLASH_SET_DPLL, 0);
    }

    /*open 32K Rosc calib*/
    REG_WRITE(SCTRL_ROSC_CAL, 0x37);
	
    /* MAC AHB slave clock enable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg | MAC_HCLK_EN_BIT);
    
    /* Mac Subsystem clock 480m enable*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~MAC_CLK480M_PWD_BIT;
    REG_WRITE(SCTRL_CONTROL, reg);

    /* Modem AHB clock enable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg | PHY_HCLK_EN_BIT);
    
    /* Modem Subsystem clock 480m enable*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~MODEM_CLK480M_PWD_BIT;
    REG_WRITE(SCTRL_CONTROL, reg);

    /* Modem pwd*/
    reg = REG_READ(SCTRL_PWR_MAC_MODEM);
    reg &= ~(MODEM_PWD_MASK << MODEM_PWD_POSI);
    reg = reg | (0 << MODEM_PWD_POSI);
    REG_WRITE(SCTRL_PWR_MAC_MODEM, reg);  
	
    /* recovery periphral unit clock config*/
    REG_WRITE(ICU_PERI_CLK_PWD, ps_saves[1].peri_clk_cfg);

    /* recovery periphral unit int config*/
    REG_WRITE(ICU_INTERRUPT_ENABLE, ps_saves[1].int_enable_cfg);  
    /* recovery gpio configuration*/
    for(i = 2; i < 32; i ++)
    {
        if(i!=10&&i!=11&&i!=14&&i!=15&&i!=17)
        REG_WRITE(0x00802800+i*4, ps_saves_gpio_cfgs[i]);
        else if(i!=16 && i!=10)
        REG_WRITE(0x00802800+i*4, 0x0);
    }
    
    os_printf("idle wake up!\r\n");    
}

void sctrl_enter_rtos_deep_sleep(PS_DEEP_CTRL_PARAM *deep_param)
{
    DD_HANDLE flash_hdl;
    UINT32 status;
    UINT32 param;
    UINT32 reg;
    UINT32 i;
	
	uart_wait_tx_over();
	
	/* close all peri clock*/
	REG_WRITE(ICU_PERI_CLK_PWD, 0xfffff);  //  icu: 0x2;
      
	uart2_exit();
	uart1_exit();

	/*ana_reg set*/
    REG_WRITE(SCTRL_ANALOG_CTRL0, 0x7819a59b);     
    REG_WRITE(SCTRL_ANALOG_CTRL1, 0x7819a59b);     
    REG_WRITE(SCTRL_ANALOG_CTRL2, 0x84036080);     
    REG_WRITE(SCTRL_ANALOG_CTRL3, 0x180004a0);     
    REG_WRITE(SCTRL_ANALOG_CTRL4, 0x84200e52);     
    REG_WRITE(SCTRL_ANALOG_CTRL5, 0x3b13b13b);     
    REG_WRITE(SCTRL_ANALOG_CTRL6, 0xb09350);     
    REG_WRITE(SCTRL_ANALOG_CTRL7, 0x441a7f0);     
    REG_WRITE(SCTRL_ANALOG_CTRL8, 0x3b187c);     
    REG_WRITE(SCTRL_ANALOG_CTRL9, 0x82204007);     
    REG_WRITE(SCTRL_ANALOG_CTRL10, 0x80801432);   	
    ps_delay(10);	

	/*clear int*/
	REG_WRITE(ICU_INTERRUPT_ENABLE, 0);
	
	/*  disable gpio0~31*/
    REG_WRITE(SCTRL_GPIO_WAKEUP_EN,0x0);  //sys_ctrl : 0x48;
    REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS,0xFFFFFFFF);  //sys_ctrl : 0x4a; 
    
	/*	disable gpio32~39*/
	REG_WRITE(SCTRL_GPIO_WAKEUP_EN1,0x0);  //sys_ctrl : 0x51;
	REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS1,0xFF);  //sys_ctrl : 0x53; 

    REG_WRITE(SCTRL_BLOCK_EN_MUX, 0x0);   //sys_ctrl : 0x4F;
    

	/* ROSC_POWER DOWN*/
	reg = REG_READ(SCTRL_SLEEP);
	reg = reg| ROSC_PWD_DEEPSLEEP_BIT ;
	REG_WRITE(SCTRL_SLEEP,reg); 
    
    /* ROSC_TIMER_int_clear*/
    reg = REG_READ(SCTRL_ROSC_TIMER);
   	reg = reg| ROSC_TIMER_INT_STATUS_BIT ;
    REG_WRITE(SCTRL_ROSC_TIMER,reg);      //sys_ctrl : 0x47;
    
	/*ROSC_TIMER close */
	reg = REG_READ(SCTRL_ROSC_TIMER);
	reg = reg & (~ROSC_TIMER_ENABLE_BIT);																   //'C'
	REG_WRITE(SCTRL_ROSC_TIMER,reg); 
	
    reg = REG_READ(SCTRL_LOW_PWR_CLK);
    reg &=~(LPO_CLK_MUX_MASK);
    //reg |=(LPO_SRC_ROSC << LPO_CLK_MUX_POSI);
    reg |=(LPO_SRC_32K_XTAL << LPO_CLK_MUX_POSI);
    REG_WRITE(SCTRL_LOW_PWR_CLK, reg);    //sys_ctrl : 0x40;

    /* close all peri int*/
	//    REG_WRITE(ICU_INTERRUPT_ENABLE, 0);

    /* MAC pwd*/
    REG_WRITE(SCTRL_PWR_MAC_MODEM, MAC_PWD << MAC_PWD_POSI);   //sys_ctrl : 0x43;
    
    /* MAC AHB slave clock disable */
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~MAC_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);

    /* Mac Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MAC_CLK480M_PWD_BIT);

    /* Modem pwd*/
    REG_WRITE(SCTRL_PWR_MAC_MODEM, MODEM_PWD << MODEM_PWD_POSI); 
	
    /* Modem AHB clock disable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~PHY_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);
    
    /* Modem Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MODEM_CLK480M_PWD_BIT);
	
    /* Flash 26MHz clock select dco clock*/
    flash_hdl = ddev_open(FLASH_DEV_NAME, &status, 0);
    ASSERT(DD_HANDLE_UNVALID != flash_hdl);
    ddev_control(flash_hdl, CMD_FLASH_SET_DCO, 0);
    
    /* MCLK(main clock) select:dco*/ /* MCLK division*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
    reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    REG_WRITE(SCTRL_CONTROL, reg);                                     //0x02
    
    reg = REG_READ(SCTRL_CONTROL);
    reg =(reg & (~0xF0) |(0<<4));
    reg =(reg & (~0x03) |(0<<MCLK_MUX_POSI));
    reg =(reg & (~0x100) |FLASH_26M_MUX_BIT);
    REG_WRITE(SCTRL_CONTROL,reg); //sys_ctrl : 0x02;

    ps_delay(10);
    if (((deep_param->wake_up_way == PS_DEEP_WAKEUP_RTC) || (deep_param->wake_up_way & PS_DEEP_WAKEUP_GPIO_RTC))
		&& (deep_param->sleep_time!= 0xffffffff))
    {
	/*ROSC_TIMER  init*/
#if (CFG_SOC_NAME == SOC_BK7221U)
		reg = (deep_param->sleep_time >> 16)& 0xffff;                                          //'A'
		REG_WRITE(SCTRL_ROSC_TIMER_PERIOD_HIGH,reg);
#endif

        reg = REG_READ(SCTRL_SLEEP);
        reg &= reg & (~ROSC_PWD_DEEPSLEEP_BIT);
        REG_WRITE(SCTRL_SLEEP, reg);  

        reg = REG_READ(SCTRL_ROSC_TIMER);
        reg |= ROSC_TIMER_INT_STATUS_BIT;                                                                   //'C'
        REG_WRITE(SCTRL_ROSC_TIMER,reg);  //sys_ctrl : 0x47;
		
        reg = REG_READ(SCTRL_ROSC_TIMER);
        reg &= ~(ROSC_TIMER_PERIOD_MASK << ROSC_TIMER_PERIOD_POSI);
        reg |= ((deep_param->sleep_time & ROSC_TIMER_PERIOD_MASK) << ROSC_TIMER_PERIOD_POSI);
        REG_WRITE(SCTRL_ROSC_TIMER,reg);   //sys_ctrl : 0x47;                         //'D'
		
        reg = REG_READ(SCTRL_ROSC_TIMER);
        reg |= ROSC_TIMER_ENABLE_BIT;                                                                              
        REG_WRITE(SCTRL_ROSC_TIMER,reg);  //sys_ctrl : 0x47;                             //'B'
    
        reg = 0x0;
       // reg &= ~(BLOCK_EN_WORD_MASK << BLOCK_EN_WORD_POSI);
        //reg = reg | (BLOCK_EN_WORD_PWD << BLOCK_EN_WORD_POSI);
       // reg |= (BLK_EN_ANALOG_SYS_LDO | BLK_EN_DIGITAL_CORE);
       	reg = (reg &(~(BLOCK_EN_WORD_MASK << 20))&(~(0x7FFFUL<<5)) &(~(0x01UL<<1)));
       	reg = (reg |(BLOCK_EN_WORD_PWD<< 20 )|BLK_EN_FLASH|BLK_EN_ROSC32K|BLK_EN_26M_XTAL|BLK_EN_32K_XTAL|BLK_EN_DIGITAL_CORE|BLK_EN_ANALOG_SYS_LDO);
        REG_WRITE(SCTRL_BLOCK_EN_CFG, reg);                  //sys_ctrl : 0x4B;                   //'E'
        
        reg = REG_READ(SCTRL_ROSC_CAL);                           //ROSC Calibration disable
        reg =(reg  & (~0x01));
        REG_WRITE(SCTRL_ROSC_CAL, reg);   
		
		REG_WRITE(SCTRL_CONTROL, 0x330100);
        REG_WRITE(SCTRL_BLOCK_EN_CFG, (0x15D|(0xA5C<<20)));
        REG_WRITE(SCTRL_ROSC_CAL, 0x30);
        REG_WRITE(SCTRL_LOW_PWR_CLK, 0x01);
        REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, 0x03);
        REG_WRITE(SCTRL_CLK_GATING, 0x1ff);
    }

	if ((deep_param->wake_up_way == PS_DEEP_WAKEUP_GPIO) || (deep_param->wake_up_way & PS_DEEP_WAKEUP_GPIO_RTC))
    {
		for(i=0; i<GPIONUM; i++)
		{
			param = GPIO_CFG_PARAM(i, GMODE_SET_HIGH_IMPENDANCE);	/*set gpio 0~39 as high impendance*/
            sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param); 
		}

		for (i = 0; i < GPIONUM; i++)
    	{		
        	if (deep_param->gpio_index_map & (0x01UL << i))			/*set gpio 0~31 mode*/
        	{
            	if(deep_param->gpio_index_map & deep_param->gpio_edge_map & (0x01UL << i))
            	{
            		param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLUP);
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                    if(0x1 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &i))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        deep_param->gpio_edge_map &= ~(0x01UL << i);
                    }
            	}
            	else
            	{
                	param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLDOWN);
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                    if(0x0 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &i))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        deep_param->gpio_edge_map |= (0x01UL << i);
                    }
            	}
        	}
			
			if (deep_param->gpio_last_index_map & (0x01UL << i))				/*set gpio 32~39 mode*/
        	{	
            	if(deep_param->gpio_last_index_map & deep_param->gpio_last_edge_map  & (0x01UL << i))
            	{
            		param = GPIO_CFG_PARAM(i+32, GMODE_INPUT_PULLUP);
            		sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param); 
                    reg = i+32;
                    if(0x1 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &reg))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        deep_param->gpio_last_edge_map &= ~(0x01UL << i);
                    }
            	}
            	else
            	{
                	param = GPIO_CFG_PARAM(i+32, GMODE_INPUT_PULLDOWN);
                	sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);  
                    reg = i+32;
                    if(0x0 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &reg))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        deep_param->gpio_last_edge_map |= (0x01UL << i);
                    }
            	}
        	}			
    	}
		/* set gpio 0~31 mode*/
        reg = 0xFFFFFFFF;
        REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS,reg);        
        reg = deep_param->gpio_edge_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_TYPE,reg);
        reg = deep_param->gpio_index_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_EN,reg); 
		
		/* set gpio 31~32 mode*/
        reg = 0xFF;
        REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS1,reg);        
        reg = deep_param->gpio_last_edge_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_TYPE1,reg);
        reg = deep_param->gpio_last_index_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_EN1,reg); 	
    }	
   
#ifdef BK_DEEP_SLEEP_DEBUG
	BK_DEEP_SLEEP_PRT("SCTRL_CONTROL=0x%08X\r\n", REG_READ(SCTRL_CONTROL)); 
	BK_DEEP_SLEEP_PRT("SCTRL_SLEEP=0x%08X\r\n", REG_READ(SCTRL_SLEEP)); 
	BK_DEEP_SLEEP_PRT("SCTRL_ROSC_TIMER=0x%08X\r\n", REG_READ(SCTRL_ROSC_TIMER));
	BK_DEEP_SLEEP_PRT("SCTRL_BLOCK_EN_CFG=0x%08X\r\n", REG_READ(SCTRL_BLOCK_EN_CFG));
	BK_DEEP_SLEEP_PRT("SCTRL_ROSC_CAL=0x%08X\r\n", REG_READ(SCTRL_ROSC_CAL));
	BK_DEEP_SLEEP_PRT("SCTRL_BLOCK_EN_MUX=0x%08X\r\n", REG_READ(SCTRL_BLOCK_EN_MUX));
	BK_DEEP_SLEEP_PRT("SCTRL_LOW_PWR_CLK=0x%08X\r\n", REG_READ(SCTRL_LOW_PWR_CLK));
	BK_DEEP_SLEEP_PRT("SCTRL_PWR_MAC_MODEM=0x%08X\r\n", REG_READ(SCTRL_PWR_MAC_MODEM));
	BK_DEEP_SLEEP_PRT("SCTRL_MODEM_CORE_RESET_PHY_HCLK=0x%08X\r\n", REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK));
	BK_DEEP_SLEEP_PRT("SCTRL_CLK_GATING=0x%08X\r\n", REG_READ(SCTRL_CLK_GATING));
	BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_INT_STATUS=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_INT_STATUS));
	BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_TYPE=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_TYPE));
	BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_EN=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_EN));
	BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_INT_STATUS1=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_INT_STATUS1));
	BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_TYPE1=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_TYPE1));
	BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_EN1=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_EN1));	
#endif      

    /* enter deep_sleep mode */
    reg = REG_READ(SCTRL_SLEEP);
    reg &= ~(SLEEP_MODE_MASK << SLEEP_MODE_POSI);
    reg = reg | SLEEP_MODE_CFG_DEEP_WORD&(~ROSC_PWD_DEEPSLEEP_BIT);
    REG_WRITE(SCTRL_SLEEP, reg);  
  
    delay(5);
}
#endif

#if (CFG_SOC_NAME != SOC_BK7231)
static int sctrl_read_efuse(void *param)
{
    UINT32 reg, ret = -1;
    EFUSE_OPER_PTR efuse;
    efuse = (EFUSE_OPER_PTR)param;
    
    if(efuse) {
        reg = REG_READ(SCTRL_EFUSE_CTRL);
        reg &= ~(EFUSE_OPER_ADDR_MASK << EFUSE_OPER_ADDR_POSI);
        reg &= ~(EFUSE_OPER_DIR);            
      
        reg |= ((efuse->addr & EFUSE_OPER_ADDR_MASK) << EFUSE_OPER_ADDR_POSI);
        reg |= (EFUSE_OPER_EN);
        REG_WRITE(SCTRL_EFUSE_CTRL, reg);

        do {
            reg = REG_READ(SCTRL_EFUSE_CTRL);
        }while(reg & EFUSE_OPER_EN);

        reg = REG_READ(SCTRL_EFUSE_OPTR);
        if(reg & EFUSE_OPER_RD_DATA_VALID) {
            efuse->data = ((reg >> EFUSE_OPER_RD_DATA_POSI) & EFUSE_OPER_RD_DATA_MASK);
            ret = 0;
        } else {
            efuse->data = 0xFF;
        }
    }
    return ret;
}

static int sctrl_write_efuse(void *param)
{
    UINT32 reg, ret = -1;
    EFUSE_OPER_ST *efuse, efuse_bak;
    
    efuse = (EFUSE_OPER_PTR)param;
    if(efuse) {
        efuse_bak.addr = efuse->addr;
        efuse_bak.data = efuse->data;        
        if(sctrl_read_efuse(&efuse_bak) == 0) {
             //read before write, ensure this byte no wrote
             if(EFUSE_INIT_VAL != efuse_bak.data)
                return -1; 
        }      

        // enable vdd2.5v first
        reg = REG_READ(SCTRL_CONTROL);
        reg |= EFUSE_VDD25_EN;
        REG_WRITE(SCTRL_CONTROL, reg);
        
        reg = REG_READ(SCTRL_EFUSE_CTRL);
        reg &= ~(EFUSE_OPER_ADDR_MASK << EFUSE_OPER_ADDR_POSI);
        reg &= ~(EFUSE_OPER_WR_DATA_MASK << EFUSE_OPER_WR_DATA_POSI);
        
        reg |= EFUSE_OPER_DIR;
        reg |= ((efuse->addr & EFUSE_OPER_ADDR_MASK) << EFUSE_OPER_ADDR_POSI);            
        reg |= ((efuse->data & EFUSE_OPER_WR_DATA_MASK) << EFUSE_OPER_WR_DATA_POSI);
        reg |= EFUSE_OPER_EN;
        REG_WRITE(SCTRL_EFUSE_CTRL, reg);

        do {
            reg = REG_READ(SCTRL_EFUSE_CTRL);
        }while(reg & EFUSE_OPER_EN);

        // disable vdd2.5v at last
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~EFUSE_VDD25_EN;
        REG_WRITE(SCTRL_CONTROL, reg);

        // check, so read
        reg = efuse->data;
        efuse->data = 0;
        if(sctrl_read_efuse(param) == 0) {
            if(((UINT8)reg) == efuse->data)
               ret = 0; 
        }        
    }
    return ret;
}
#endif // (CFG_SOC_NAME != SOC_BK7231)

#if CFG_USE_USB_CHARGE
#if (CFG_SOC_NAME == SOC_BK7221U)
UINT32 usb_is_pluged(void)
{
    UINT32 reg;
    reg = sctrl_analog_get(SCTRL_CHARGE_STATUS);
    return (reg & (1 << 21));
}

void charger_module_enable(UINT32 enable)
{
    if(! usb_is_pluged())
    {
        return;
    }

    sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4) & ~(1 << 12)) | (!!enable << 12));
}

void charger_vlcf_calibration(UINT32 type)
{
    if(! usb_is_pluged())
    {
        return;
    }
    if(type == 0)
    {
        /*Internal hardware calibration*/
        /*vlcf calibration*/
        /*>>> Added 4.2V voltage on vbattery*/
        /*>>> Set pwd=0*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 12));
        /*calEn*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 21));
        /*softCalen*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(1 << 20));
        /*vlcfSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 18));
        /*IcalSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 17));
        /*vcvSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 16));
        /*vlcf_caltrig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 21));
        /*vlcf_caltrig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 21));
        /*Wait for at least 4 clock cycles*/
        delay_ms(1);
        /*vlcf_caltrig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 21));
        /*Waiting for 1ms, calibration finished*/
        delay_ms(1);
        /*Read the value vcal<5:0>, Recorded*/
        calib_charger[0] = (sctrl_analog_get(SCTRL_CHARGE_STATUS) >> CHARGE_VCAL_POS) & CHARGE_VCAL_MASK ;

        os_printf("calib_charger[0] = 0x%02x\r\n", calib_charger[0]);
    }
    else
    {
        /*External software calibration*/
        /*TODO*/
    }
}

void charger_icp_calibration(UINT32 type)
{
    if(! usb_is_pluged())
    {
        return;
    }
    if(type == 0)
    {
        /*Internal hardware calibration*/
        /*Icp calibration*/
        /*>>> Added parallel 60ohm resistor and 100nF capacitor from vbattery to ground.(Removed the external 4.2V)*/
        /*>>> Set pwd=0*/
        /*>>> Porb=0*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 12));
        /*Icp=60mA*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)\
                                              & ~(CHARGE_LCP_MASK << CHARGE_LCP_POS)) | (0x4 << CHARGE_LCP_POS));
        /*calEn*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 21));
        /*softCalen*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(1 << 20));
        /*vlcfSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 18));
        /*vcal<5:0>=previous vlcf calibration value*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)\
                                              & ~(CHARGE_VCAL_MASK << 0)) | (calib_charger[0] << 0));
        /*IcalSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 17));
        /*vcvSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 16));
        /*Ical_trig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 20));
        /*Ical_trig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 20));
        /*Wait for at least 4 clock cycles*/
        delay_ms(1);
        /*Ical_trig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 20));
        /*Waiting for 1ms, calibration finished*/
        delay_ms(1);
        /*Read the value Ical<4:0>, Recorded*/
        calib_charger[1] = (sctrl_analog_get(SCTRL_CHARGE_STATUS) >> CHARGE_LCAL_POS) & CHARGE_LCAL_MASK ;

        os_printf("calib_charger[1] = 0x%02x\r\n", calib_charger[1]);
    }
    else
    {
        /*External software calibration*/
        /*TODO*/
    }
}

void charger_vcv_calibration(UINT32 type)
{
    if(! usb_is_pluged())
    {
        return;
    }
    if(type == 0)
    {
        /*Internal hardware calibration*/
        /*vcv calibration*/
        /*>>> Added 4.2V voltage on vbattery*/
        /*>>> Set pwd=0*/
        /*>>> Porb=0*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 12));
        /*Icp=60mA*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)\
                                              & ~(CHARGE_LCP_MASK << CHARGE_LCP_POS)) | (0x4 << CHARGE_LCP_POS));
        /*calEn*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 21));
        /*softCalen*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(1 << 20));
        /*vlcfSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 18));
        /*vcal<5:0>=previous vlcf calibration value*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)\
                                              & ~(CHARGE_VCAL_MASK << 0)) | (calib_charger[0] << 0));
        /*IcalSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 17));
        /*Ical<4:0>=previous Ical calibration value*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4)\
                                              & ~(CHARGE_LCAL_MASK << 27)) | (calib_charger[1] << 27));
        /*vcvSel*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 16));
        /*vcv_caltrig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 19));
        /*vcv_caltrig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 19));
        /*Wait for at least 4 clock cycles*/
        delay_ms(1);
        /*vcv_caltrig*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(1 << 19));
        /*Waiting for 1ms, calibration finished*/
        delay_ms(1);
        /*Read the value vcvcal<4:0>, Recorded*/
        calib_charger[2] = (sctrl_analog_get(SCTRL_CHARGE_STATUS) >> CHARGE_VCVCAL_POS) & CHARGE_VCVCAL_MASK ;

        os_printf("calib_charger[2] = 0x%02x\r\n", calib_charger[2]);
    }
    else
    {
        /*External software calibration*/
        /*TODO*/
    }
}

void charger_calib_get(UINT8 value[])
{
    value[0] = calib_charger[0];
    value[1] = calib_charger[1];
    value[2] = calib_charger[2];
    return;
}

void charger_calib_set(UINT8 value[])
{
    if(!value[0] || !value[1] || !value[2])
    {
        return;
    }

    calib_charger[0] = value[0];
    calib_charger[1] = value[1];
    calib_charger[2] = value[2];
    return;
}

UINT32 charger_is_full(void)
{
    UINT32 reg;
    reg = sctrl_analog_get(SCTRL_CHARGE_STATUS);
    return (reg & (1 << 20));
}

void charger_start(void * param)
{
    UINT32 charge_cal_type ;
    CHARGE_OPER_ST *chrg;

    chrg = (CHARGE_OPER_ST *)param;

    if(! usb_is_pluged())
    {
        return;
    }

    charger_calib_set(chrg->cal);
    os_printf("%s(%d) %x %x %x %x\r\n", __FUNCTION__, chrg->type,chrg->oper, calib_charger[0], calib_charger[1], calib_charger[2]);

    if(chrg->type == 0)
    {
        /*Internal*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(1 << 21));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 27));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 12));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(1 << 11));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 18));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 17));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 16));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(CHARGE_LCP_MASK << CHARGE_LCP_POS)) | (chrg->oper << CHARGE_LCP_POS));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(CHARGE_VCAL_MASK << 0)) | (calib_charger[0] << 0));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(CHARGE_LCAL_MASK << 27)) | (calib_charger[1] << 27));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(CHARGE_LCAL_MASK << 22)) | (calib_charger[2] << 22));

    }
    else
    {
        /*External*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 27));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 12));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, sctrl_analog_get(SCTRL_ANALOG_CTRL4) | (1 << 11));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 18));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 17));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, sctrl_analog_get(SCTRL_ANALOG_CTRL3) | (1 << 16));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(CHARGE_LCP_MASK << CHARGE_LCP_POS)) | (chrg->oper << CHARGE_LCP_POS));
        sctrl_analog_set(SCTRL_ANALOG_CTRL3, (sctrl_analog_get(SCTRL_ANALOG_CTRL3)& ~(CHARGE_VCAL_MASK << 0)) | (calib_charger[0] << 0));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(CHARGE_LCAL_MASK << 27)) | (calib_charger[1] << 27));
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4)& ~(CHARGE_LCAL_MASK << 22)) | (calib_charger[2] << 22));
    }
}

void charger_stop(UINT32 type)
{
    os_printf("%s(%d)\r\n", __FUNCTION__, type);

    charger_module_enable(0);
    if(type == 0)
    {
        /*Internal*/
    }
    else
    {
        /*External*/
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, (sctrl_analog_get(SCTRL_ANALOG_CTRL4) & ~(1 << 11)));
    }
}
#endif
#endif


UINT32 sctrl_ctrl(UINT32 cmd, void *param)
{
    UINT32 ret;
    UINT32 reg;

    ret = SCTRL_SUCCESS;
    switch(cmd)
    {
#if PS_SUPPORT_MANUAL_SLEEP
    case CMD_SCTRL_NORMAL_SLEEP:
        break;

    case CMD_SCTRL_NORMAL_WAKEUP:
        break;
#endif
#if CFG_USE_DEEP_PS
    case CMD_SCTRL_RTOS_IDLE_SLEEP:
        sctrl_enter_rtos_idle_sleep(*(UINT32 *)param);
        break;

    case CMD_SCTRL_RTOS_IDLE_WAKEUP:
        sctrl_exit_rtos_idle_sleep();
        break;


    case CMD_SCTRL_RTOS_DEEP_SLEEP:
        sctrl_enter_rtos_deep_sleep((PS_DEEP_CTRL_PARAM *)param);
        break;
#endif

    case CMD_GET_CHIP_ID:
        ret = REG_READ(SCTRL_CHIP_ID);
        break;

    case CMD_SCTRL_SET_FLASH_DPLL:
        reg = REG_READ(SCTRL_CONTROL);
        reg |= FLASH_26M_MUX_BIT;
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_SCTRL_SET_FLASH_DCO:
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~FLASH_26M_MUX_BIT;
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_SCTRL_DSP_POWERDOWN:
    case CMD_SCTRL_USB_POWERDOWN:
    case CMD_SCTRL_MODEM_POWERDOWN:
    case CMD_SCTRL_MAC_POWERDOWN:
    case CMD_SCTRL_DSP_POWERUP:
    case CMD_SCTRL_USB_POWERUP:
    case CMD_SCTRL_MAC_POWERUP:
    case CMD_SCTRL_MODEM_POWERUP:
    case CMD_SCTRL_BLE_POWERDOWN:
    case CMD_SCTRL_BLE_POWERUP:
        sctrl_subsys_power(cmd);
        break;

    case CMD_GET_DEVICE_ID:
        ret = REG_READ(SCTRL_DEVICE_ID);
        break;

    case CMD_GET_SCTRL_CONTROL:
        *((UINT32 *)param) = REG_READ(SCTRL_CONTROL);
        break;

    case CMD_SET_SCTRL_CONTROL:
        REG_WRITE(SCTRL_CONTROL, *((UINT32 *)param));
        break;

    case CMD_SCTRL_MCLK_SELECT:
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
        reg |= ((*(UINT32 *)param) & MCLK_MUX_MASK) << MCLK_MUX_POSI;
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_SCTRL_MCLK_DIVISION:
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
        reg |= ((*(UINT32 *)param) & MCLK_DIV_MASK) << MCLK_DIV_POSI;
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_SCTRL_RESET_SET:
        reg = REG_READ(SCTRL_RESET);
        reg |= ((*(UINT32 *)param) & SCTRL_RESET_MASK);
        REG_WRITE(SCTRL_RESET, reg);
        break;

    case CMD_SCTRL_RESET_CLR:
        reg = REG_READ(SCTRL_RESET);
        reg &= ~((*(UINT32 *)param) & SCTRL_RESET_MASK);
        REG_WRITE(SCTRL_RESET, reg);
        break;

    case CMD_SCTRL_MODEM_SUBCHIP_RESET:
    case CMD_SCTRL_MAC_SUBSYS_RESET:
    case CMD_SCTRL_USB_SUBSYS_RESET:
    case CMD_SCTRL_DSP_SUBSYS_RESET:
        sctrl_subsys_reset(cmd);
        break;

    case CMD_SCTRL_MODEM_CORE_RESET:
        ret = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
        ret = ret & (~((MODEM_CORE_RESET_MASK) << MODEM_CORE_RESET_POSI));
        reg = ret | ((MODEM_CORE_RESET_WORD & MODEM_CORE_RESET_MASK)
                     << MODEM_CORE_RESET_POSI);
        REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);

        delay(1);
        reg = ret;
        REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);

        /*resetting, and waiting for done*/
        reg = REG_READ(SCTRL_RESET);
        while(reg & MODEM_CORE_RESET_BIT)
        {
            delay(10);
            reg = REG_READ(SCTRL_RESET);
        }
        ret = SCTRL_SUCCESS;
        break;

    case CMD_SCTRL_MPIF_CLK_INVERT:
        reg = REG_READ(SCTRL_CONTROL);
        reg |= MPIF_CLK_INVERT_BIT;
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_SCTRL_BLK_ENABLE:
        reg = REG_READ(SCTRL_BLOCK_EN_CFG);
        reg &= (~(BLOCK_EN_WORD_MASK << BLOCK_EN_WORD_POSI));
        reg |= (BLOCK_EN_WORD_PWD & BLOCK_EN_WORD_MASK) << BLOCK_EN_WORD_POSI;
        reg |= ((*(UINT32 *)param) & BLOCK_EN_VALID_MASK);
        REG_WRITE(SCTRL_BLOCK_EN_CFG, reg);
        break;

    case CMD_SCTRL_BLK_DISABLE:
        CHECK_OPERATE_RF_REG_IF_IN_SLEEP();
        reg = REG_READ(SCTRL_BLOCK_EN_CFG);
        reg &= ~((*(UINT32 *)param) & BLOCK_EN_VALID_MASK);
        REG_WRITE(SCTRL_BLOCK_EN_CFG, reg);
        CHECK_OPERATE_RF_REG_IF_IN_SLEEP_END();
        break;

    case CMD_SCTRL_BIAS_REG_SET:
        reg = REG_READ(SCTRL_BIAS);
        reg |= (*(UINT32 *)param);
        REG_WRITE(SCTRL_BIAS, reg);
        break;

    case CMD_SCTRL_BIAS_REG_CLEAN:
        reg = REG_READ(SCTRL_BIAS);
        reg &= ~(*(UINT32 *)param);
        REG_WRITE(SCTRL_BIAS, reg);
        break;

    case CMD_SCTRL_BIAS_REG_READ:
        ret = REG_READ(SCTRL_BIAS);
        break;

    case CMD_SCTRL_BIAS_REG_WRITE:
        REG_WRITE(SCTRL_BIAS, *(UINT32 *)param);
        break;

    case CMD_SCTRL_ANALOG_CTRL4_SET:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL4);
        reg |= (*(UINT32 *)param);
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, reg);
        break;

    case CMD_SCTRL_ANALOG_CTRL4_CLEAN:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL4);
        reg &= ~(*(UINT32 *)param);
        sctrl_analog_set(SCTRL_ANALOG_CTRL4, reg);
        break;

    case CMD_SCTRL_CALI_DPLL:
        sctrl_cali_dpll(0);
        break;

#if (CFG_SOC_NAME != SOC_BK7231)
    case CMD_SCTRL_SET_XTALH_CTUNE:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL2);
        reg &= ~(XTALH_CTUNE_MASK<< XTALH_CTUNE_POSI);
        reg |= (((*(UINT32 *)param) &XTALH_CTUNE_MASK) << XTALH_CTUNE_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL2, reg);
        break;

    case CMD_SCTRL_GET_XTALH_CTUNE:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL2);
        ret = ((reg >> XTALH_CTUNE_POSI) & XTALH_CTUNE_MASK);
        break;

    case CMD_BLE_RF_BIT_SET:
        reg = REG_READ(SCTRL_CONTROL);
        reg |= BLE_RF_EN_BIT;
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_BLE_RF_BIT_CLR:
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(BLE_RF_EN_BIT);
        REG_WRITE(SCTRL_CONTROL, reg);
        break; 
        
    case CMD_EFUSE_WRITE_BYTE: 
        ret = sctrl_write_efuse(param);
        break;
        
    case CMD_EFUSE_READ_BYTE:
        ret = sctrl_read_efuse(param);
        break; 

    case CMD_QSPI_VDDRAM_VOLTAGE:
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(PSRAM_VDDPAD_VOLT_MASK << PSRAM_VDDPAD_VOLT_POSI);
        reg |=(((*(UINT32 *)param) & PSRAM_VDDPAD_VOLT_MASK) << PSRAM_VDDPAD_VOLT_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);
        break;

    case CMD_QSPI_IO_VOLTAGE:
        reg = REG_READ(SCTRL_CONTROL);
        reg &= ~(QSPI_IO_VOLT_MASK << QSPI_IO_VOLT_POSI);
        reg |=(((*(UINT32 *)param) & QSPI_IO_VOLT_MASK) << QSPI_IO_VOLT_POSI);
        REG_WRITE(SCTRL_CONTROL, reg);
        break;
        
#endif // (CFG_SOC_NAME != SOC_BK7231)

#if (CFG_SOC_NAME == SOC_BK7221U)
    case CMD_SCTRL_OPEN_DAC_ANALOG:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL9);
        reg |= EN_AUD_DAC_L | EN_AUD_DAC_R 
              | DAC_PA_OUTPUT_EN | DAC_DRIVER_OUTPUT_EN
              | AUD_DAC_DGA_EN;
        sctrl_analog_set(SCTRL_ANALOG_CTRL9, reg);

        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL10);
        reg |= DAC_N_END_OUPT_L | DAC_N_END_OUPT_R;
        sctrl_analog_set(SCTRL_ANALOG_CTRL10, reg);
        break;

    case CMD_SCTRL_CLOSE_DAC_ANALOG:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL9);
        reg &= ~(EN_AUD_DAC_L | EN_AUD_DAC_R 
              | DAC_PA_OUTPUT_EN | DAC_DRIVER_OUTPUT_EN
              | AUD_DAC_DGA_EN);
        sctrl_analog_set(SCTRL_ANALOG_CTRL9, reg);

        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL10);
        reg &= ~(DAC_N_END_OUPT_L | DAC_N_END_OUPT_R);
        sctrl_analog_set(SCTRL_ANALOG_CTRL10, reg);
        break;

    case CMD_SCTRL_OPEN_ADC_MIC_ANALOG:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        reg &= ~(SPI_PWD_AUD_ADC_L | SPI_PWD_AUD_ADC_R);
        sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        break;
        
    case CMD_SCTRL_CLOSE_ADC_MIC_ANALOG:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        reg |= (SPI_PWD_AUD_ADC_L | SPI_PWD_AUD_ADC_R);
        sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        break;

    case CMD_SCTRL_ENALBLE_ADC_LINE_IN:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        reg |= LINE_IN_EN;
        sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        break; 

    case CMD_SCTRL_DISALBLE_ADC_LINE_IN:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        reg &= ~LINE_IN_EN;
        sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        break;  

    case CMD_SCTRL_SET_DAC_VOLUME_ANALOG:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        reg &= ~(AUD_DAC_GAIN_MASK << AUD_DAC_GAIN_POSI);
        reg |= (((*(UINT32 *)param) & AUD_DAC_GAIN_MASK) << AUD_DAC_GAIN_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        break;

    case CMD_SCTRL_SET_LINEIN_VOLUME_ANALOG:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        reg &= ~(LINE_IN_GAIN_MASK << LINE_IN_GAIN_POSI);
        reg |= (((*(UINT32 *)param) & LINE_IN_GAIN_MASK) << LINE_IN_GAIN_POSI);
        sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        break;

    case CMD_SCTRL_SET_VOLUME_PORT:
        if((*(UINT32 *)param) == AUDIO_DAC_VOL_DIFF_MODE)
        {
            reg = sctrl_analog_get(SCTRL_ANALOG_CTRL9);
            reg |= (DAC_DIFF_EN);
            sctrl_analog_set(SCTRL_ANALOG_CTRL9, reg);

            reg = sctrl_analog_get(SCTRL_ANALOG_CTRL10);
            reg |= (DAC_N_END_OUPT_L | DAC_N_END_OUPT_R);
            reg &= ~(DAC_VSEL_MASK << DAC_VSEL_POSI);
            reg |= ((0x3 & DAC_VSEL_MASK) << DAC_VSEL_POSI);;
            sctrl_analog_set(SCTRL_ANALOG_CTRL10, reg);
        }
        else if((*(UINT32 *)param) == AUDIO_DAC_VOL_SINGLE_MODE)
        {
            reg = sctrl_analog_get(SCTRL_ANALOG_CTRL9);
            reg &= ~(DAC_DIFF_EN);
            sctrl_analog_set(SCTRL_ANALOG_CTRL9, reg);

            reg = sctrl_analog_get(SCTRL_ANALOG_CTRL10);
            reg &= ~(DAC_N_END_OUPT_L | DAC_N_END_OUPT_R);
            reg &= ~(DAC_VSEL_MASK << DAC_VSEL_POSI);
            reg |= ((0 & DAC_VSEL_MASK) << DAC_VSEL_POSI);;
            sctrl_analog_set(SCTRL_ANALOG_CTRL10, reg);
        }
        break;

    case CMD_SCTRL_SET_AUD_DAC_MUTE:
        reg = sctrl_analog_get(SCTRL_ANALOG_CTRL8);
        if((*(UINT32 *)param) == AUDIO_DAC_ANALOG_MUTE)
        {
            reg |= (AUD_DAC_MUTE_EN);
            sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        }
        else if((*(UINT32 *)param) == AUDIO_DAC_ANALOG_UNMUTE)
        {
            reg &= ~(AUD_DAC_MUTE_EN);
            sctrl_analog_set(SCTRL_ANALOG_CTRL8, reg);
        }        
        break;
	case CMD_SCTRL_SET_ANALOG6:
		reg = sctrl_analog_get(SCTRL_ANALOG_CTRL6);
		reg |= (DPLL_CLK_FOR_AUDIO_EN | DPLL_DIVIDER_CLK_SEL | DPLL_RESET );
		sctrl_analog_set(SCTRL_ANALOG_CTRL6, reg);
		break;
	case CMD_SCTRL_SET_ANALOG0:
		sctrl_analog_set(SCTRL_ANALOG_CTRL0, 0x7819A59B);
		break;
	case CMD_SCTRL_SET_ANALOG1:
		sctrl_analog_set(SCTRL_ANALOG_CTRL1, 0x6AC03102);
		break;
	case CMD_SCTRL_SET_ANALOG2:
		sctrl_analog_set(SCTRL_ANALOG_CTRL2, 0x84036080);
		break;
	case CMD_SCTRL_SET_ANALOG3:
		sctrl_analog_set(SCTRL_ANALOG_CTRL3, 0x180004A0);
		break;
	case CMD_SCTRL_SET_ANALOG4:
		sctrl_analog_set(SCTRL_ANALOG_CTRL4, 0x84200E52);
		break;
	case CMD_SCTRL_SET_ANALOG5:
		sctrl_analog_set(SCTRL_ANALOG_CTRL5, 0x3B13B13B);
		break;
	case CMD_SCTRL_SET_ANALOG7:
		sctrl_analog_set(SCTRL_ANALOG_CTRL7, 0x0441A7F0);
		break;
	case CMD_SCTRL_SET_ANALOG8:
		sctrl_analog_set(SCTRL_ANALOG_CTRL8, 0x003B187C);
		break;
	case CMD_SCTRL_SET_ANALOG9:
		sctrl_analog_set(SCTRL_ANALOG_CTRL9, 0x82204007);
		break;	
	case CMD_SCTRL_SET_ANALOG10:
		sctrl_analog_set(SCTRL_ANALOG_CTRL10, 0x80801432);
		break;
	case CMD_SCTRL_AUDIO_PLL:	
		if((*(UINT32 *)param) == 48000000)						//48MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3B13B13B);
		}
		else if((*(UINT32 *)param) == 48128000)					//48.128MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3B3C05AC);
		}
		else if((*(UINT32 *)param) == 48384000)					//48.384MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3B8CAE8E);
		}
		else if((*(UINT32 *)param) == 49152000)					//49.152MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3C7EA932);
		}
		else if((*(UINT32 *)param) == 49392000)					//49.392MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3CCA4785);
		}
		else if((*(UINT32 *)param) == 50688000)					//50.688MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3E629E7C);
		}
		else if((*(UINT32 *)param) == 50803200)					//50.8032MHz
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3E86EA7A);
		}
		else
		{
			sctrl_analog_set(SCTRL_ANALOG_CTRL5,0x3B13B13B);
		}		
		break;

#if CFG_USE_USB_CHARGE
    case CMD_SCTRL_USB_CHARGE_CAL:
        if(1 == ((CHARGE_OPER_PTR)param)->oper)
            charger_vlcf_calibration(0);
        else if(2 == ((CHARGE_OPER_PTR)param)->oper)
            charger_icp_calibration(0);
        else if(3 == ((CHARGE_OPER_PTR)param)->oper)
            charger_vcv_calibration(0);
        else if(4 == ((CHARGE_OPER_PTR)param)->oper)
            charger_calib_get(((CHARGE_OPER_PTR)param)->cal);
        break;
    case CMD_SCTRL_USB_CHARGE_START:
        charger_start(param);
        break;
    case CMD_SCTRL_USB_CHARGE_STOP:
        charger_stop((*(UINT32 *)param));
        break;
#endif

#endif // (CFG_SOC_NAME == SOC_BK7221U)

    case CMD_SCTRL_SET_VDD_VALUE:
    	reg = REG_READ(SCTRL_DIGTAL_VDD);
    	reg &= (~(DIG_VDD_ACTIVE_MASK << DIG_VDD_ACTIVE_POSI));
        reg |=((*(UINT32 *)param) << DIG_VDD_ACTIVE_POSI);
    	REG_WRITE(SCTRL_DIGTAL_VDD, reg);
        break;

    default:
        ret = SCTRL_FAILURE;
        break;
    }

    return ret;
}

// EOF
