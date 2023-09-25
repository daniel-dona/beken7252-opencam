/*
 * File      : rt_cld_init.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-20     Murphy       the first version
 */

#include <rtthread.h>

#ifdef PKG_USING_EASYFLASH
#include <easyflash.h>

void rt_cld_init(void)
{
    extern EfErrCode easyflash_init(void);
    easyflash_init();
}
INIT_ENV_EXPORT(rt_cld_init);

#endif
