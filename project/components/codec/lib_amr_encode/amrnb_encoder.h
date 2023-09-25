/**
 **************************************************************************************
 * @file    amrnb_encoder.h
 * @brief   A narrow band AMR encoder implementation
 * 
 * @author  Aixing.Li
 * @version V1.0.0
 *
 * &copy; 2018 BEKEN Corporation Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __AMRNB_ENCODER_H__
#define __AMRNB_ENCODER_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>

#define AMRNB_ENCODER_SAMPLES_PER_FRAME		(160)
#define AMRNB_ENCODER_MAX_FRAME_SIZE		(32)

enum AMRNB_MODE
{
	AMRNB_MODE_MR475 = 0,/* 4.75 kbps */
	AMRNB_MODE_MR515,    /* 5.15 kbps */
	AMRNB_MODE_MR59,     /* 5.90 kbps */
	AMRNB_MODE_MR67,     /* 6.70 kbps */
	AMRNB_MODE_MR74,     /* 7.40 kbps */
	AMRNB_MODE_MR795,    /* 7.95 kbps */
	AMRNB_MODE_MR102,    /* 10.2 kbps */
	AMRNB_MODE_MR122,    /* 12.2 kbps */
	AMRNB_MODE_MRDTX,    /* DTX       */
	AMRNB_MODE_N_MODES   /* Not Used  */
};

enum AMRNB_ENCODER_ERROR_CODE
{
	AMRNB_ENCODER_INIT_FAIL = -128,
	AMRNB_ENCODER_DEINIT_FAIL,
	AMRNB_ENCODER_ENCODE_FAIL,
	AMRNB_DECODER_INIT_FAIL,
	AMRNB_DECODER_DEINIT_FAIL,
	AMRNB_DECODER_DECODE_FAIL,
	AMRNB_NO_ERROR = 0
};

/**
 * @brief  AMR-NB encoder initialize.
 * @param  amrnb AMR-NB encoder point
 * @param  dtx   AMR-NB discontinus transmission
 * @param  pmalloc malloc function pointer
 * @param  pfree   free function pointer
 * @return initialize result, 0: success, others: error code
 */
int32_t amrnb_encoder_init(void** amrnb, uint32_t dtx, void* pmalloc, void* pfree);

/**
 * @brief  AMR-NB encoder de-initialize.
 * @param  amrnb AMR-NB encoder point
 * @return de-initialize result, 0: success, others: error code
 */
int32_t amrnb_encoder_deinit(void** amrnb);

/**
 * @brief  AMR-NB encoder encode one frame.
 * @param  amrnb AMR-NB encoder point
 * @param  mode  AMR-NB mode, @see AMRNB_MODE
 * @param  in    input frame PCM buffer
 * @param  out   output encoded AMR buffer
 * @return encode result, >0: number of encoded bytes, others: error code
 */
int32_t amrnb_encoder_encode(void* amrnb, uint32_t mode, const int16_t in[AMRNB_ENCODER_SAMPLES_PER_FRAME], uint8_t out[AMRNB_ENCODER_MAX_FRAME_SIZE]);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__AMRNB_ENCODER_H__
