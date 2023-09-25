/**
 ****************************************************************************************
 *
 * @file boot.h
 *
 * @brief This file contains the declarations of the boot related variables.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _BOOT_H_
#define _BOOT_H_

#if 0
/// Address of beginning of the CODE
extern char code_base;
#define CODE_BASE (&(code_base))

/// Address of the end of the CODE
extern char code_end;
#define CODE_END (&(code_end))

/// Length of the code
#define CODE_LENGTH ((CODE_END) - (CODE_BASE))

/// Address of beginning of the DATA
extern char data_base;
#define DATA_BASE (&(data_base))

/// Address of the end of the DATA
extern char data_end;
#define DATA_END (&(data_end))

/// Length of the DATA
#define DATA_LENGTH ((DATA_END) - (DATA_BASE))

/// Unloaded RAM area base address
extern char unloaded_area_start;
#define RAM_UNLOADED_BASE   (&(unloaded_area_start))

/// Stack base address



#endif




/// Length of the code
extern const unsigned int Image$$ROM$$Length[];
#define CODE_LENGTH        ((uint32_t)Image$$ROM$$Length)

/// Length of the RW data
extern const unsigned int Image$$RAM_DATA$$Length[];
#define DATA_LENGTH        ((uint32_t)Image$$RAM_DATA$$Length)

/// Unloaded RAM area base address 
extern const unsigned int Image$$RAM_UNLOADED$$Base[];
#define RAM_UNLOADED_BASE   ((uint32_t)Image$$RAM_UNLOADED$$Base)

/// Stack base address and len of UNUSED
extern const unsigned int boot_stack_base_UNUSED;
#define STACK_BASE_UNUSED   ((uint32_t)boot_stack_base_UNUSED)
extern const unsigned int boot_stack_len_UNUSED;
#define STACK_LEN_UNUSED   ((uint32_t)boot_stack_len_UNUSED)

/// Stack base address and len of SVC
extern const unsigned int boot_stack_base_SVC ;
#define STACK_BASE_SVC   ((uint32_t)boot_stack_base_SVC)
extern const unsigned int boot_stack_len_SVC;
#define STACK_LEN_SVC  ((uint32_t)boot_stack_len_SVC)

/// Stack base address and len of IRQ
extern const unsigned int boot_stack_base_IRQ;
#define STACK_BASE_IRQ  ((uint32_t)boot_stack_base_IRQ)
extern const unsigned int boot_stack_len_IRQ;
#define STACK_LEN_IRQ   ((uint32_t)boot_stack_len_IRQ)

/// Stack base address and len of FIQ
extern const unsigned int boot_stack_base_FIQ;
#define STACK_BASE_FIQ   ((uint32_t)boot_stack_base_FIQ)
extern const unsigned int boot_stack_len_FIQ;
#define STACK_LEN_FIQ   ((uint32_t)boot_stack_len_FIQ)

#define BOOT_PATTERN_UNUSED   0xAAAAAAAA      // Pattern to fill UNUSED stack
#define BOOT_PATTERN_SVC      0xBBBBBBBB      // Pattern to fill SVC stack
#define BOOT_PATTERN_IRQ      0xCCCCCCCC      // Pattern to fill IRQ stack
#define BOOT_PATTERN_FIQ      0xDDDDDDDD      // Pattern to fill FIQ stack

#endif // _BOOT_H_
