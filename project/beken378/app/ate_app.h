#ifndef _ATE_APP_H_
#define _ATE_APP_H_

#define ATE_APP_FUN 1

#if ATE_APP_FUN
#include "sys_config.h"
#include "gpio_pub.h"

#include "uart_pub.h"
#define ATE_DEBUG
#ifdef ATE_DEBUG
#define ATE_PRT      os_printf
#define ATE_WARN     warning_prf
#define ATE_FATAL    fatal_prf
#else
#define ATE_PRT      null_prf
#define ATE_WARN     null_prf
#define ATE_FATAL    null_prf
#endif

#define ATE_GPIO_ID            GPIO0   
#define ATE_ENABLE_GIPO_LEVEL  0

void ate_gpio_init(void);
uint32_t ate_mode_check(void);

void ate_app_init(void);
uint32_t get_ate_mode_state(void);
void ate_start(void);

#endif /*ATE_APP_FUN */
#endif // _ATE_APP_H_
// eof

