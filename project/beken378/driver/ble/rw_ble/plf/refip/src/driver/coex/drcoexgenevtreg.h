/**
****************************************************************************************
*
* @file drcoexgenevtreg.h
*
* @brief      Coexistence Event Generator registers definitions 
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/
#ifndef DR_COEXGENEVT_H_
#define DR_COEXGENEVT_H_
/**
****************************************************************************************
 * @addtogroup COEXGENEVT
 * @ingroup DRIVERS
 *
 * @brief Coexistence generator
 *
 * @{
****************************************************************************************
*/

/*
 * REGISTER and register bits definitions
 ****************************************************************************************
 */
/// Coex Event Generator registers area
#define DR_COEXGEN_REG_SIZE  (0x40/4)

/// WMGENCNTL
#define WMGENCNTL           0x00/4
#define WMGENEN             0x00000001
#define WLGENEN             0x00000002
#define WMGENSYNCEN         0x00000004
#define WMGENCLKSRC         0x00000008
#define WLGENCLKSRC         0x00000010
#define WLGENMODE           0x00000020
#define WMGENDELAY_MSK      0x000FFF00
#define WMGENDELAY_SFT               8

/// WMGENPERIOD
#define WMGENPERIOD         0x04/4
/// WMGENPERIOD
#define WMGENDCYCLE         0x08/4
/// WLGENPERIOD
#define WLGENPERIOD         0x0C/4
/// WLGENPERIOD
#define WLGENDCYCLE         0x10/4

/*
 * VARIABLE DECLARATION
 ****************************************************************************************
 */
/// Coexistence Event Generator registers
extern volatile uint32_t DR_BtCoexGenEvtRegs[DR_COEXGEN_REG_SIZE];

/// } @ COEXGENEVT

#endif // DR_COEXGENEVT_H_
