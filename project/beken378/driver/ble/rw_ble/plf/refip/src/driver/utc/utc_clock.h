/**
****************************************************************************************
* @addtogroup utc_clock
* @ingroup beken corp
* @brief utc_clock
* @author Alen
*
* This is the driver block for utc_clock
* @{
****************************************************************************************
*/


#ifndef __UTC_CLOCK_H__
#define __UTC_CLOCK_H__


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define UTC_ENABLE			0

// number of seconds since 0 hrs, 0 minutes, 0 seconds, on the
// 1st of January 2000 UTC
typedef uint32_t UTCTime;


// To be used with
typedef struct
{
  uint8_t seconds;  // 0-59
  uint8_t minutes;  // 0-59
  uint8_t hour;     // 0-23
  uint8_t day;      // 0-30
  uint8_t month;    // 0-11
  uint16_t year;    // 2000+
} UTCTimeStruct;


void utc_update( void );
void utc_set_time(UTCTimeStruct *tm);
void utc_get_time(UTCTimeStruct *tm);

#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif      /* __SOFT_RTC_H__ */

