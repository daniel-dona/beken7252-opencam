/*
 * File: tts_player.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-05     liu2guang    add play tts function(Block and Break).
 */

#ifndef __TTS_PLAYER_H__
#define __TTS_PLAYER_H__

#include "rtthread.h"

/**
 * play tts file list with Block mode, wait for the last play to finish before playing
 * 
 * @param files: the two-dimensional array for tts file list
 * @param num: the num of tts file list, it has to be greater than zero
 *
 * @return == RT_EOK: play success
 *         ==-RT_EINVAL: invalid parameter
 */
extern rt_err_t tts_player_block(char *files[], rt_uint8_t num); 

/**
 * play tts file list with Break mode, break and does not resume the previous playback
 * 
 * @param files: the two-dimensional array for tts file list
 * @param num: the num of tts file list, it has to be greater than zero
 *
 * @return == RT_EOK: play success
 *         ==-RT_EINVAL: invalid parameter
 */
extern rt_err_t tts_player_break(char *files[], rt_uint8_t num); 

#endif
