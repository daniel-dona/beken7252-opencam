/**
 ****************************************************************************************
 *
 * @file kernel_msg.h
 *
 * @brief This file contains the definition related to message scheduling.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_MSG_H_
#define _KERNEL_MSG_H_

/**
 ****************************************************************************************
 * @defgroup MSG Message Exchange
 * @ingroup KERNEL
 * @brief Message scheduling module.
 *
 * The MSG module implements message scheduling functions.

 * A kernel message has an ID, a receiver task ID and a source task ID.
 * In most cases, it also has parameters which are defined in
 * a structure dynamically embedded in the message structure,
 * so the whole message will be managed internally as one block.
 *
 * A message can also have one extra parameter which is referenced
 * in the normal parameter structure. This extra block is assumed
 * to be large by the kernel and will be moved by DMA if needed.
 * This feature allows moving MMPDU from LMAC to UMAC.
 *
 * In order to send a message, a function first have to allocate
 * the memory for this message. It can be done with the wrapper
 * macro KERNEL_MSG_ALLOC() (which will call kernel_msg_alloc()).

 * The message can then be sent with kernel_msg_send(). The kernel
 * will take care of freeing the allocated memory.

 * If the message has no parameters, the kernel_msg_send_basic() function
 * can be used.
 *
 * @{
 ****************************************************************************************
 */

#include <stddef.h>          // standard definition
#include <stdint.h>          // standard integer
#include <stdbool.h>         // standard boolean

#include "common_list.h"         // list definition
#include "architect.h"            // architectural definition
#include "kernel_config.h"       // kernel configuration
#include "ble_compiler.h"        // compiler definition

/// Task Identifier. Composed by the task type and the task index.
typedef uint16_t kernel_task_id_t;

/// Builds the task identifier from the type and the index of that task.
#define KERNEL_BUILD_ID(type, index) ( (kernel_task_id_t)(((index) << 8)|(type)) )

/// Retrieves task type from task id.
#define KERNEL_TYPE_GET(kernel_task_id) ((kernel_task_id) & 0xFF)

/// Retrieves task index number from task id.
#define KERNEL_IDX_GET(kernel_task_id) (((kernel_task_id) >> 8) & 0xFF)

/// Task State
typedef uint8_t kernel_state_t;

/// Message Identifier. The number of messages is limited to 0xFFFF.
/// The message ID is divided in two parts:
/// - bits[15..10] : task index (no more than 64 tasks supported).
/// - bits[9..0] : message index (no more that 1024 messages per task).
typedef uint16_t kernel_msg_id_t;

/// Message structure.
struct kernel_msg
{
    struct common_list_hdr hdr;     ///< List header for chaining

    kernel_msg_id_t     id;         ///< Message id.
    kernel_task_id_t    dest_id;    ///< Destination kernel identifier.
    kernel_task_id_t    src_id;     ///< Source kernel identifier.
    uint16_t        param_len;  ///< Parameter embedded struct length.
    uint32_t        param[1];   ///< Parameter embedded struct. Must be word-aligned.
};


/// Status returned by a task when handling a message
enum kernel_msg_status_tag
{
    KERNEL_MSG_CONSUMED = 0, ///< consumed, msg and ext are freed by the kernel
    KERNEL_MSG_NO_FREE,      ///< consumed, nothing is freed by the kernel
    KERNEL_MSG_SAVED,        ///< not consumed, will be pushed in the saved queue
};

/**
 ****************************************************************************************
 * @brief Convert a parameter pointer to a message pointer
 *
 * @param[in]  param_ptr Pointer to the parameter member of a kernel_msg
 *                       Usually retrieved by a kernel_msg_alloc()
 *
 * @return The pointer to the kernel_msg
 ****************************************************************************************
 */
__INLINE struct kernel_msg * kernel_param2msg(void const *param_ptr)
{
    return (struct kernel_msg*) (((uint8_t*)param_ptr) - offsetof(struct kernel_msg, param));
}

/**
 ****************************************************************************************
 * @brief Convert a message pointer to a parameter pointer
 *
 * @param[in]  msg Pointer to the kernel_msg.
 *
 * @return The pointer to the param member
 ****************************************************************************************
 */
__INLINE void * kernel_msg2param(struct kernel_msg const *msg)
{
    return (void*) (((uint8_t*) msg) + offsetof(struct kernel_msg, param));
}

/**
 ****************************************************************************************
 * @brief Convenient wrapper to kernel_msg_alloc()
 *
 * This macro calls kernel_msg_alloc() and cast the returned pointer to the
 * appropriate structure. Can only be used if a parameter structure exists
 * for this message (otherwise, use kernel_msg_send_basic()).
 *
 * @param[in] id        Message identifier
 * @param[in] dest      Destination Identifier
 * @param[in] src       Source Identifier
 * @param[in] param_str parameter structure tag
 *
 * @return Pointer to the parameter member of the kernel_msg.
 ****************************************************************************************
 */
#define KERNEL_MSG_ALLOC(id, dest, src, param_str) \
    (struct param_str*) kernel_msg_alloc(id, dest, src, sizeof(struct param_str))

/**
 ****************************************************************************************
 * @brief Convenient wrapper to kernel_msg_free()
 *
 * This macro calls kernel_msg_free() with the appropriate msg pointer as parameter, according
 * to the message parameter pointer passed.
 *
 * @param[in] param_ptr parameter structure pointer
 ****************************************************************************************
 */
