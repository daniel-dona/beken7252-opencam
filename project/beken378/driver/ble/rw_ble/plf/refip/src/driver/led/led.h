/**
 ****************************************************************************************
 *
 * @file led.h
 *
 * @brief LED driver: control and defines for simple LED initialization and control.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef LED_H_
#define LED_H_

/**
 ****************************************************************************************
 * @addtogroup LED
 * @ingroup DRIVERS
 *
 * @brief LED driver.
 *
 * This driver allows simple initialization of LED related registers and generating
 * a simple functionality for each passing in the main FW loop.
 *
 * @{
 ****************************************************************************************
 */

#include <stdint.h>          // standard integer definitions

/*
 * MACROS
 ****************************************************************************************
 */
#define led_set(index)
#define led_reset(index)
#define led_toggle(index)


/*
 * FUNCTION DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize LED.
 *
 ****************************************************************************************
 */
void led_init(void);

/**
 ****************************************************************************************
 * @brief Set LED.
 *
 * @param[in] value    LED settings
 *
 ****************************************************************************************
 */
void led_set_all(uint32_t value);

/// @} LED

#endif // LED_H_
