/*
 * File: tts_player_sample.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-05     liu2guang    add play tts function(Block and Break).
 */

#include "tts_player.h" 

/* 支付宝收款10万元 */ 
static char *list1[] = {
    "/mnt/sdcard/root/alipay.mp3",          /* 支付宝收款.mp3 */ 
    "/mnt/sdcard/root/ten.mp3",             /* 十.mp3 */ 
    "/mnt/sdcard/root/ten_thousand.mp3",    /* 万.mp3 */ 
    "/mnt/sdcard/root/rmb.mp3"              /* 元.mp3 */
}; 

/* 连接服务器成功 */ 
static char *list2[] = 
{
    "/mnt/sdcard/root/connect_server_success.mp3"
}; 

/* 阻塞式播放 TTS */ 
void tts_player_block_run(void *p)
{
    tts_player_block(list1, sizeof(list1)/sizeof(char *)); 
}

/* 打断不恢复式播放 TTS */ 
void tts_player_break_run(void *p)
{
    tts_player_break(list2, sizeof(list2)/sizeof(char *)); 
}

/* 阻塞播放 TTS:支付宝收款10万元 */ 
static int tts_player_block_test(int argc, char** argv)
{
    rt_thread_t thread = RT_NULL;
   
    thread = rt_thread_create("tts_block", tts_player_block_run, RT_NULL, 1024 * 4, 10, 5);
    if(thread == RT_NULL)
    {
        return (-RT_ERROR); 
    }
    rt_thread_startup(thread);
   
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(tts_player_block_test, tts_player_block_test, tts_player_block_test); 

/* 打断播放 TTS:连接服务器成功 */ 
static int tts_player_break_test(int argc, char** argv)
{
    rt_thread_t thread = RT_NULL;
   
    thread = rt_thread_create("tts_break", tts_player_break_run, RT_NULL, 1024 * 4, 10, 5);
    if(thread == RT_NULL)
    {
        return (-RT_ERROR); 
    }
    rt_thread_startup(thread);
   
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(tts_player_break_test, tts_player_break_test, tts_player_break_test); 
