#ifndef __DRV_WLAN_FAST_CONNECT_H__
#define __DRV_WLAN_FAST_CONNECT_H__

#define FLASH_FAST_DATA_ADDR        (0x1FF000)

struct wlan_fast_connect
{
    unsigned char ssid[32];
    unsigned char bssid[6];
    unsigned char channel;
    unsigned char security;
    unsigned char psk[32];
};
typedef struct wlan_fast_connect wlan_fast_connect_t;

int wlan_fast_connect_info_write(wlan_fast_connect_t *data_info);
int wlan_fast_connect_info_read(wlan_fast_connect_t *data_info);
int wlan_fast_connect_info_erase(void);

#endif
