#include <rtthread.h>
#include <finsh.h>

#include "ble_pub.h"
#include "param_config.h"

#define BLE_DEBUG   1
#if BLE_DEBUG
#define BLE_DBG(...)     rt_kprintf("[BLE]"),rt_kprintf(__VA_ARGS__)
#else
#define BLE_DBG(...)
#endif

static void ble_usage(void)
{
    rt_kprintf("ble help     - Help information\n");
    rt_kprintf("ble active   - Active ble to config network\n");
}

int ble(int argc, char **argv)
{
    uint8_t mac[6];
    char ble_name[20];
	
    if ((argc == 1) || (strcmp(argv[1], "help") == 0))
    {
        ble_usage();
        return 0;
    }
    else if (strcmp(argv[1], "active") == 0)
    {
		ble_activate(NULL);
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        uint8_t adv_idx, adv_name_len;

        wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
        adv_name_len = rt_snprintf(ble_name, sizeof(ble_name), "bk-%02x%02x", mac[4], mac[5]);

        memset(&adv_info, 0x00, sizeof(adv_info));

        adv_idx = 0;
        adv_info.advData[adv_idx] = 0x02; adv_idx++;
        adv_info.advData[adv_idx] = 0x01; adv_idx++;
        adv_info.advData[adv_idx] = 0x06; adv_idx++;

        adv_info.advData[adv_idx] = adv_name_len + 1; adv_idx +=1;
        adv_info.advData[adv_idx] = 0x09; adv_idx +=1; //name
        memcpy(&adv_info.advData[adv_idx], ble_name, adv_name_len); adv_idx +=adv_name_len;

        adv_info.advDataLen = adv_idx;

        adv_idx = 0;
 
        adv_info.respData[adv_idx] = adv_name_len + 1; adv_idx +=1;
        adv_info.respData[adv_idx] = 0x08; adv_idx +=1; //name
        memcpy(&adv_info.respData[adv_idx], ble_name, adv_name_len); adv_idx +=adv_name_len;
        adv_info.respDataLen = adv_idx;

        appm_start_advertising();
    }
	
    return 0;
}


MSH_CMD_EXPORT(ble, ble command);

