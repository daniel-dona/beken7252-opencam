/**
****************************************************************************************
* @addtogroup RTC
* @ingroup beken corp
* @brief RTC
* @author Alen
*
* This is the driver block for RTC
* @{
****************************************************************************************
*/


#ifndef __RTC_H__
#define __RTC_H__


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "BK3435_reg.h"

//----------------------------------------------
// RTC driver description
//----------------------------------------------
typedef struct
{
    unsigned char       second;     // second, 0~59
    unsigned char       minute;     // minute, 0~59
    unsigned char       hour;       // hour,   0~23
    unsigned char       week_day;   // week_day, 0~6
} RTC_DATE_DESC;


#define ENABLE_RTC						0

#define ICU_RTC_CLK_PWD_CLEAR()        do {REG_AHB0_ICU_RTCCLKCON = 0x00;} while (0)
#define ICU_RTC_CLK_PWD_SET()          do {REG_AHB0_ICU_RTCCLKCON = 0x01;} while (0)


void rtc_init(RTC_DATE_DESC *p_RTC_date_desc);
void rtc_enable(void);
void rtc_disable(void);
void rtc_alarm_init(unsigned char ucMode, RTC_DATE_DESC *p_RTC_alarm_time, 
                           unsigned long ulMiiliSecond, void (*p_Int_Handler)(void));
void rtc_alarm_enable(void);
void rtc_alarm_disable(void);
void rtc_set_time(RTC_DATE_DESC *p_RTC_date_desc);
void rtc_get_time(RTC_DATE_DESC *p_RTC_date_desc);
void rtc_int_handler_clear(void);

void rtc_isr(void);


#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif      /* __RTC_H__ */

