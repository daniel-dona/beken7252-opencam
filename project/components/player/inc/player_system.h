/*
 * File: player_system.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __PLAYER_SYSTEM_H__
#define __PLAYER_SYSTEM_H__

#include <stdint.h>
#include <stdlib.h>
#include <player_config.h>

struct player_mem_ops
{
    void *    (*malloc     )(rt_size_t size);
    void      (*free       )(void *p);
    void *    (*realloc    )(void *ptr, rt_size_t size); 
    void *    (*tcm_malloc )(rt_size_t size);
    void      (*tcm_free   )(void *p);
    rt_size_t (*_read_pipe )(void *from, void *to, rt_size_t size); 
    rt_size_t (*_write_pipe)(void *from, void *to, rt_size_t size); 
	uint32_t _pipe_addr; 
    int32_t _pipe_size; 
};
typedef struct player_mem_ops* player_mem_ops_t; 

void* player_malloc(size_t size);
void  player_free(void *ptr);
void* player_realloc(void *ptr, size_t size);

void* player_tcm_malloc(size_t size);
void  player_tcm_free(void *ptr);

char *player_strdup(const char *s1);

int player_mem_ops_register(player_mem_ops_t mem_ops); 

#endif
