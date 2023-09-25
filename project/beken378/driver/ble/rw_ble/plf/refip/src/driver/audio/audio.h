/**
 ****************************************************************************************
 *
 * @file audio.h
 *
 * @brief audio Driver for audio operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_



#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions

/**
 ****************************************************************************************
 * @defgroup AUDIO 
 * @ingroup DRIVERS
 * @brief AUDIO driver
 *
 * @{
 *
 ****************************************************************************************
 */
 

#define MAX_PCM_BUF_ROW		 6
#define MAX_PCM_BUF_LEN      80	

extern int16_t pcm_sample_buffer[MAX_PCM_BUF_ROW][MAX_PCM_BUF_LEN];

extern uint8_t pcm_index_in;
extern uint8_t pcm_index_out;

void audio_init(void);
void audio_isr(void);
void audio_start(void);
void audio_stop(void);


#endif
