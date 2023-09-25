#include <stdint.h>
#include <stdlib.h>

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <finsh.h>

#include "error.h"
#include "include.h"
#include "sys_ctrl_pub.h"
#include "drv_model_pub.h"

static void vddram(int argc,char *argv[])
{
    uint32_t param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);

    param = PSRAM_VDD_3_3V; /* default 3.3V */
    if (argc == 2)
    {
        if(strcmp(argv[1], "1.8") == 0)
        {
            rt_kprintf("VDDRAM set to 1.8\n");
            param = PSRAM_VDD_1_8V;
        }
        else if(strcmp(argv[1], "2.5") == 0)
        {
            rt_kprintf("VDDRAM set to 2.5\n");
            param = PSRAM_VDD_2_5V;
        }
        else if(strcmp(argv[1], "3.3") == 0)
        {
            rt_kprintf("VDDRAM set to 3.3\n");
            param = PSRAM_VDD_3_3V;
        }
        else
        {
            rt_kprintf("unkown %s, default 3.3V\n", argv[1]);
        }
    }
    else
    {
        rt_kprintf("Usage: vddram 1.8 [1.8 2.5 3.3], default 3.3V\n");
    }

    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_VDDRAM_VOLTAGE, &param);
}

MSH_CMD_EXPORT(vddram, vddram test);
