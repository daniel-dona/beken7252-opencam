/** **************************************************************************************
 *
 * @file bk3435_enc_key.h
 *
 * @brief bk3435 enc module application
 *
 * @auth  alen
 *
 * @date  2017.10.13
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _BK3435_ENC_KEY_H__
#define _BK3435_ENC_KEY_H__


/*----------------------------------------------------------------------------*
 *  NAME
 *   	generate_key
 *  DESCRIPTION
 *		To Generate Key Value.
 *  PARAMETERS
 *
 *  RETURNS
 *		VOID
 *---------------------------------------------------------------------------*/
void generate_key(uint8_t *mac, uint8_t *data, uint8_t *key );

#endif

