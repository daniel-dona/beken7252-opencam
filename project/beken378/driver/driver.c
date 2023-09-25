#include "include.h"
#include "driver_pub.h"
#include "dd_pub.h"
#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"

UINT32 driver_init(void)
{    
    uint32_t param;
    drv_model_init();
    g_dd_init();

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);

    param = PSRAM_VDD_3_3V_DEF;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_VDDRAM_VOLTAGE, &param);
    
    return 0;
}

// eof
