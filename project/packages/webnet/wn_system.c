#include <rtthread.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "wn_system.h"

#if RT_USING_PSRAM
#include <drv_sdram.h>

void* wn_malloc(size_t size)
{
    return sdram_malloc(size);
}

void  wn_free(void *ptr)
{
    sdram_free(ptr);
}

void* wn_realloc(void *ptr, size_t size)
{
    return sdram_realloc(ptr, size);
}

void* wn_tcm_malloc(size_t size)
{
    return RT_KERNEL_MALLOC(size);
}

void  wn_tcm_free(void *ptr)
{
    RT_KERNEL_FREE(ptr);
}

char *wn_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *tmp = (char *)wn_malloc(len);

    if (!tmp) return NULL;

    memcpy(tmp, s, len);

    return tmp;
}
#endif
