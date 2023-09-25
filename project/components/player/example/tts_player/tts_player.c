/*
 * File: tts_player.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-05     liu2guang    add play tts function(Block and Break).
 */

#include "tts_player.h"

#include "player.h"
#include "data_work.h"
#include "audio_codec.h"

#include "dfs.h"
#include "dfs_posix.h"

/**
 * play tts file list with Block mode, Wait for the last play to finish before playing
 * 
 * @param files: the two-dimensional array for tts file list
 * @param num: the num of tts file list, it has to be greater than zero
 *
 * @return == RT_EOK: play success
 *         ==-RT_EINVAL: invalid parameter
 */
rt_err_t tts_player_block(char *files[], rt_uint8_t num)
{
    rt_uint8_t index = 0;
    static char buffer[2048] = {0}; 
     
    rt_kprintf("start tts play.\n"); 
    
    /* 检查传入 TTS 文件数量，数量不能为 0 */ 
    if(num == 0)
    {
        rt_kprintf("the number of songs cannot be zero."); 
        return (-RT_EINVAL); 
    }
    
    /* 检查传入 TTS 文件是否为空 */ 
    while(1)
    {
        if(files[index] == RT_NULL)
        {
            rt_kprintf("the files cannot be empty.");
            return (-RT_EINVAL); 
        }
        else
        {
            rt_kprintf("list[%d] = %s.\n", index, files[index]);
        }
        
        index++; 
        if(index >= num)
        {
            index = 0; 
            break; 
        }
    }
    
    /* 等待上一次 TTS 或者歌曲播放完毕 */ 
    while(player_get_state() == PLAYER_STAT_PLAYING)
    {
        rt_thread_mdelay(1); 
    }
 
    /* 启动 TTS 播放 */ 
    player_play_data(CODEC_MP3, -1); 

    /* 循环打开列表文件写入音频数据到 player的 pipe中 */ 
    while(1)
    {
        int len = (-1);
        int fd  = (-1);

        fd = open(files[index], O_RDONLY); 
        do{ 
            len = read(fd, buffer, 2048); 
            if(len > 0)
            {
                player_write_data(buffer, len);
            }
        }
        while(len > 0); 
        close(fd); 

        index++; 
        if(index >= num)
        {
            index = 0; 
            break; 
        }
    }

    rt_kprintf("start tts play finish.\n"); 

    return RT_EOK; 
}

/**
 * play tts file list with Break mode, break and does not resume the previous playback
 * 
 * @param files: the two-dimensional array for tts file list
 * @param num: the num of tts file list, it has to be greater than zero
 *
 * @return == RT_EOK: play success
 *         ==-RT_EINVAL: invalid parameter
 */
rt_err_t tts_player_break(char *files[], rt_uint8_t num)
{
    rt_uint8_t index = 0;
    static char buffer[2048] = {0}; 
     
    rt_kprintf("start tts play.\n"); 
    
    /* 检查传入 TTS 文件数量，数量不能为 0 */ 
    if(num == 0)
    {
        rt_kprintf("the number of songs cannot be zero."); 
        return (-RT_EINVAL); 
    }
    
    /* 检查传入 TTS 文件是否为空 */ 
    while(1)
    {
        if(files[index] == RT_NULL)
        {
            rt_kprintf("the files cannot be empty.");
            return (-RT_EINVAL); 
        }
        else
        {
            rt_kprintf("list[%d] = %s.\n", index, files[index]);
        }
        
        index++; 
        if(index >= num)
        {
            index = 0; 
            break; 
        }
    }
 
    /* 启动 TTS 播放 */ 
    player_play_data(CODEC_MP3, -1); 

    /* 循环打开列表文件写入音频数据到 player的 pipe中 */ 
    while(1)
    {
        int len = (-1);
        int fd  = (-1);

        fd = open(files[index], O_RDONLY); 
        do{ 
            len = read(fd, buffer, 2048); 
            if(len > 0)
            {
                player_write_data(buffer, len);
            }
        }
        while(len > 0); 
        close(fd); 

        index++; 
        if(index >= num)
        {
            index = 0; 
            break; 
        }
    }

    rt_kprintf("start tts play finish.\n"); 

    return RT_EOK; 
}
