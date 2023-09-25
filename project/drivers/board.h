/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-5-30      Bernard      the first version
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <stdint.h>

extern unsigned char _empty_ram;

/* High Speed */ 
#define RT_HW_TCM_BEGIN     (void*)&_empty_ram
#define RT_HW_TCM_END       (void*)(0x00400000 + 256 * 1024)

/* Low Speed */ 
#define RT_HW_SDRAM_BEGIN   (void*)(0x00900000)
#define RT_HW_SDRAM_END     (void*)(0x00900000 + 256 * 1024) 

void rt_hw_board_init(void);

void rt_sdram_heap_init(void);
void *sdram_malloc(unsigned long size);
void sdram_free(void *ptr);
void *sdram_calloc(unsigned int n, unsigned int size);
void *sdram_realloc(void *ptr, unsigned long size);

void *tcm_malloc(unsigned long size);
void  tcm_free(void *ptr);
void *tcm_calloc(unsigned int n, unsigned int size);
void *tcm_realloc(void *ptr, unsigned long size);

#endif
