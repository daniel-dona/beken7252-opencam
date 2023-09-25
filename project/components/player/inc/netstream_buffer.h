/*
 * File: netstream_buffer.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __NETSTREAM_BUFFER_H__
#define __NETSTREAM_BUFFER_H__

#include "player_config.h"

int player_netstream_buffer_init(void);
void player_netstream_buffer_deinit(void);
uint8_t* player_netstream_get_buffer(void);
int player_netstream_get_bufsz(void);

#endif

