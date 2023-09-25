/**
 ****************************************************************************************
 *
 * @file intc.h
 *
 * @brief Declaration of the Interrupt Controller API.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _INTC_H_
#define _INTC_H_

/**
 ****************************************************************************************
 * @addtogroup INTC INTC
 * @ingroup DRIVERS
 *
 * @brief Declaration of the Interrupt Controller API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "ble_compiler.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/** @name Mapping of the peripherals interrupts in the interrupt controller.
 * @{
 */

//FIQ
#define INT_PWM0_bit     (0x01<<0)
#define INT_LBD_bit      (0x01<<10)
#define INT_BLE_bit      (0x01<<15)
//IRQ
#define INT_RTC_bit      (0x01<<12)
#define INT_GPIO_bit     (0x01<< 9)
#define INT_ADC_bit      (0x01<< 8)
#define INT_I2C_bit      (0x01<< 7)
#define INT_SPI_bit      (0x01<< 6)
#define INT_UART_bit     (0x01<< 5)
#define INT_PWM4_bit     (0x01<< 4)
#define INT_PWM3_bit     (0x01<< 3)
#define INT_PWM2_bit     (0x01<< 2)
#define INT_PWM1_bit     (0x01<< 1)

//FIQ
#define INT_STATUS_PWM0_bit     (0x01<<0)
#define INT_STATUS_LBD_bit      (0x01<<10)
#define INT_STATUS_BLE_bit      (0x01<<30)
//IRQ
#define INT_STATUS_SDM_bit      (0x01<<17)
#define INT_STATUS_RTC_bit      (0x01<<12)
#define INT_STATUS_GPIO_bit     (0x01<< 9)
#define INT_STATUS_ADC_bit      (0x01<< 8)
#define INT_STATUS_I2C_bit      (0x01<< 7)
#define INT_STATUS_SPI_bit      (0x01<< 6)
#define INT_STATUS_UART_bit     (0x01<< 5)
#define INT_STATUS_PWM4_bit     (0x01<< 4)
#define INT_STATUS_PWM3_bit     (0x01<< 3)
#define INT_STATUS_PWM2_bit     (0x01<< 2)
#define INT_STATUS_PWM1_bit     (0x01<< 1)

#define INT_IRQ_BIT             (0x01<<0)
#define FIQ_IRQ_BIT             (0x01<<1)

/// @} INTC mapping

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialize and configure the reference interrupt controller.
 *
 * This function configures the interrupt controller according to the system needs.
 *
 ****************************************************************************************
 */
void intc_init(void);

/**
 ****************************************************************************************
 * @brief Clear status of interrupts.
 *
 * This function clear interrupt status.
 *
 ****************************************************************************************
 */
void intc_stat_clear(void);

/**
 ****************************************************************************************
 * @brief IRQ Handler.
 *
 ****************************************************************************************
 */
__IRQ void intc_irq(void);

/**
 ****************************************************************************************
 * @brief FIQ Handler.
 *
 ****************************************************************************************
 */
__FIQ void intc_fiq(void);

/// @} INTC

#endif // _INTC_H_
