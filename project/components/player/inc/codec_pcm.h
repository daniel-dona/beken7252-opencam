/*
 * File: codec_pcm.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __CODEC_PCM_H__
#define __CODEC_PCM_H__

int player_codec_pcm_register(void); 
rt_err_t codec_pcm_set_sample_rate(int sample_rate); 
rt_err_t codec_pcm_set_channel_num(int channel_num); 

#endif
