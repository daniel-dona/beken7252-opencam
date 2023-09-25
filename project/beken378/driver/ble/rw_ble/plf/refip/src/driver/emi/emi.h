/**
 ****************************************************************************************
 *
 * @file emi.h
 *
 * @brief EMI initialization API
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef EMI_H_
#define EMI_H_

/**
 ****************************************************************************************
 * @addtogroup EMI
 * @ingroup DRIVERS
 * @brief EMI initialization
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

#define IF_TYPE_8_BITS          0
#define IF_TYPE_16_BITS         1
#define IF_TYPE_32_BITS         2

#define SETUP_WR                1
#define HOLD_WR                 1
#define SETUP_RD                1
#define HOLD_RD                 1

#define BOOT_ZONE               0
#define SWAP_CS                 1
#define SINGLE_CS               2

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialize and configure the External Memory Interface .
 *
 * This function configures the EMI according to the system needs.
 *
 ****************************************************************************************
 */
void emi_init(void);
/// @} EMI

#endif // EMI_H_
