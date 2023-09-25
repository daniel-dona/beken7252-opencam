/**
 ****************************************************************************************
 *
 * @file alinkapp.h
 *
 * @brief  Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2017.11.08
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef _ALINKAPP_H_
#define _ALINKAPP_H_



#include "api_export.h"


void alink_status_changed_cb(alink_event_t event);

void alink_set_dev_status_cb(uint8_t *buffer, uint32_t length);


void alink_get_dev_status_cb(uint8_t *buffer, uint32_t length);







#endif // _ALINKAPP_H_

