/*
 * File: data_work.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __DATA_WORK_H_ 
#define __DATA_WORK_H_ 

void data_work(struct rt_work *work, void *user_data); 
int data_work_write(char *data, int len, int timeout); 
int data_work_set_content_length(int len); 

#endif 
