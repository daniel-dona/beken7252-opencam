/**
 ******************************************************************************
 * @file    BkDriverPwm.h
 * @brief   This file provides all the headers of PWM operation functions.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2017 BEKEN Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#ifndef __BEKENDRIVERPWM_H__
#define __BEKENDRIVERPWM_H__
#include "include.h"
#include "bk_rtos_pub.h"
#include "qspi_pub.h"



/**@brief Initialises a QSPI pin
 *
 *
 * @param qspi_dcache_drv_desc        : the qspi configure which should be initialised
 * @param linemode  				  : linemode : 1 :1 linemode
 												   4: 4 linemode
 * @param div   					  : QSPI div freqence
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */

OSStatus bk_qspi_dcache_initialize(qspi_dcache_drv_desc *qspi_config);

/**@brief Start QSPI 
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */



OSStatus bk_qspi_start(void);

/**@brief Stops QSPI 
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */



OSStatus bk_qspi_stop(void);


#endif
