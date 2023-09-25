/*
 * File      : interrupt.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013-2018, RT-Thread Development Team
 * 
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "intc_pub.h"

void rt_hw_interrupt_init(void);
void rt_hw_interrupt_mask(int vector);
void rt_hw_interrupt_umask(int vector);
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler, void *param, char *name);

#endif /* __INTERRUPT_H__ */
