#ifndef __BLE_CONFIG_H_
#define __BLE_CONFIG_H_

#define BLE_TIMEOUT         500         /* ble timeout */
#define BLE_RESPONSE_LEN    1024

typedef int (*rt_ble_netconfig_result_cb)(char *ssid, char *password, void *user_data, void *userdata_len);

enum ble_status
{
    START = 0,
    RECVING,
};

struct ble_session
{
    uint16_t len;
    char* response_buf;
    rt_tick_t tick;
    rt_ble_netconfig_result_cb result_cb;
    rt_uint8_t status;
};

rt_err_t rt_ble_netconfig_start(rt_ble_netconfig_result_cb result_cb);
rt_err_t rt_ble_netconfig_stop(void);

#endif
