#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "airkiss.h"
#include <stdio.h>
#include <sys/socket.h>
#include "lwip/sockets.h"
#define AIRKISS_PRINTF rt_kprintf

#define AIRKISS_SWITCH_TIMER    rt_tick_from_millisecond(50)    // ms
#define AIRKISS_DOING_TIMER     rt_tick_from_millisecond(20000)  // 20s
#define MAX_CHANNEL_NUM         14

static airkiss_context_t *ak_contex = RT_NULL;
const airkiss_config_t ak_conf =
{
    (airkiss_memset_fn) &rt_memset,
    (airkiss_memcpy_fn) &rt_memcpy,
    (airkiss_memcmp_fn) &rt_memcmp,
    (airkiss_printf_fn) &rt_kprintf,
};

static rt_timer_t g_switch_timer;
static rt_timer_t g_doing_timer;
static struct rt_wlan_device *g_wlan_device = RT_NULL;
static volatile uint8_t g_current_channel;
static int airkiss_recv_ret;
static rt_sem_t g_cfg_done_sem;
static airkiss_result_t result;

static void airkiss_switch_channel(void *parameter)
{
    g_current_channel++;
    if (g_current_channel > MAX_CHANNEL_NUM)
    {
        g_current_channel = 1;
    }
    rt_wlan_set_channel(g_wlan_device, g_current_channel);
    airkiss_change_channel(ak_contex);
    AIRKISS_PRINTF("Switch channel %d \n", g_current_channel);
}

static void airkiss_doing_timeout(void *parameter)
{
    AIRKISS_PRINTF("airkiss_doing_timeout, restart channel switch timer\r\n");
    rt_timer_start(g_switch_timer);
}

static void airkiss_monitor_callback(uint8_t *data, int len, void *user_data)
{
    airkiss_recv_ret = airkiss_recv(ak_contex, data, len);
    if (airkiss_recv_ret == AIRKISS_STATUS_CHANNEL_LOCKED)
    {
        rt_timer_stop(g_switch_timer);
        AIRKISS_PRINTF("Lock channel in %d \n", g_current_channel);
        rt_timer_start(g_doing_timer);
    }
    else if (airkiss_recv_ret == AIRKISS_STATUS_COMPLETE)
    {
        rt_timer_stop(g_doing_timer);
        rt_sem_release(g_cfg_done_sem);
    }
}

static int get_wifi_status(struct netif *netif)
{
    ip_addr_t ip_addr;
    int result = 0;

    ip_addr_set_zero(&ip_addr);
    if (ip_addr_cmp(&(netif->ip_addr), &ip_addr))
        result = 0;
    else
    {
        result = 1;
        rt_kprintf("Got IP address : %s\n", ipaddr_ntoa(&(netif->ip_addr)));
    }

    return result;
}

static struct rt_wlan_info g_wlan_info;

static int station_connect(char *ssid, char *passwd)
{
    rt_err_t result = RT_EOK;
    struct rt_wlan_info info;
    struct rt_wlan_device *wlan;
    rt_tick_t tick = 0;

    wlan = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_STA_NAME);
    if (!wlan)
    {
        rt_kprintf("no wlan:%s device\n", WIFI_DEVICE_STA_NAME);
        return -1;
    }

    result = rt_wlan_init(wlan, WIFI_STATION);
    rt_wlan_info_init(&info, WIFI_STATION, SECURITY_WPA2_AES_PSK, ssid);
    result = rt_wlan_connect(wlan, &info, passwd);
    rt_wlan_info_deinit(&info);

    return result;
}

static void airkiss_send_notification_thread(void *parameter)
{
    int sock = -1;
    int udpbufsize = 2;
    uint8_t random = (uint32_t)parameter;
    struct sockaddr_in g_stUDPBCAddr, g_stUDPBCServerAddr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        AIRKISS_PRINTF("notify create socket error!\n");
        goto _exit;
    }

    g_stUDPBCAddr.sin_family = AF_INET;
    g_stUDPBCAddr.sin_port = htons(10000);
    g_stUDPBCAddr.sin_addr.s_addr = htonl(0xffffffff);
    rt_memset(&(g_stUDPBCAddr.sin_zero), 0, sizeof(g_stUDPBCAddr.sin_zero));

    g_stUDPBCServerAddr.sin_family = AF_INET;
    g_stUDPBCServerAddr.sin_port = htons(10000);
    g_stUDPBCServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(g_stUDPBCServerAddr.sin_zero), 0, sizeof(g_stUDPBCServerAddr.sin_zero));

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &udpbufsize, sizeof(int)) != 0)
    {
        AIRKISS_PRINTF("notify socket setsockopt error\n");
        goto _exit;
    }

    if (bind(sock, (struct sockaddr *)&g_stUDPBCServerAddr, sizeof(g_stUDPBCServerAddr)) != 0)
    {
        AIRKISS_PRINTF("notify socket bind error\n");
        goto _exit;
    }

    for (int i = 0; i <= 20; i++)
    {
        int ret = sendto(sock, (char *)&random, 1, 0, (struct sockaddr *)&g_stUDPBCAddr, sizeof(g_stUDPBCAddr));
        rt_thread_delay(10);
    }

    AIRKISS_PRINTF("airkiss notification thread exit!\n");

_exit:
    if (sock >= 0)
    {
        close(sock);
    }
}

