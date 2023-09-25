/**
 ****************************************************************************************
 *
 * @file common_utils.h
 *
 * @brief Common utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

/**
 ****************************************************************************************
 * @defgroup COMMON_UTILS Utilities
 * @ingroup COMMON
 * @brief  Common utilities
 *
 * This module contains the common utilities functions and macros.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>       // standard definitions
#include <stddef.h>       // standard definitions

#include "common_bt.h"        // common bt definitions
#include "rwip_config.h"  // SW configuration
#include "ble_compiler.h"     // for inline functions


/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */

/// Number of '1' bits in a byte
#define NB_ONE_BITS(byte)   (one_bits[byte & 0x0F] + one_bits[byte >> 4])

/// Get the number of elements within an array, give also number of rows in a 2-D array
#define ARRAY_LEN(array)   (sizeof((array))/sizeof((array)[0]))

/// Get the number of columns within a 2-D array
#define ARRAY_NB_COLUMNS(array)  (sizeof((array[0]))/sizeof((array)[0][0]))


/// Macro for LMP message handler function declaration or definition
#define LMP_MSG_HANDLER(msg_name)   __STATIC int lmp_##msg_name##_handler(struct lmp_##msg_name const *param,  \
                                                                                kernel_task_id_t const dest_id)

/// Macro for HCI message handler function declaration or definition (for multi-instanciated tasks)
#define HCI_CMD_HANDLER_C(cmd_name, param_struct)   __STATIC int hci_##cmd_name##_cmd_handler(param_struct const *param,  \
                                                                                kernel_task_id_t const dest_id,  \
                                                                                uint16_t opcode)

/// Macro for HCI message handler function declaration or definition (with parameters)
#define HCI_CMD_HANDLER(cmd_name, param_struct)   __STATIC int hci_##cmd_name##_cmd_handler(param_struct const *param,  \
                                                                                uint16_t opcode)

/// Macro for HCI message handler function declaration or definition (with parameters)
#define HCI_CMD_HANDLER_TAB(task)   __STATIC const struct task##_hci_cmd_handler task##_hci_command_handler_tab[] =


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * CONSTANT DECLARATIONS
 ****************************************************************************************
 */

/// Number of '1' bits in values from 0 to 15, used to fasten bit counting
extern const unsigned char one_bits[16];

/// Conversion table Sleep Clock Accuracy to PPM
extern const uint16_t common_sca2ppm[];

/// NULL BD address
extern const struct bd_addr common_null_bdaddr;

/// NULL BD address
extern /*const */struct bd_addr common_default_bdaddr;

/*
 * MACROS
 ****************************************************************************************
 */

/// MACRO to build a subversion field from the Minor and Release fields
#define COMMON_SUBVERSION_BUILD(minor, release)     (((minor) << 8) | (release))


/// Macro to get a structure from one of its structure field
#define CONTAINER_OF(ptr, type, member)    ((type *)( (char *)ptr - offsetof(type,member) ))

/*
 * OPERATIONS ON BT CLOCK
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Clocks addition with 2 operands
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @return      result    operation result (in BT slots)
 ****************************************************************************************
 */
#define CLK_ADD_2(clock_a, clock_b)     ((uint32_t)(((clock_a) + (clock_b)) & MAX_SLOT_CLOCK))

/**
 ****************************************************************************************
 * @brief Clocks addition with 3 operands
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @param[in]   clock_c   3rd operand value (in BT slots)
 * @return      result    operation result (in BT slots)
 ****************************************************************************************
 */
#define CLK_ADD_3(clock_a, clock_b, clock_c)     ((uint32_t)(((clock_a) + (clock_b) + (clock_c)) & MAX_SLOT_CLOCK))

/**
 ****************************************************************************************
 * @brief Clocks subtraction
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @return      result    operation result (in BT slots)
 ****************************************************************************************
 */
#define CLK_SUB(clock_a, clock_b)     ((uint32_t)(((clock_a) - (clock_b)) & MAX_SLOT_CLOCK))

