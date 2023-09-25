/**
 ****************************************************************************************
 *
 * @file kernel_env.h
 *
 * @brief This file contains the definition of the kernel.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_ENV_H_
#define _KERNEL_ENV_H_

/**
 ****************************************************************************************
 * @addtogroup ENV Environment
 * @ingroup KERNEL
 * @brief Kernel Environment
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "common_list.h"              // kernel queue definition
#include "kernel_config.h"            // kernel configuration
#include "kernel_event.h"             // kernel event
#include "rwip_config.h"          // stack configuration

// forward declaration
struct mblock_free;

/// Kernel environment definition
struct kernel_env_tag
{
    /// Queue of sent messages but not yet delivered to receiver
    struct common_list queue_sent;
    /// Queue of messages delivered but not consumed by receiver
    struct common_list queue_saved;
    /// Queue of timers
    struct common_list queue_timer;

    #if (KERNEL_MEM_RW)
    /// Root pointer = pointer to first element of heap linked lists
    struct mblock_free * heap[KERNEL_MEM_BLOCK_MAX];
    /// Size of heaps
    uint16_t heap_size[KERNEL_MEM_BLOCK_MAX];

    #if (KERNEL_PROFILING)
    /// Size of heap used
    uint16_t heap_used[KERNEL_MEM_BLOCK_MAX];
    /// Maximum heap memory used
    uint32_t max_heap_used;
    #endif //KERNEL_PROFILING
    #endif //KERNEL_MEM_RW
};

/// Kernel environment
extern struct kernel_env_tag kernel_env;

/// @} ENV

#endif // _KERNEL_ENV_H_
