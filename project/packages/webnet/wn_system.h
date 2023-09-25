#ifndef WN_SYSTEM_H__
#define WH_SYSTEM_H__

#include <stdint.h>
#include <stdlib.h>

#if RT_USING_PSRAM
void* wn_malloc(size_t size);
void  wn_free(void *ptr);
void* wn_realloc(void *ptr, size_t size);

void* wn_tcm_malloc(size_t size);
void  wn_tcm_free(void *ptr);

char *wn_strdup(const char *s1);
#else
#define wn_malloc       rt_malloc
#define wn_free         rt_free
#define wn_realloc      rt_realloc

#define wn_tcm_malloc   rt_malloc
#define wn_tcm_free     rt_free

#define wn_strdup       rt_strdup

#endif

#endif
