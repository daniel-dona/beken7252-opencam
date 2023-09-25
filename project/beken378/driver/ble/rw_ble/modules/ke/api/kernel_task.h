/**
 ****************************************************************************************
 *
 * @file kernel_task.h
 *
 * @brief This file contains the definition related to kernel task management.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_TASK_H_
#define _KERNEL_TASK_H_

/**
 ****************************************************************************************
 * @defgroup TASK Task and Process
 * @ingroup KERNEL
 * @brief Task management module.
 *
 * This module implements the functions used for managing tasks.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>          // standard integer
#include <stdbool.h>         // standard boolean

#include "kernel_msg.h"          // kernel message defines
#include "rwip_config.h"     // stack configuration
#include "ble_compiler.h"        // compiler defines, INLINE
 
/* Default Message handler code to handle several message type in same handler. */
#define KERNEL_MSG_DEFAULT_HANDLER  (0xFFFF)
/* Invalid task */
#define KERNEL_TASK_INVALID         (0xFFFF)
/* Used to know if a message is not present in kernel queue */
#define KERNEL_MSG_NOT_IN_QUEUE     ((struct common_list_hdr *) 0xFFFFFFFF)

/// Status of kernel_task API functions
enum KERNEL_TASK_STATUS
{
    KERNEL_TASK_OK = 0,
    KERNEL_TASK_FAIL,
    KERNEL_TASK_UNKNOWN,
    KERNEL_TASK_CAPA_EXCEEDED,
    KERNEL_TASK_ALREADY_EXISTS,
};


#define MSG_T(msg)         ((kernel_task_id_t)((msg) >> 8))
#define MSG_I(msg)         ((msg) & ((1<<8)-1))

/// Format of a task message handler function
typedef int (*kernel_msg_func_t)(kernel_msg_id_t const msgid, void const *param,
                             kernel_task_id_t const dest_id, kernel_task_id_t const src_id);

/// Macro for message handler function declaration or definition
#define KERNEL_MSG_HANDLER(msg_name, param_struct)   __STATIC int msg_name##_handler(kernel_msg_id_t const msgid,     \
                                                                                param_struct const *param,  \
                                                                                kernel_task_id_t const dest_id, \
                                                                                kernel_task_id_t const src_id)

/// Macro for message handlers table declaration or definition
#define KERNEL_MSG_HANDLER_TAB(task)   __STATIC const struct kernel_msg_handler task##_default_state[] =

/// Macro for state handler declaration or definition
#define KERNEL_MSG_STATE(task) const struct kernel_state_handler task##_default_handler = KERNEL_STATE_HANDLER(task##_default_state);

/// Element of a message handler table.
struct kernel_msg_handler
{
    /// Id of the handled message.
    kernel_msg_id_t id;
    /// Pointer to the handler function for the msgid above.
    kernel_msg_func_t func;
};

/// Element of a state handler table.
struct kernel_state_handler
{
    /// Pointer to the message handler table of this state.
    const struct kernel_msg_handler *msg_table;
    /// Number of messages handled in this state.
    uint16_t msg_cnt;
};

/// Helps writing the initialization of the state handlers without errors.
#define KERNEL_STATE_HANDLER(hdl) {hdl, sizeof(hdl)/sizeof(struct kernel_msg_handler)}

/// Helps writing empty states.
#define KERNEL_STATE_HANDLER_NONE {NULL, 0}

/// Task descriptor grouping all information required by the kernel for the scheduling.
struct kernel_task_desc
{
    /// Pointer to the state handler table (one element for each state).
    const struct kernel_state_handler* state_handler;
    /// Pointer to the default state handler (element parsed after the current state).
    const struct kernel_state_handler* default_handler;
    /// Pointer to the state table (one element for each instance).
    kernel_state_t* state;
    /// Maximum number of states in the task.
    uint16_t state_max;
    /// Maximum index of supported instances of the task.
    uint16_t idx_max;
};

/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialize Kernel task module.
 ****************************************************************************************
 */
void kernel_task_init(void);

/**
 ****************************************************************************************
 * @brief Create a task.
 *
 * @param[in]  task_type       Task type.
 * @param[in]  p_task_desc     Pointer to task descriptor.
 *
 * @return                     Status
 ****************************************************************************************
 */
uint8_t kernel_task_create(uint8_t task_type, struct kernel_task_desc const * p_task_desc);

/**
 ****************************************************************************************
 * @brief Delete a task.
 *
 * @param[in]  task_type       Task type.
 *
 * @return                     Status
 ****************************************************************************************
 */
uint8_t kernel_task_delete(uint8_t task_type);

/**
 ****************************************************************************************
 * @brief Retrieve the state of a task.
 *
 * @param[in]  id   Task id.
 *
 * @return          Current state of the task
 ****************************************************************************************
 */
kernel_state_t kernel_state_get(kernel_task_id_t const id);

/**
 ****************************************************************************************
 * @brief Set the state of the task identified by its Task Id.
 *
 * In this function we also handle the SAVE service: when a task state changes we
 * try to activate all the messages currently saved in the save queue for the given
 * task identifier.
 *
 * @param[in]  id          Identifier of the task instance whose state is going to be modified
 * @param[in]  state_id    New State
 *
 ****************************************************************************************
 */
void kernel_state_set(kernel_task_id_t const id, kernel_state_t const state_id);

/**
 ****************************************************************************************
 * @brief Generic message handler to consume message without handling it in the task.
 *
 * @param[in] msgid Id of the message received (probably unused)
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id TaskId of the receiving task.
 * @param[in] src_id TaskId of the sending task.
 *
 * @return KERNEL_MSG_CONSUMED
 ****************************************************************************************
 */
int kernel_msg_discard(kernel_msg_id_t const msgid, void const *param,
                   kernel_task_id_t const dest_id, kernel_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Generic message handler to consume message without handling it in the task.
 *
 * @param[in] msgid Id of the message received (probably unused)
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id TaskId of the receiving task.
 * @param[in] src_id TaskId of the sending task.
 *
 * @return KERNEL_MSG_CONSUMED
 ****************************************************************************************
 */
int kernel_msg_save(kernel_msg_id_t const msgid, void const *param,
                kernel_task_id_t const dest_id, kernel_task_id_t const src_id);



/**
 ****************************************************************************************
 * @brief This function flushes all messages, currently pending in the kernel for a
 * specific task.
 *
 * @param[in] task The Task Identifier that shall be flushed.
 ****************************************************************************************
 */
void kernel_task_msg_flush(kernel_task_id_t task);


/**
 ****************************************************************************************
 * @brief Check validity of a task. If task type or task instance does not exist,
 * return invalid task
 *
 * @param[in] task Task Identifier to check.
 *
 * @return Task identifier if valid, invalid identifier else.
 ****************************************************************************************
 */
kernel_task_id_t kernel_task_check(kernel_task_id_t task);

/// @} TASK

#endif // _KERNEL_TASK_H_

