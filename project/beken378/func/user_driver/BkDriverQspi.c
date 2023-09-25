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
#include "include.h"
#include "bk_rtos_pub.h"
#include "BkDriverQspi.h"
#include "drv_model_pub.h"
#include "error.h"
#include "qspi_pub.h"

OSStatus bk_qspi_dcache_initialize(qspi_dcache_drv_desc *qspi_config)
{
	UINT32 param, ret;

    ret = sddev_control(QSPI_DEV_NAME, QSPI_CMD_DCACHE_CONFIG, qspi_config);
    ASSERT(QSPI_SUCCESS == ret);
		
	param = 2;
	ret = sddev_control(QSPI_DEV_NAME, QSPI_CMD_SET_VOLTAGE, &param);
    ASSERT(QSPI_SUCCESS == ret);

	ret = sddev_control(QSPI_DEV_NAME, QSPI_CMD_CLK_SET_26M, NULL);
    ASSERT(QSPI_SUCCESS == ret);
	
	param = qspi_config->clk_set;
	ret = sddev_control(QSPI_DEV_NAME, QSPI_CMD_DIV_CLK_SET,  &param);
    ASSERT(QSPI_SUCCESS == ret);
	
	if(qspi_config->mode == 0)
	{
		param = 1;
	}
	else if(qspi_config->mode == 3)
	{
		param = 4;		
	}
	ret = sddev_control(QSPI_DEV_NAME, QSPI_CMD_GPIO_CONFIG,  &param);
    ASSERT(QSPI_SUCCESS == ret);
	

    return kNoErr;
}

OSStatus bk_qspi_start(void)
{
	UINT32 ret;

	ret = sddev_control(QSPI_DEV_NAME, QSPI_DCACHE_CMD_OPEN,  NULL);
	
    return kNoErr;
}

OSStatus bk_qspi_stop(void)
{
	UINT32 ret;

	ret = sddev_control(QSPI_DEV_NAME, QSPI_DCACHE_CMD_CLOSE,  NULL);
	
    return kNoErr;
}

// eof

