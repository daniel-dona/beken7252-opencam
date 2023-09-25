#ifndef _SYS_RTOS_H_
#define _SYS_RTOS_H_

#include "include.h"

#if CFG_OS_FREERTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "rtos.h"
#include "queue.h"
#include "semphr.h"
#endif

#endif // _SYS_RTOS_H_

