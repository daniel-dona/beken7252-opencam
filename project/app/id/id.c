#include "id.h"
#include "param_config.h"


DeviceInfo *get_device(void){

    uint8_t mac[6];
    wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);

    DeviceInfo *device_info = malloc(sizeof(DeviceInfo));
    memset(device_info->name, 0, 33);

    snprintf(device_info->name, sizeof(device_info->name), "opencam-%02x%02x%02x", mac[3], mac[4], mac[5]);

    return device_info;

}
