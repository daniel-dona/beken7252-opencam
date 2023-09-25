/**
 ****************************************************************************************
 *
 * @file pwm.h
 *
 * @brief pwm Driver for pwm operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _PWM_H_
#define _PWM_H_

#include "BK3435_reg.h"

#define ENABLE_PWM							0

#define PWM_CHANNEL_NUMBER_MAX       		5

#define ICU_PWM_CLK_PWM_X_PWD_POSI(x)       ((x) * 2)
#define ICU_PWM_CLK_PWM_X_PWD_MASK(x)       (0x01UL << ICU_PWM_CLK_PWM_X_PWD_POSI(x))

#define ICU_PWM_CLK_PWM_X_SEL_POSI(x)       ((x) * 2 + 1)
#define ICU_PWM_CLK_PWM_X_SEL_MASK(x)       (0x01UL << ICU_PWM_CLK_PWM_X_SEL_POSI(x))
#define ICU_PWM_CLK_PWM_X_PWD_CLEAR(x)      do {REG_AHB0_ICU_PWMCLKCON &= (~ICU_PWM_CLK_PWM_X_PWD_MASK(x));} while (0)
#define ICU_PWM_CLK_PWM_X_PWD_SET(x)        do {REG_AHB0_ICU_PWMCLKCON |= ( ICU_PWM_CLK_PWM_X_PWD_MASK(x));} while (0)
#define ICU_PWM_CLK_PWM_X_SEL_32KHZ(x)      do {REG_AHB0_ICU_PWMCLKCON &= (~ICU_PWM_CLK_PWM_X_SEL_MASK(x));} while (0)
#define ICU_PWM_CLK_PWM_X_SEL_16MHZ(x)      do {REG_AHB0_ICU_PWMCLKCON |= ( ICU_PWM_CLK_PWM_X_SEL_MASK(x));} while (0)

#define PWM_CNT_END_VALUE_POSI              0
#define PWM_CNT_END_VALUE_MASK              (0x0000FFFFUL << PWM_CNT_END_VALUE_POSI)

#define PWM_CNT_DUTY_CYCLE_POSI             16
#define PWM_CNT_DUTY_CYCLE_MASK             (0x0000FFFFUL << PWM_CNT_DUTY_CYCLE_POSI)

#define REG_PWM_X_CNT_ADDR(x)               (APB1_PWM_BASE + (x) * 2 * 4 + 0x08)
#define REG_PWM_X_CNT_MASK                  0xFFFFFFFFUL
#define REG_PWM_X_CNT(x)                    (*((volatile unsigned long *) REG_PWM_X_CNT_ADDR(x)))

#define REG_PWM_CTRL_ADDR                   (APB1_PWM_BASE + 0x00 * 4)
#define REG_PWM_CTRL_MASK                   0xFFFFFFFFUL
#define REG_PWM_CTRL                        (*((volatile unsigned long *) REG_PWM_CTRL_ADDR))

#define ICU_INT_ENABLE_IRQ_PWM_X_POSI(x)    (((x) == 5) ? (13) : (x))
#define ICU_INT_ENABLE_IRQ_PWM_X_MASK(x)    (0x01UL << ICU_INT_ENABLE_IRQ_PWM_X_POSI(x))

#define GPIO_CONFIG_SECOND_FUNCTION_POSI        0
#define GPIO_CONFIG_X_SECOND_FUNCTION_POSI(x)   (x)
#define GPIO_CONFIG_SECOND_FUNCTION_MASK        (0x00FFUL << GPIO_CONFIG_SECOND_FUNCTION_POSI)
#define GPIO_CONFIG_X_SECOND_FUNCTION_MASK(x)   (0x0001UL << GPIO_CONFIG_X_SECOND_FUNCTION_POSI(x))


#define PWM_CTRL_PWM_X_ENABLE_POSI(x)       (4 * (x))
#define PWM_CTRL_PWM_X_ENABLE_MASK(x)       (0x01UL << PWM_CTRL_PWM_X_ENABLE_POSI(x))
#define PWM_CTRL_PWM_X_ENABLE_SET(x)        (0x01UL << PWM_CTRL_PWM_X_ENABLE_POSI(x))

#define PWM_CTRL_PWM_X_INT_ENABLE_POSI(x)   (4 * (x) + 1)
#define PWM_CTRL_PWM_X_INT_ENABLE_MASK(x)   (0x01UL << PWM_CTRL_PWM_X_INT_ENABLE_POSI(x))
#define PWM_CTRL_PWM_X_INT_ENABLE_SET(x)    (0x01UL << PWM_CTRL_PWM_X_INT_ENABLE_POSI(x))

#define REG_PWM_X_CAP_OUT_ADDR(x)           (APB1_PWM_BASE + (x) * 2 * 4 + 0x0C)
#define REG_PWM_X_CAP_OUT_MASK              0x0000FFFFUL
#define REG_PWM_X_CAP_OUT(x)                (*((volatile unsigned long *) REG_PWM_X_CAP_OUT_ADDR(x)))


#define REG_PWM_INTR_ADDR                   (APB1_PWM_BASE + 0x01 * 4)
#define REG_PWM_INTR_MASK                   0x3F
#define REG_PWM_INTR                        (*((volatile unsigned long *) REG_PWM_INTR_ADDR))

#define ICU_INT_ENABLE_SET(x)               do {                                    \
                REG_AHB0_ICU_INT_ENABLE |=  (x);                                    \                                                               \
      		} while(0)
      		
#define ICU_INT_ENABLE_CLEAR(x)             do {                                    \
                REG_AHB0_ICU_INT_ENABLE &= (~(x));                                  \
            } while(0)

//----------------------------------------------
// PWM driver description
//----------------------------------------------
typedef struct
{
    unsigned char     channel;        // PWM 0~5, GPIOB P1.0~P1.5

   /* mode:   PWM mode
	* bit[0]:  PWM enable
	*          0: PWM disable
	*          1: PWM enable
	* bit[1]:   PWM interrupt enable
	*          0: PWM interrupt disable
	*          1: PWM interrupt enable
	* bit[3:2]: PWM mode selection
	*          00: PWM mode
	*          01: TIMER
	*          10: Capture Posedge
	*          11: Capture Negedge
	* bit[5:4]: PWM clock select
	*          00: PWM clock 32KHz
	*          01: PWM clock 16MHz
	* bit[7:6]: reserved
	*/
    unsigned char     mode;
    unsigned char     pre_divid;	// PWM pre-divide clk
    unsigned short    end_value;    // PWM counter end value
    unsigned short    duty_cycle;   // PWM counter duty cycle, 
                                    // this value must smaller or equal to end value
    void (*p_Int_Handler)(unsigned char ucChannel);     // PWM channel Interrupt Handler
} PWM_DRV_DESC;


/**
 ****************************************************************************************
 * @defgroup PWM 
 * @ingroup DRIVERS
 * @brief PWM driver
 *
 * @{
 *
 ****************************************************************************************
 */

//void pwm_init(void);
void pwm_init(PWM_DRV_DESC *pwm_drv_desc);

void pwm_enable(unsigned char ucChannel);

void pwm_disable(unsigned char ucChannel);

void pwm_int_enable(unsigned char ucChannel);

void pwm_int_disable(unsigned char ucChannel);

unsigned short pwm_capture_value_get(unsigned char ucChannel);

void pwm_int_handler_clear(unsigned char ucChannel);

void pwm_isr(void);
 
#endif // _PWM_H_
