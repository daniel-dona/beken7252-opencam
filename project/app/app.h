#ifndef _APP_H_
#define _APP_H_

#pragma once

#include "sys_rtos.h"
#include "rtos_pub.h"

//#define APP_DEBUG

#ifdef APP_DEBUG
#define APP_PRT       os_printf
#define APP_WPRT      warning_prf
#else
#define APP_PRT       os_null_printf
#define APP_WPRT      warning_prf
#endif

enum
{
    BMSG_NULL_TYPE          = 0,
    BMSG_RX_TYPE            = 1,
    BMSG_TX_TYPE            = 2,
    BMSG_IOCTL_TYPE         = 3,
    BMSG_SKT_TX_TYPE        = 4,
    BMSG_MEDIA_TYPE         = 5,

#if CFG_USE_AP_PS
    BMSG_TXING_TYPE         = 6,
#endif
    BMSG_STA_PS_TYPE        = 7,

    BMSG_TX_RAW_TYPE        = 8,
    BMSG_TX_RAW_CB_TYPE     = 9,

#if (SUPPORT_LSIG_MONITOR)
    BMSG_RX_LSIG = 10,      /* phy receive 802.11 LSIG*/
#endif
};

typedef struct bus_message
{
    uint32_t type;
    uint32_t arg;
    uint32_t len;
    beken_semaphore_t sema;

	void *cb;
	void *param;
} BUS_MSG_T;

#define CORE_QITEM_COUNT          (64)
#if (CFG_SUPPORT_ALIOS)
#define CORE_STACK_SIZE           (4 * 1024)
#else
#define CORE_STACK_SIZE           (2 * 1024)
#endif

typedef struct _wifi_core_
{
    uint32_t queue_item_count;
    beken_queue_t io_queue;

    beken_thread_t handle;
    uint32_t stack_size;
} WIFI_CORE_T;

void app_start(void);
void app_pre_start(void);
int bmsg_is_empty(void);
void core_thread_uninit(void);
int bmsg_tx_raw_cb_sender(uint8_t *buffer, int length, void *cb, void *param);

#endif // _APP_H_
// eof

