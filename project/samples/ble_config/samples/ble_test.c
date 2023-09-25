#include "rtthread.h"

#include "ble_config.h"
#include "finsh.h"

int result_cb(char *ssid, char *password, void *user_data, void *userdata_len)
{
    rt_kprintf("ssid : %s, password : %s\n", ssid, password);

    return RT_EOK;
}

void ble_config(void)
{
    rt_ble_netconfig_start(result_cb);
}
MSH_CMD_EXPORT(ble_config, ble_config);

void ble_config_stop(void)
{
    rt_ble_netconfig_stop();
}
MSH_CMD_EXPORT(ble_config_stop, ble_config_stop);
