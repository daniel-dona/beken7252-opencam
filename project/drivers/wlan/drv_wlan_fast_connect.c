#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "drv_wlan.h"
#include "drv_flash.h"
#include "drv_wlan_fast_connect.h"
#include "stdio.h"

#ifdef PKG_USING_EASYFLASH
#include "easyflash.h"
#endif

extern uint16_t inet_chksum(void *dataptr, uint16_t len);
static int hex2byte(const char *hex);
static uint32_t hex2num(char c);

void flash_data_info_dump(wlan_fast_connect_t *data_info)
{
    char tmp[68];
    int i = 0;

    if (data_info == RT_NULL)
    {
        return;
    }

    memcpy(tmp, data_info->ssid, 32);
    tmp[32] = '\0';
    rt_kprintf("ssid:%s \n", tmp);

    for (i = 0; i < 6; i ++)
    {
        sprintf(&tmp[i * 3], "%02X:",  data_info->bssid[i]);
    }
    tmp[6 * 3 - 1] = '\0';

    rt_kprintf("bssid:%s \n", tmp);
    rt_kprintf("channel:%d \n", data_info->channel);
    rt_kprintf("security:%d \n", data_info->security);
    {
        for (i = 0; i < 32; i ++)
        {
            sprintf(&tmp[i * 2], "%02x",  data_info->psk[i]);
        }
        tmp[64] = '\0';
        rt_kprintf("psk = %s \n", tmp);
    }
}

int fast_connect_dump(void)
{
    struct wlan_fast_connect data_info;

    memset(&data_info, 0, sizeof(struct wlan_fast_connect));
    wlan_fast_connect_info_read(&data_info);
    flash_data_info_dump(&data_info);
}
MSH_CMD_EXPORT(fast_connect_dump, fast_connect_dump);

int wlan_fast_connect_info_write(wlan_fast_connect_t *data_info)
{
#ifdef PKG_USING_EASYFLASH
    uint32_t len;
    struct wlan_fast_connect old_data_info;
    int result = RT_EOK;
    char* ssid = RT_NULL;
    char* bssid = RT_NULL;
    char* channel = RT_NULL;
    char* security = RT_NULL;
    char* psk = RT_NULL;
    char tmp[32*2+1] = {0};
    char ef_bssid[6+1] = {0};
    char ef_psk[32+1] = {0};
    int i = 0;
    int a = 0;

    if(data_info == RT_NULL)
    {
        rt_kprintf("[%s] - [%d] wifi fast connect info data_info null\n", __FUNCTION__, __LINE__);
        return -RT_EINVAL;
    }

    len = sizeof(struct wlan_fast_connect);
	memset(&old_data_info, 0, len);
    ssid = ef_get_env("fc_ssid");
    bssid = ef_get_env("fc_bssid");

    //str to num
    if(bssid != RT_NULL)
    {
        for(i = 0; i < 6; i++)
        {
            a = hex2byte(bssid);
            if(a < 0)
            {
                break;
            }
            ef_bssid[i] = a;
            bssid += 2;
        }
    }
    
    channel = ef_get_env("fc_channel");
    security = ef_get_env("fc_security");
    psk = ef_get_env("fc_psk");

    if(psk != RT_NULL)
    {
        for(i = 0; i < 32; i++)
        {
            a = hex2byte(psk);
            if(a < 0)
            {
                break;
            }
            ef_psk[i] = a;
            psk += 2;
        }
    }
    memcpy(old_data_info.ssid, ssid, strlen(ssid));
    memcpy(old_data_info.bssid, ef_bssid, (sizeof(ef_bssid) - 1));
    old_data_info.channel = atoi(channel);
    old_data_info.security = atoi(security);
    memcpy(old_data_info.psk, ef_psk, (sizeof(ef_psk) - 1));

    //wirte it to flash if different content: SSID, Passphrase, Channel, Security type 
    if((memcmp(&old_data_info, data_info, sizeof(struct wlan_fast_connect)) != 0))
    {
        rt_kprintf("write new profile to flash 0x%08X %d byte!\n", FLASH_FAST_DATA_ADDR, len);
        ef_set_env("fc_ssid", data_info->ssid);
        memset(tmp, 0x0, sizeof(tmp));
        rt_sprintf(tmp, "%02x%02x%02x%02x%02x%02x", data_info->bssid[0], 
                                                data_info->bssid[1],
                                                data_info->bssid[2],
                                                data_info->bssid[3],
                                                data_info->bssid[4],
                                                data_info->bssid[5]); 
        ef_set_env("fc_bssid", tmp);

        memset(tmp, 0x0, sizeof(tmp));
        sprintf(&tmp[0], "%d", data_info->channel);
        ef_set_env("fc_channel", &tmp[0]);

        memset(tmp, 0x0, sizeof(tmp));
        sprintf(&tmp[0], "%d", data_info->security);
        ef_set_env("fc_security", &tmp[0]);

        memset(tmp, 0x0, sizeof(tmp));
        for(int i = 0; i < 32; i++)
        {
            sprintf(&tmp[i * 2], "%02x", data_info->psk[i]);
        }
        ef_set_env("fc_psk", &tmp[0]);
        ef_print_env();
        ef_save_env();
    }
    else
    {
        rt_kprintf("wlan_fast_connect data not modfiy, skip write to flash!\n");
    }

    // flash_data_info_dump(data_info);
    return RT_EOK;
#else
    uint32_t crc1, crc2, len;
    uint32_t data[(sizeof(struct wlan_fast_connect) + 4 - 1) / 4 + 1];
    int result = RT_EOK;

    if (data_info == RT_NULL)
    {
        return -RT_EINVAL;
    }

    memset(data, 0, sizeof(data));
    len = sizeof(struct wlan_fast_connect);
    beken_flash_read(FLASH_FAST_DATA_ADDR, data, sizeof(data));

    crc1 = inet_chksum(data_info, sizeof(struct wlan_fast_connect));
    crc2 = data[(sizeof(struct wlan_fast_connect) + 4 - 1 ) / 4];
    //wirte it to flash if different content: SSID, Passphrase, Channel, Security type
    if ((memcmp(data, (uint8_t *) data_info, sizeof(struct wlan_fast_connect)) != 0) || (crc1 != crc2))
    {
        rt_kprintf("write new profile to flash 0x%08X %d byte!\n", FLASH_FAST_DATA_ADDR, len);

        memcpy(data, data_info, sizeof(struct wlan_fast_connect));
        data[(sizeof(struct wlan_fast_connect) + 4 - 1 ) / 4] = crc1;

        beken_flash_erase(FLASH_FAST_DATA_ADDR);
        beken_flash_write(FLASH_FAST_DATA_ADDR, (uint8_t *)data, len + 4);
    }
    else
    {
        rt_kprintf("wlan_fast_connect data not modfiy, skip write to flash!\n");
    }

    // flash_data_info_dump(data_info);

    return RT_EOK;
#endif /* PKG_USING_EASYFLASH */
    
}

