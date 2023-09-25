/**
 ****************************************************************************************
 *
 * @file kernel_queue.h
 *
 * @brief This file contains the definition of the message object, queue element
 * object and queue object
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_QUEUE_H_
#define _KERNEL_QUEUE_H_

/**
 ****************************************************************************************
 * @addtogroup QUEUE Queues and Lists
 * @ingroup KERNEL
 * @brief Queue management module
 *
 * This module implements the functions used for managing message queues.
 * These functions must not be called under IRQ!
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>              // standard integer
#include <stdbool.h>             // standard boolean

#include "common_list.h"             // list definition
#include "kernel_config.h"           // kernel configuration
#include "ble_compiler.h"            // compiler definitions

/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Pop entry to the queue
 *
 * @param[in]  queue    Pointer to the queue.
 * @param[in]  element  Pointer to the element.
 ****************************************************************************************
 */
__INLINE void kernel_queue_push(struct common_list *const queue, struct common_list_hdr *const element)
{
    common_list_push_back(queue, element);
}

/**
 ****************************************************************************************
 * @brief Pop entry from the queue
 *
 * @param[in]  queue    Pointer to the queue.
 *
 * @return              Pointer to the element.
 ****************************************************************************************
 */
__INLINE struct common_list_hdr *kernel_queue_pop(struct common_list *const queue)
{
    return common_list_pop_front(queue);
}

/**
 ****************************************************************************************
 * @brief Extracts an element matching a given algorithm.
 *
 * @param[in]  queue    Pointer to the queue.
 * @param[in]  func     Matching function.
 * @param[in]  arg      Match argument.
 *
 * @return              Pointer to the element found and removed (NULL otherwise).
 ****************************************************************************************
 */
struct common_list_hdr *kernel_queue_extract(struct common_list * const queue,
                                 bool (*func)(struct common_list_hdr const * elmt, uint32_t arg),
                                 uint32_t arg);

/**
 ****************************************************************************************
 * @brief Insert an element in a sorted queue.
 *
 * This primitive use a comparison function from the parameter list to select where the
 * element must be inserted.
 *
 * @param[in]  queue    Pointer to the queue.
 * @param[in]  element  Pointer to the element to insert.
 * @param[in]  cmp      Comparison function (return true if first element has to be inserted
 *                      before the second one).
 *
 * @return              Pointer to the element found and removed (NULL otherwise).
 ****************************************************************************************
 */
void kernel_queue_insert(struct common_list * const queue, struct common_list_hdr * const element,
                     bool (*cmp)(struct common_list_hdr const *elementA,
                     struct common_list_hdr const *elementB));

/// @} QUEUE

#endif // _KERNEL_QUEUE_H_