#define KERNEL_MSG_FREE(param_ptr) kernel_msg_free(kernel_param2msg((param_ptr)))

/**
 ****************************************************************************************
 * @brief Convenient wrapper to kernel_msg_alloc()
 *
 * This macro calls kernel_msg_alloc() and cast the returned pointer to the
 * appropriate structure with a variable length. Can only be used if a parameter structure exists
 * for this message (otherwise, use kernel_msg_send_basic()).Can only be used if the data array is
 * located at the end of the structure.
 *
 * @param[in] id        Message identifier
 * @param[in] dest      Destination Identifier
 * @param[in] src       Source Identifier
 * @param[in] param_str parameter structure tag
 * @param[in] length    length for the data
 *
 * @return Pointer to the parameter member of the kernel_msg.
 ****************************************************************************************
 */
#define KERNEL_MSG_ALLOC_DYN(id, dest, src, param_str,length)  (struct param_str*)kernel_msg_alloc(id, dest, src, \
    (sizeof(struct param_str) + (length)));

/**
 ****************************************************************************************
 * @brief Allocate memory for a message
 *
 * This primitive allocates memory for a message that has to be sent. The memory
 * is allocated dynamically on the heap and the length of the variable parameter
 * structure has to be provided in order to allocate the correct size.
 *
 * Several additional parameters are provided which will be preset in the message
 * and which may be used internally to choose the kind of memory to allocate.
 *
 * The memory allocated will be automatically freed by the kernel, after the
 * pointer has been sent to kernel_msg_send(). If the message is not sent, it must
 * be freed explicitly with kernel_msg_free().
 *
 * Allocation failure is considered critical and should not happen.
 *
 * @param[in] id        Message identifier
 * @param[in] dest_id   Destination Task Identifier
 * @param[in] src_id    Source Task Identifier
 * @param[in] param_len Size of the message parameters to be allocated
 *
 * @return Pointer to the parameter member of the kernel_msg. If the parameter
 *         structure is empty, the pointer will point to the end of the message
 *         and should not be used (except to retrieve the message pointer or to
 *         send the message)
 ****************************************************************************************
 */
void *kernel_msg_alloc(kernel_msg_id_t const id, kernel_task_id_t const dest_id,
                   kernel_task_id_t const src_id, uint16_t const param_len);

/**
 ****************************************************************************************
 * @brief Message sending.
 *
 * Send a message previously allocated with any kernel_msg_alloc()-like functions.
 *
 * The kernel will take care of freeing the message memory.
 *
 * Once the function have been called, it is not possible to access its data
 * anymore as the kernel may have copied the message and freed the original
 * memory.
 *
 * @param[in] param_ptr  Pointer to the parameter member of the message that
 *                       should be sent.
 ****************************************************************************************
 */

void kernel_msg_send(void const *param_ptr);

/**
 ****************************************************************************************
 * @brief Basic message sending.
 *
 * Send a message that has a zero length parameter member. No allocation is
 * required as it will be done internally.
 *
 * @param[in] id        Message identifier
 * @param[in] dest_id   Destination Identifier
 * @param[in] src_id    Source Identifier
 ****************************************************************************************
 */
void kernel_msg_send_basic(kernel_msg_id_t const id, kernel_task_id_t const dest_id, kernel_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Message forwarding.
 *
 * Forward a message to another task by changing its destination and source tasks IDs.
 *
 * @param[in] param_ptr  Pointer to the parameter member of the message that
 *                       should be sent.
 * @param[in] dest_id New destination task of the message.
 * @param[in] src_id New source task of the message.
 ****************************************************************************************
 */
void kernel_msg_forward(void const *param_ptr, kernel_task_id_t const dest_id, kernel_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Message forwarding.
 *
 * Forward a message to another task by changing its message ID and its destination and source tasks IDs.
 *
 * @param[in] param_ptr  Pointer to the parameter member of the message that
 *                       should be sent.
 * @param[in] msg_id  New ID of the message.
 * @param[in] dest_id New destination task of the message.
 * @param[in] src_id  New source task of the message.
 ****************************************************************************************
 */
void kernel_msg_forward_new_id(void const *param_ptr,
                           kernel_msg_id_t const msg_id, kernel_task_id_t const dest_id, kernel_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Free allocated message
 *
 * @param[in] msg   Pointer to the message to be freed (not the parameter member!)
 ****************************************************************************************
 */
void kernel_msg_free(struct kernel_msg const *param);



/**
 ****************************************************************************************
 * @brief Retrieve destination task identifier of a kernel message
 *
 * @param[in] param_ptr  Pointer to the parameter member of the message.
 *
 * @return message destination task
 ****************************************************************************************
 */
kernel_msg_id_t kernel_msg_dest_id_get(void const *param_ptr);



/**
 ****************************************************************************************
 * @brief Retrieve source task identifier of a kernel message
 *
 * @param[in] param_ptr  Pointer to the parameter member of the message.
 *
 * @return message source task
 ****************************************************************************************
 */
kernel_msg_id_t kernel_msg_src_id_get(void const *param_ptr);

/**
 * Used to know if message is present in kernel queue or not.
 *
 * @param[in]  param_ptr  Pointer to the parameter member of the message.
 *
 * @return True if message is present in Kernel Queue, False else.
 */
bool kernel_msg_in_queue(void const *param_ptr);
/// @} MSG

#endif // _KERNEL_MSG_H_
