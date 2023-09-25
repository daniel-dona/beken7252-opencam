#ifndef _MCU_PS_H_
#define _MCU_PS_H_

//#define MCU_PS_DEBUG

#ifdef MCU_PS_DEBUG
#define MCU_PS_PRT                 os_printf

#else
#define MCU_PS_PRT                 os_null_printf

#endif

#define     MS_TO_TICK      (1000 / RT_TICK_PER_SECOND)

extern void ps_pwm0_disable(void );
extern void ps_pwm0_enable(void);
extern void ps_timer3_disable(void);
extern void ps_timer3_enable(UINT32 );

#endif