int wlan_fast_connect_info_read(wlan_fast_connect_t *data_info)
{
#ifdef PKG_USING_EASYFLASH
    struct wlan_fast_connect old_data_info;
    char* ssid = RT_NULL;
    char* bssid = RT_NULL;
    char* channel = RT_NULL;
    char* security = RT_NULL;
    char* psk = RT_NULL;
    char ef_bssid[6+1] = {0};
    char ef_psk[32+1] = {0};
    int i = 0;
    int a = 0;
    int result = RT_EOK;

    if(data_info == RT_NULL)
    {
        return -RT_EINVAL;
    }

    ssid = ef_get_env("fc_ssid");
    bssid = ef_get_env("fc_bssid");
    channel = ef_get_env("fc_channel");
    security = ef_get_env("fc_security");
    psk = ef_get_env("fc_psk");
    if(ssid == RT_NULL || bssid == RT_NULL || channel == RT_NULL || security == RT_NULL || psk == RT_NULL)
    {
        return -RT_ERROR;
    }

    for(i = 0; i < 6; i++)
    {
		a = hex2byte(bssid);
		if(a < 0)
        {
            break;
        }	
		ef_bssid[i] = a;
		bssid += 2;
	}
    
    for(i = 0; i < 32; i++)
    {
        a = hex2byte(psk);
        if(a < 0)
        {
            break;
        }
        ef_psk[i] = a;
        psk += 2;
    }
    memcpy(data_info->ssid, ssid, strlen(ssid));
    memcpy(data_info->bssid, ef_bssid, (sizeof(ef_bssid) - 1));
    data_info->channel = atoi(channel);
    data_info->security = atoi(security);
    memcpy(data_info->psk, ef_psk, (sizeof(ef_psk) - 1));

	// check
	if((data_info->channel < 1) || (data_info->channel > 13))
	{
		rt_kprintf("[%s] channel out of range! \n", __FUNCTION__, (int)data_info->channel);
		result = -RT_EINVAL;
	}

    // rt_kprintf("========================%s=============\n", data);
    return result;
#else
    uint32_t crc1, crc2;
    uint32_t data[(sizeof(struct wlan_fast_connect) + 4 - 1) / 4 + 1];
    int result = RT_EOK;

    if (data_info == RT_NULL)
    {
        return -RT_EINVAL;
    }

    memset(data, 0, sizeof(struct wlan_fast_connect) + 4);
    beken_flash_read(FLASH_FAST_DATA_ADDR, data, sizeof(data));
    crc1 = data[(sizeof(struct wlan_fast_connect) + 4 - 1 ) / 4];
    crc2 = inet_chksum(data, sizeof(struct wlan_fast_connect));

    if ((data[0] != ~0x0)  && (crc1 == crc2)) // 0xFFFFFFFF
    {
        memcpy(data_info, data, sizeof(struct wlan_fast_connect));
        // flash_data_info_dump(data_info);
        rt_kprintf("crc1:%X crc2:%X\n", crc1, crc2);
        result = RT_EOK;
    }
    else
    {
        rt_kprintf("fast_connect ap info crc failed, crc1:%X crc2:%X \n", crc1, crc2);
        result = -RT_ERROR;
    }

	// check
	if( (data_info->channel < 1) || (data_info->channel > 13) )
	{
		rt_kprintf("[%s] channel out of range! \n", __FUNCTION__, (int)data_info->channel);
		result = -RT_EINVAL;
	}

    return result;
#endif /* PKG_USING_EASYFLASH */
   
}

int wlan_fast_connect_info_erase(void)
{
#ifdef PKG_USING_EASYFLASH
    // ef_env_set_default();
#else
    beken_flash_erase(FLASH_FAST_DATA_ADDR);
#endif
}

static uint32_t hex2num(char c)
{
    // rt_kprintf("%X\n", c);
    if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}