uint32_t airkiss_get_status(void)
{
	return airkiss_recv_ret;
}

airkiss_result_t *airkiss_result_get(void)
{
	return &result;
}

static void airkiss_thread_entry(void *parameter)
{
	int ret;

    g_switch_timer = rt_timer_create("switch_channel",
                                     airkiss_switch_channel,
                                     RT_NULL,
                                     AIRKISS_SWITCH_TIMER,
                                     RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    if (!g_switch_timer)
    {
        rt_kprintf("Create airkiss swtich channel timer failed \n");
        goto _exit;
    }

    g_doing_timer = rt_timer_create("doing_timeout",
                                    airkiss_doing_timeout,
                                    RT_NULL,
                                    AIRKISS_DOING_TIMER,
                                    RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
    if (!g_doing_timer)
    {
        rt_kprintf("Create airkiss doing timeout timer failed \n");
        goto _exit;
    }

    g_cfg_done_sem = rt_sem_create("tlink", 0, RT_IPC_FLAG_FIFO);
    if (!g_cfg_done_sem)
    {
        rt_kprintf("Create  airkiss config done sem failed! \n");
        goto _exit;
    }

    ak_contex = (airkiss_context_t *)rt_malloc(sizeof(airkiss_context_t));
    if (!ak_contex)
    {
        rt_kprintf("Malloc memory for airkiss context \n");
        goto _exit;
    }

    ret = airkiss_init(ak_contex, &ak_conf);
    if (ret != RT_EOK)
    {
        rt_kprintf("Airkiss init failed!!\r\n");
        goto _exit;
    }

    AIRKISS_PRINTF("Airkiss version: %s\r\n", airkiss_version());

    g_wlan_device = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_STA_NAME);
    if (g_wlan_device == RT_NULL)
    {
        rt_kprintf("Device not found\n");
        return;
    }

    g_current_channel = 1;
    rt_wlan_set_channel(g_wlan_device, g_current_channel);
    rt_wlan_set_monitor_callback(g_wlan_device, airkiss_monitor_callback);
    rt_wlan_cfg_monitor(g_wlan_device, WIFI_MONITOR_START);

    rt_timer_start(g_switch_timer);

    if (rt_sem_take(g_cfg_done_sem, rt_tick_from_millisecond(1000 * 90)) != RT_EOK)
    {
        AIRKISS_PRINTF("Wait semaphore timeout \n");
    }
	
    if (airkiss_recv_ret == AIRKISS_STATUS_COMPLETE)
    {
        int8_t err;
        int8_t tick = 0;

        err = airkiss_get_result(ak_contex, &result);
        if (err == 0)
        {
            AIRKISS_PRINTF("airkiss_get_result() ok!\n");
            AIRKISS_PRINTF(" ssid = %s \n pwd = %s \n, ssid_length = %d \n pwd_length = %d \n, random = 0x%02x\r\n",
                           result.ssid, result.pwd, result.ssid_length, result.pwd_length, result.random);
        }

        rt_wlan_cfg_monitor(g_wlan_device, WIFI_MONITOR_STOP);
        rt_wlan_set_monitor_callback(g_wlan_device, RT_NULL);

        station_connect(result.ssid, result.pwd);
        do
        {
            tick ++;
            rt_thread_delay(rt_tick_from_millisecond(1000));
            if (tick >= 30)
            {
                rt_kprintf("GET IP Time Out!!! \n");
                goto _exit;
            }

        }while (!get_wifi_status(g_wlan_device->parent.netif));

        {
            rt_thread_t tid;

            tid = rt_thread_create("air_echo",
                                   airkiss_send_notification_thread, (void *)result.random,
                                   1536, RT_THREAD_PRIORITY_MAX - 3, 20);
            if (tid != RT_NULL)
            {
                rt_thread_startup(tid);
            }
        }
    }

_exit:
    if (g_switch_timer)
    {
        rt_timer_stop(g_switch_timer);
        rt_timer_delete(g_switch_timer);
    }
    if (g_doing_timer)
    {
        rt_timer_stop(g_doing_timer);
        rt_timer_delete(g_doing_timer);
    }
    if (ak_contex != RT_NULL)
    {
        rt_free(ak_contex);
        ak_contex = RT_NULL;
    }

    if (g_cfg_done_sem)
    {
        rt_sem_delete(g_cfg_done_sem);
        g_cfg_done_sem = 0;
    }
}

int airkiss(void)
{
	
	int result = 0;
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("airkiss",
                           airkiss_thread_entry,
                           RT_NULL,
                           1024 * 4,
                           20,
                           10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
		result = 1;
    }
	return result;
}

int start_airkiss(int argc, char *argv[])
{
	airkiss_result_t *result;
	if(1 == airkiss())
		rt_kprintf("airkiss start\r\n");
	else
		rt_kprintf("airkiss fail\r\n");

	while(airkiss_get_status() != AIRKISS_STATUS_COMPLETE)
	{
		rt_thread_delay(rt_tick_from_millisecond(100));	
	}

	result = airkiss_result_get();

	rt_kprintf("---ssid:%s , key:%s---\r\n", result->ssid,result->pwd);
}
#ifdef FINSH_USING_MSH
#include "finsh.h"

MSH_CMD_EXPORT(start_airkiss, start_ariksss);

#endif