#include <rtthread.h>
#include <finsh.h>

#include "ble_pub.h"
#include "param_config.h"
#include "cJSON.h"
#include "ble_config.h"

#define str_begin_with(s, prefix)       (strstr(s, prefix) == s)
#define str_end_with(buf, len, ending)  (strstr(buf+len-1, ending) == buf+len-1)

static struct ble_session _ble_session = {0}, *ble_session = &_ble_session;

static void ble_session_dump(void)
{
    rt_kprintf("ble session len:%d\n", ble_session->len);
    rt_kprintf("ble session buf:%s\n",ble_session->response_buf);
    rt_kprintf("ble status:%d\n", ble_session->status);
}
MSH_CMD_EXPORT(ble_session_dump, ble_session_dump);

static rt_err_t ble_push_data(char* buf, uint16_t len)
{
    if(ble_session->len + len > BLE_RESPONSE_LEN)
    {
        rt_kprintf("data len is too small\n");
        return -RT_ERROR;
    }
    memcpy(ble_session->response_buf + ble_session->len, buf, len);
    ble_session->len += len;

    return RT_EOK;
}

static void ble_clean_data(void)
{
    memset(ble_session->response_buf, 0x0, BLE_RESPONSE_LEN);
    ble_session->len = 0;
}

static void ble_receive_cb(void *buf, uint16_t len)
{
    cJSON *root = RT_NULL;
    cJSON *ssid = RT_NULL;
    cJSON *password = RT_NULL;

    if(ble_session->status == START)
    {
        ble_session->status = RECVING;
        ble_push_data(buf, len);
        ble_session->tick = rt_tick_get();
        return;
    }
    else if(ble_session->status == RECVING)
    {
        /* timeout */
        if((rt_tick_get() - ble_session->tick) > BLE_TIMEOUT)
        {
            rt_kprintf("ble recv timeout\n");
            ble_clean_data();
            if(str_begin_with(buf, "{") != RT_NULL)
            {
                ble_push_data(buf, len);
                ble_session->tick = rt_tick_get();
                // ble_response->status = RECVING;
                // goto __exit;
            }
            else
            {
                 ble_session->status = START;
                 ble_session->result_cb(RT_NULL, RT_NULL, RT_NULL, RT_NULL);
                 goto __restart;
            }
        }
        else
        {
            /* not timeout */
            ble_push_data(buf, len);
            ble_session->tick = rt_tick_get();
        }
    }

    if(str_end_with(buf, len, "}"))
    {
        root = cJSON_Parse(ble_session->response_buf);
        if(root == RT_NULL)
        {
            rt_kprintf("ble data parse failed\n");
            ble_session->result_cb(RT_NULL, RT_NULL, RT_NULL, RT_NULL);
            ble_session->status = START;
            goto __restart;
        }
        else
        {
            ssid = cJSON_GetObjectItem(root, "ssid");
            password = cJSON_GetObjectItem(root, "passwd");
            // rt_kprintf("ssid : %s, passwd : %s\n", ssid->valuestring, password->valuestring);
            ble_session->result_cb(ssid->valuestring, password->valuestring, RT_NULL, RT_NULL);
            ble_session->status = START;
            goto __restart;
        }
    }
    else
    {
        /* continue receive */
        ble_session->status = RECVING;
        goto __exit;
    }
    
__restart:
    if(root != RT_NULL)
    {
        cJSON_Delete(root);
        root = RT_NULL;
    }
    ble_clean_data();
__exit:
    return;    
}

static void ble_start(void)
{
    uint8_t adv_idx, adv_name_len;
    uint8_t mac[6];
    char ble_name[20];

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

    app_btl_set_recive_cb(ble_receive_cb);
    appm_start_advertising();
}

rt_err_t rt_ble_netconfig_start(rt_ble_netconfig_result_cb result_cb)
{
    static rt_uint8_t is_start = 0;

    if(is_start == 1)
    {
        rt_kprintf("ble is already init\n");
        return RT_EOK;
    }

    ble_session->response_buf = rt_malloc(1024);
    if(ble_session->response_buf == RT_NULL) 
    {
        rt_kprintf("malloc failed\n");
        return -RT_ENOMEM;
    }
    ble_clean_data();
    ble_session->len = 0;
    ble_session->tick = 0;
    ble_session->result_cb = *result_cb;
    ble_session->status = START;
    
    //ble activate
    ble_activate(NULL);
    rt_thread_delay(3000);
    //ble start advertise
    ble_start();
    
    is_start = 1;
    return RT_EOK;
}

rt_err_t rt_ble_netconfig_stop(void)
{
    appm_stop_advertising();
    
    return RT_EOK;
}
