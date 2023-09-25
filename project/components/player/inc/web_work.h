/*
 * File: web_work.c
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __WEB_WORK_H__
#define __WEB_WORK_H__

void webclient_work(struct rt_work* work, void* user_data);
rt_err_t player_set_link_convert_probe_func(rt_bool_t (*probe)(char *uri)); 
rt_err_t player_set_custom_codec_probe_func(rt_bool_t (*probe)(char *uri)); 
rt_err_t player_set_custom_link_convert_func(char *(*convert)(char *uri)); 

#endif
