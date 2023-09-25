/**
****************************************************************************************
*
* @file drcoexgen.h
*
* @brief Coexistence event generator registers definitions
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/
#ifndef DR_COEXGEN_H_
#define DR_COEXGEN_H_
/**
****************************************************************************************
 * @addtogroup COEXGEN
 * @ingroup DRIVERS
 *
 * @brief Coexistence generator
 *
 * @{
****************************************************************************************
*/

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Start the WiMAX event generator with a given phase.
 *
 * @param[in] EventPhase     Time between the a Rwbtcore internal clkn event and the first
 *                           WiMAx Generator Event
 *****************************************************************************************
 */
void DR_WmGenStart(uint16_t EventPhase);
/**
 ****************************************************************************************
 * @brief Start the WLAN generator with a given period and active phase.
 *
 * @param[in] Period          Signal period
 * @param[in] ActivePhase     Length in us, during which the fr_sync is high
 *****************************************************************************************
 */
void DR_WlGenStart(uint32_t Period, uint32_t ActivePhase);
/**
 ****************************************************************************************
 * @brief Stop the WiMAX event generator.
 *
 *****************************************************************************************
 */
void DR_WmGenStop(void);
/**
 ****************************************************************************************
 * @brief Stop the WiFI event generator.
 *
 *****************************************************************************************
 */
void DR_WlGenStop(void);

/// @} COEXGEN

#endif // DR_COEXGEN_H_