/**
 ****************************************************************************************
 * @brief Clocks time difference
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @return      result    return the time difference from clock A to clock B
 *                           - result < 0  => clock_b is in the past
 *                           - result == 0 => clock_a is equal to clock_b
 *                           - result > 0  => clock_b is in the future
 ****************************************************************************************
 */
#define CLK_DIFF(clock_a, clock_b)     ( (CLK_SUB((clock_b), (clock_a)) > ((MAX_SLOT_CLOCK+1) >> 1)) ?                      \
                          ((int32_t)((-CLK_SUB((clock_a), (clock_b))))) : ((int32_t)((CLK_SUB((clock_b), (clock_a))))) )

/**
 ****************************************************************************************
 * @brief Check if an instant is passed
 *
 * @param[in]   instant   Instant reference (in BT slots)
 * @param[in]   clock     Current timestamp (in BT slots)
 * @return      result    True: clock is after or equal to the instant | False: clock is before the instant
 ****************************************************************************************
 */
#define CLK_INSTANT_PASSED(instant, clock)    ((uint32_t)(clock - instant) < (MAX_SLOT_CLOCK >> 1))

/// macro to extract a field from a value containing several fields
/// @param[in] __r bit field value
/// @param[in] __f field name
/// @return the value of the register masked and shifted
#define GETF(__r, __f)                                                           \
    (( (__r) & (__f##_MASK) ) >> (__f##_LSB))

/// macro to set a field value into a value  containing several fields.
/// @param[in] __r bit field value
/// @param[in] __f field name
/// @param[in] __v value to put in field
#define SETF(__r, __f, __v)                                                      \
    do {                                                                         \
        ASSERT_ERR( ( ( ( (__v) << (__f##_LSB) ) & ( ~(__f##_MASK) ) ) ) == 0 ); \
        __r = (((__r) & ~(__f##_MASK)) | (__v) << (__f##_LSB));                  \
    } while (0)



/// macro to extract a bit field from a value containing several fields
/// @param[in] __r bit field value
/// @param[in] __b bit field name
/// @return the value of the register masked and shifted
#define GETB(__r, __b)                                                           \
    (( (__r) & (__b##_BIT) ) >> (__b##_POS))

/// macro to set a bit field value into a value containing several fields.
/// @param[in] __r bit field value
/// @param[in] __b bit field name
/// @param[in] __v value to put in field
#define SETB(__r, __b, __v)                                                      \
    do {                                                                         \
        ASSERT_ERR( ( ( ( (__v) << (__b##_POS) ) & ( ~(__b##_BIT) ) ) ) == 0 ); \
        __r = (((__r) & ~(__b##_BIT)) | (__v) << (__b##_POS));                  \
    } while (0)



/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Read an aligned 32 bit word.
 * @param[in] ptr32 The address of the first byte of the 32 bit word.
 * @return The 32 bit value.
 ****************************************************************************************
 */
__INLINE uint32_t common_read32(void const *ptr32)
{
    return *((uint32_t*)ptr32);
}

/**
 ****************************************************************************************
 * @brief read a 8 bits word.
 * @param[in] ptr8 The address of the first byte of the 8 bits word.
 * @param[in] value The value to read.
 ****************************************************************************************
 */
__INLINE uint8_t common_read8(void const *ptr8)
{
   return *((uint16_t*)ptr8);
}

/**
 ****************************************************************************************
 * @brief Read an aligned 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @return The 16 bits value.
 ****************************************************************************************
 */
__INLINE uint16_t common_read16(void const *ptr16)
{
    return *((uint16_t*)ptr16);
}

/**
 ****************************************************************************************
 * @brief Write an aligned 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__INLINE void common_write32(void const *ptr32, uint32_t value)
{
    *(uint32_t*)ptr32 = value;
}

/**
 ****************************************************************************************
 * @brief Write an aligned 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__INLINE void common_write16(void const *ptr16, uint32_t value)
{
    *(uint16_t*)ptr16 = value;
}

/**
 ****************************************************************************************
 * @brief Write a 8 bits word.
 * @param[in] ptr8 The address of the first byte of the 8 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__INLINE void common_write8(void const *ptr8, uint32_t value)
{
    *(uint8_t*)ptr8 = value;
}

/**
 ****************************************************************************************
 * @brief Read a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @return The 16 bits value.
 ****************************************************************************************
 */
__INLINE uint16_t common_read16p(void const *ptr16)
{
    uint16_t value = ((uint8_t *)ptr16)[0] | ((uint8_t *)ptr16)[1] << 8;
    return value;
}

/**
 ****************************************************************************************
 * @brief Read a packed 24 bits word.
 * @param[in] ptr24 The address of the first byte of the 24 bits word.
 * @return The 24 bits value.
 ****************************************************************************************
 */
__INLINE uint32_t common_read24p(void const *ptr24)
{
    uint16_t addr_l, addr_h;
    addr_l = common_read16p((uint16_t *)ptr24);
    addr_h = *((uint16_t *)ptr24 + 1) & 0x00FF;
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}

/**
 ****************************************************************************************
 * @brief Write a packed 24 bits word.
 * @param[in] ptr24 The address of the first byte of the 24 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__INLINE void common_write24p(void const *ptr24, uint32_t value)
{
    uint8_t *ptr=(uint8_t*)ptr24;

    *ptr++ = (uint8_t)(value&0xff);
    *ptr++ = (uint8_t)((value&0xff00)>>8);
    *ptr++ = (uint8_t)((value&0xff0000)>>16);
}

/**
 ****************************************************************************************
 * @brief Read a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @return The 32 bits value.
 ****************************************************************************************
 */
__INLINE uint32_t common_read32p(void const *ptr32)
{
    uint16_t addr_l, addr_h;
    addr_l = common_read16p((uint16_t *)ptr32);
    addr_h = common_read16p((uint16_t *)ptr32 + 1);
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}
/**
 ****************************************************************************************
 * @brief Write a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__INLINE void common_write32p(void const *ptr32, uint32_t value)
{
    uint8_t *ptr=(uint8_t*)ptr32;

    *ptr++ = (uint8_t)(value&0xff);
    *ptr++ = (uint8_t)((value&0xff00)>>8);
    *ptr++ = (uint8_t)((value&0xff0000)>>16);
    *ptr = (uint8_t)((value&0xff000000)>>24);
}

/**
 ****************************************************************************************
 * @brief Write a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__INLINE void common_write16p(void const *ptr16, uint16_t value)
{
    uint8_t *ptr=(uint8_t*)ptr16;

    *ptr++ = value&0xff;
    *ptr = (value&0xff00)>>8;
}


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if RW_DEBUG
/**
 ****************************************************************************************
 * @brief Convert bytes to hexadecimal string
 *
 * @param[out] dest      Pointer to the destination string (must be 2x longer than input table)
 * @param[in]  src       Pointer to the bytes table
 * @param[in]  nb_bytes  Number of bytes to display in the string
 ****************************************************************************************
 */
void common_bytes_to_string(char* dest, uint8_t* src, uint8_t nb_bytes);
#endif //RW_DEBUG

/**
 ****************************************************************************************
 * @brief Compares two Bluetooth device addresses
 *
 * This function checks if the two bd address are equal.
 *
 * @param[in] bd_address1        Pointer on the first bd address to be compared.
 * @param[in] bd_address2        Pointer on the second bd address to be compared.
 *
 * @return result of the comparison (true or false).
 *
 ****************************************************************************************
 */
bool common_bdaddr_compare(struct bd_addr const *bd_address1,
                          struct bd_addr const *bd_address2);

#if (BT_EMB_PRESENT)

/**
 ******************************************************************************
 * @brief Convert an duration in baseband slot to a duration in number of ticks.
 * @param[in]  slot_cnt  Duration in number of baseband slot
 * @return  Duration (in number of ticks).
 ******************************************************************************
 */
uint32_t common_slot_to_duration(uint16_t slot_cnt);

/**
 ******************************************************************************
 * @brief Count the number of good channels in a map
 * @param[in]  map  Channel Map (bit fields for the 79 BT RF channels)
 * @return  Number of good channels
 ******************************************************************************
 */
uint8_t common_nb_good_channels(const struct chnl_map* map);

#endif //BT_EMB_PRESENT

/// @} COMMON_UTILS

#endif // _COMMON_UTILS_H_
