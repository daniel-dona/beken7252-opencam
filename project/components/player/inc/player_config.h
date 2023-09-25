/*
 * File: player_config.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __PLAYER_CONFIG_H__
#define __PLAYER_CONFIG_H__

#include <rtthread.h>

// CI: Auto build library
#ifndef PLAYER_USING_DEBUG
#define PLAYER_USING_DEBUG 0
#endif 

/* Debug */ 
#define DBG_ENABLE
#if (PLAYER_USING_DEBUG == 1)
#define DBG_LEVEL DBG_LOG
#else
#define DBG_LEVEL DBG_INFO
#endif 
#define DBG_COLOR
#include <rtdbg.h>

// Stream config
#ifdef PLAYER_ENABLE_NET_STREAM
#define CONFIG_NET_STREAM      1
#endif
#ifdef PLAYER_ENABLE_HTTP_STREAM
#define CONFIG_HTTP_STREAM     1
#endif
#ifdef PLAYER_ENABLE_HTTPS_STREAM
#define CONFIG_HTTPS_STREAM    1
#endif
#ifdef PLAYER_ENABLE_HLS_STREAM
#define CONFIG_HLS_STREAM      1
#endif
#ifdef PLAYER_ENABLE_PLS_STREAM
#define CONFIG_PLS_STREAM      1
#endif
#ifdef PLAYER_ENABLE_SESSION_STREAM
#define CONFIG_SESSION_STREAM  1
#endif
#ifdef PLAYER_ENABLE_TTS_STREAM
#define CONFIG_TTS_STREAM      1
#endif

#define CONFIG_PLAYER_APP      1
#define CONFIG_PLAYER_MEMALLOC 1 

// 开启HLS功能时默认缓存为720K, 未开启时默认未96K
#ifndef PLAYER_USING_NETSTREAM_BUFSZ
#ifndef PLAYER_ENABLE_NET_STREAM
#define PLAYER_USING_NETSTREAM_BUFSZ (96)
#else
#define PLAYER_USING_NETSTREAM_BUFSZ (720)
#endif 
#endif

#define NETSTREAM_BUFSZ (PLAYER_USING_NETSTREAM_BUFSZ * 1024)

#endif

