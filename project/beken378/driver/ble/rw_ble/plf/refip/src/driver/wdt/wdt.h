#ifndef __WDT_H__
#define __WDT_H__


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "BK3435_reg.h"

#define ENABLE_WDT							0


#define ICU_WDT_CLK_PWD_CLEAR()             do {REG_AHB0_ICU_WDTCLKCON = 0x00;} while (0)
#define ICU_WDT_CLK_PWD_SET()               do {REG_AHB0_ICU_WDTCLKCON = 0x01;} while (0)

#define WDKEY_ENABLE1           0x005A
#define WDKEY_ENABLE2           0x00A5

#define WDKEY_ENABLE_FOREVER    0x00FF

#define WDKEY_DISABLE1          0x00DE
#define WDKEY_DISABLE2          0x00DA

#define WDT_CONFIG_PERIOD_POSI          0
#define WDT_CONFIG_PERIOD_MASK          (0x0000FFFFUL << WDT_CONFIG_PERIOD_POSI)

#define WDT_CONFIG_WDKEY_POSI           16
#define WDT_CONFIG_WDKEY_MASK           (0x00FFUL << WDT_CONFIG_WDKEY_POSI)


void  wdt_init(void);
void  wdt_feed(void);
void  wdt_disable(void);
void  wdt_enable(void);



#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif      /* __DRIVER_WDT_H__ */


