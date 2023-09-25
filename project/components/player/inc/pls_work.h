/*
 * File: pls_work.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __PLS_WORK_H__ 
#define __PLS_WORK_H__ 

void pls_work(struct rt_work* work, void* user_data);
rt_err_t pls_mode(int mode); // 0: once, 1: cycle 
int pls_dump_music(void); 

#endif 
