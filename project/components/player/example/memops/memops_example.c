/*
 * File: tts_player_sample.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-13     liu2guang    add player memops example.
 */

#include "rtthread.h"
#include "player.h" 
#include "player_init.h" 

static struct player_mem_ops mem_ops = 
{
    rt_malloc,   /* PLAYER 内部低速内存 */
    rt_free,     /* PLAYER 内部低速内存 */
    rt_realloc,  /* PLAYER 内部低速内存 */
    rt_malloc,   /* PLAYER 内部高速内存 */
    rt_free,     /* PLAYER 内部高速内存 */ 
    psram_rpipe, /* PLAYER 网络缓存内存 */ 
    psram_wpipe, /* PLAYER 网络缓存内存 */ 
    RT_NULL,     /* PLAYER 网络缓存地址, 外部分配后传入 */ 
    0            /* PLAYER 网络缓存大小(Bytes) */ 
}; 

static rt_size_t psram_rpipe(void *from, void *to, rt_size_t size)
{ 
    /* 这一步需要将from指针开始的size大小的数据搬运到to指定地址 */ 
    rt_memcpy(from, to, size); 
    return 0;
}

static rt_size_t psram_wpipe(void *from, void *to, rt_size_t size)
{
    /* 这一步需要将from指针开始的size大小的数据搬运到to指定地址 */ 
    rt_memcpy(from, to, size); 
    return 0;
}

rt_err_t memops_example(void)
{
    rt_err_t ret = RT_EOK; 

    /* 使用 96KBytes 内存 */
    #define PIPE_SIZE (96*1024) 

    rt_uint8_t *ppipe = RT_NULL; 
	ppipe = (rt_uint8_t *)rt_malloc(PIPE_SIZE); 
    if(ppipe == RT_NULL)
    {
        rt_kprintf("ppipe malloc failed\n"); 
        return (-RT_ERROR); 
    }
    mem_ops._pipe_addr = (int)ppipe; 
    mem_ops._pipe_size = (int)PIPE_SIZE; 
    player_mem_ops_register(&mem_ops); 

    /* 需要先注册配置 mem ops 后初始化 player */ 
    player_system_init();

    return ret; 
} 
