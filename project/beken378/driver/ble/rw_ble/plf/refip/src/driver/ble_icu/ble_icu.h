#ifndef _BLE_ICU_H_
#define _BLE_ICU_H_

#include "stdint.h"


#define MCU_CLK_16M   1
#define MCU_CLK_64M   2
#define MCU_CLK_32M	  3
#define MCU_CLK_48M   4


#define DEBUG_HW		0

#define MCU_DEFAULT_CLK 	MCU_CLK_16M //MCU_CLK_64M

#define MCU_CLK_32KHz       32000
#define MCU_CLK_16MHz       16000000
#define MCU_CLK_26MHz       26000000
#define MCU_CLK_48MHz       48000000
#define MCU_CLK_96MHz       96000000
#define MCU_CLK_120MHz      120000000

#define MCU_CLK            MCU_CLK_26MHz// MCU_CLK_120MHz


#define PER_CLK_32KHz       32000
#define PER_CLK_16MHz       16000000
#define PER_CLK_26MHz       26000000
#define PER_CLK_48MHz       48000000
#define PER_CLK_96MHz       96000000
#define PER_CLK_120MHz      120000000

#define PER_CLK             PER_CLK_26MHz











void ble_sleep(void);

void ble_icu_init(void);

void cpu_reduce_voltage_sleep(void);

void cpu_idle_sleep(void);

void cpu_wakeup(void);

void mcu_clk_config(void);

void udi_wdt_enable(uint16_t wdt_cnt);

void switch_clk(uint8_t clk);

void bk3435_singleWaveCfg(uint8_t freq, uint8_t power_level);

#endif

