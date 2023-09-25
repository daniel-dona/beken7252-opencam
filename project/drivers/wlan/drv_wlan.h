#ifndef __DRV_WLAN_H__
#define __DRV_WLAN_H__

#define MAX_ADDR_LEN            6

enum STATION_MODE
{
    NORMAL_MODE = 0,
    ADVANCED_MODE = 1,
};

enum CONNECT_STATE
{
    CONNECT_DONE = 0,
    CONNECT_DOING = 1,
    CONNECT_FAILED = 2,
};

struct beken_wifi_info
{
    rt_uint8_t mac[MAX_ADDR_LEN];
    rt_uint8_t state;       /* 0:done 1:doding 2:failed */
    rt_uint8_t mode;        /* 0:normal 1:advanced */
};

struct netif *wlan_get_sta_netif(void);
struct netif *wlan_get_uap_netif(void);

#endif
