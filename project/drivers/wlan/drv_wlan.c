#include "include.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "ethernetif.h"

#include <stdio.h>
#include <string.h>

#include "netif/etharp.h"

#include "lwip_netif_address.h"

#include "sa_station.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#include "common.h"

#include "hostapd_cfg.h"

#include "sk_intf.h"
#include "rw_pub.h"
#include "error.h"
#include "bk_rtos_pub.h"
#include "param_config.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#include "uart_pub.h"

#include "ieee802_11_defs.h"
#include "wlan_ui_pub.h"
#include "net_param_pub.h"

#define DRV_WLAN_DEBUG   1
#if DRV_WLAN_DEBUG
#define DRV_WLAN_DBG(...)     rt_kprintf("[DRV_WLAN]"),rt_kprintf(__VA_ARGS__)
#else
#define DRV_WLAN_DBG(...)
#endif


#define ETH_INTF_DEBUG
#ifdef ETH_INTF_DEBUG
#define ETH_INTF_PRT      warning_prf
#define ETH_INTF_WARN     warning_prf
#define ETH_INTF_FATAL    fatal_prf
#else
#define ETH_INTF_PRT      null_prf
#define ETH_INTF_WARN     null_prf
#define ETH_INTF_FATAL    null_prf
#endif

extern void *net_get_sta_handle(void);
extern void *net_get_uap_handle(void);
extern void wifi_get_mac_address(char *mac, u8 type);
extern void *net_get_netif_handle(uint8_t iface);

extern int bmsg_tx_sender(struct pbuf *p, uint32_t vif_idx);

//#define MINI_DUMP
//#define ETH_RX_DUMP
//#define ETH_TX_DUMP

#if defined(ETH_RX_DUMP) ||  defined(ETH_TX_DUMP)
static void packet_dump(const char *msg, const struct pbuf *p)
{
    const struct pbuf *q;
    rt_uint32_t i, j;
    rt_uint8_t *ptr;

    rt_kprintf("%s %d byte\n", msg, p->tot_len);

#ifdef MINI_DUMP
    return;
#endif

    i = 0;
    for (q = p; q != RT_NULL; q = q->next)
    {
        ptr = q->payload;

        for (j = 0; j < q->len; j++)
        {
            if ((i % 8) == 0)
            {
                rt_kprintf("  ");
            }
            if ((i % 16) == 0)
            {
                rt_kprintf("\r\n");
            }
            rt_kprintf("%02x ", *ptr);

            i++;
            ptr++;
        }
    }

    rt_kprintf("\n\n");
}
#endif /* dump */

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    int ret;
    err_t err = ERR_OK;
    uint8_t vif_idx = rwm_mgmt_get_netif2vif(netif);

#ifdef ETH_TX_DUMP
    packet_dump("TX dump", p);
#endif /* ETH_TX_DUMP */

    if (!netif_is_link_up(netif))
    {
        return ERR_IF;
    }

    ret = bmsg_tx_sender(p, (uint32_t)vif_idx);
    if (0 != ret)
    {
        err = ERR_TIMEOUT;
    }

    return err;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(int iface, struct pbuf *p)
{
    struct eth_hdr *ethhdr;
    struct netif *netif;

#ifdef ETH_RX_DUMP
    packet_dump("RX dump", p);
#endif /* ETH_RX_DUMP */

    if (p->len <= SIZEOF_ETH_HDR)
    {
        pbuf_free(p);
        return;
    }

    netif = rwm_mgmt_get_vif2netif((uint8_t)iface);
    if (!netif)
    {
        ETH_INTF_PRT("ethernetif_input no netif found %d\r\n", iface);
        pbuf_free(p);
        p = NULL;
        return;
    }

    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;

    switch (htons(ethhdr->type))
    {
    /* IP or ARP packet? */
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
#if PPPOE_SUPPORT
    /* PPPoE packet? */
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
        /* full packet send to tcpip_thread to process */
        if (netif->input(p, netif) != ERR_OK)    // ethernet_input
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\r\n"));
            pbuf_free(p);
            p = NULL;
        }
        break;

    case ETHTYPE_EAPOL:
        ke_l2_packet_tx(p->payload, p->len, iface);
        pbuf_free(p);
        p = NULL;
        break;

    default:
        pbuf_free(p);
        p = NULL;
        break;
    }

}
/**
 * RT-Thread LwIP Interface
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "drv_flash.h"
#include "drv_wlan.h"
#include "drv_wlan_fast_connect.h"
#define RT_WLAN_DEVICE(eth) (struct rt_wlan_device *)(eth)
static rt_err_t _wifi_easyjoin(rt_device_t dev, void *passwd);

#ifdef RT_USING_WLAN_STA
static struct rt_wlan_device _g_sta_device;
static struct beken_wifi_info _g_sta_info;
#endif

#ifdef RT_USING_WLAN_AP
static struct rt_wlan_device _g_ap_device;
static struct beken_wifi_info _g_ap_info;
#endif

#define RT_WLAN_SSID_MAX_LEN 32
#define SCAN_WAIT_OUT_TIME 10000
static rt_sem_t _g_scan_done_sem;
int start_connect_tick = 0;
int end_connect_tick = 0;

struct netif *wlan_get_sta_netif(void)
{
#ifdef RT_USING_WLAN_STA
    return _g_sta_device.parent.netif;
#else
    return RT_NULL;
#endif
}

struct netif *wlan_get_uap_netif(void)
{
#ifdef RT_USING_WLAN_AP
    return _g_ap_device.parent.netif;
#else
    return RT_NULL;
#endif
}

static int wlan_event_handle(enum rw_evt_type event, void *data)
{
    if ((event < 0) || (event > RW_EVT_MAX))
    {
        return -RT_ERROR;
    }

    switch (event)
    {
    case RW_EVT_STA_CONNECTED:
        end_connect_tick = rt_tick_get();
        rt_kprintf("[wlan_connect]:start tick =  %d, connect done tick = %d, total = %d \n", start_connect_tick, end_connect_tick, end_connect_tick - start_connect_tick);
        rt_wlan_indicate_event_handle(&_g_sta_device, WIFI_EVT_STA_CONNECTED, RT_NULL);
        break;

    case RW_EVT_STA_DISCONNECTED:
        rt_wlan_indicate_event_handle(&_g_sta_device, WIFI_EVT_STA_DISCONNECTED, RT_NULL);
        break;

    case RW_EVT_STA_CONNECT_FAILED:
        if (_g_sta_info.mode == ADVANCED_MODE)
        {
            wlan_fast_connect_info_erase();
        }
        rt_wlan_indicate_event_handle(&_g_sta_device, WIFI_EVT_STA_CONNECT_FAILED, RT_NULL);
        break;

#ifdef RT_USING_WLAN_AP
    case RW_EVT_AP_CONNECTED:
        rt_wlan_indicate_event_handle(&_g_ap_device, WIFI_EVT_AP_ASSOCIATED, RT_NULL);
        break;

    case RW_EVT_AP_DISCONNECTED:
        rt_wlan_indicate_event_handle(&_g_ap_device, WIFI_EVT_AP_DISASSOCIATED, RT_NULL);
        break;

    case RW_EVT_AP_CONNECT_FAILED:
        rt_wlan_indicate_event_handle(&_g_ap_device, WIFI_EVT_AP_ASSOCIATE_FAILED, RT_NULL);
        break;
#endif

    default:
        break;
    }

    return RT_EOK;
}
static void scan_ap_callback(void *ctxt, uint8_t param)
{
    if (_g_scan_done_sem)
    {
        rt_sem_release(_g_scan_done_sem);
        DRV_WLAN_DBG("release scan done semaphore \n");
    }
}

static int rt_wlan_malloc_scan_result(struct rt_wlan_scan_result **scan_result, int num)
{
    struct rt_wlan_scan_result *_scan_result;
    int i;
    int result = RT_EOK;

    _scan_result = rt_malloc(sizeof(struct rt_wlan_scan_result));
    if (_scan_result == RT_NULL)
    {
        rt_kprintf("rt_wlan_scan_result malloc failed!\r\n");
        result = -RT_ERROR;
        goto _exit;
    }
    rt_memset(_scan_result, 0, sizeof(struct rt_wlan_scan_result));

    _scan_result->ap_num = num;
    _scan_result->ap_table = rt_malloc(sizeof(struct rt_wlan_info) * num);
    if (_scan_result->ap_table == RT_NULL)
    {
        rt_kprintf("rt_scan_rst table malloc failed!\r\n");
        result = -RT_ERROR;
        goto _exit;
    }
    rt_memset(_scan_result->ap_table, 0, sizeof(struct rt_wlan_info) * num);

    for (i = 0; i < num; i++)
    {
        _scan_result->ap_table[i].ssid = (char *)rt_malloc(RT_WLAN_SSID_MAX_LEN + 1);
        if (_scan_result->ap_table[i].ssid == RT_NULL)
        {
            rt_kprintf("malloc memory for scan ssid failed \n");
        }
        rt_memset(_scan_result->ap_table[i].ssid, 0, RT_WLAN_SSID_MAX_LEN + 1);
    }
    *scan_result = _scan_result;

    return RT_EOK;
_exit:
    for (i = 0; i < num; i++)
    {
        if (_scan_result->ap_table[i].ssid)
        {
            rt_free(_scan_result->ap_table[i].ssid);
            _scan_result->ap_table[i].ssid = RT_NULL;
        }
    }
    if (_scan_result->ap_table)
    {
        rt_free(_scan_result->ap_table);
        _scan_result->ap_table = RT_NULL;
    }
    if (_scan_result)
    {
        rt_free(_scan_result);
        _scan_result = RT_NULL;
    }
    *scan_result = RT_NULL;

    return -RT_ERROR;
}

static const char *wlan_sec_type_string[] =
{
    "NONE",
    "WEP",
    "WPA-TKIP",
    "WPA-AES",
    "WPA2-TKIP",
    "WPA2-AES",
    "WPA2-MIX",
    "AUTO"
};

int wlan_scan_done_handler(struct rt_wlan_scan_result **scan_result)
{
    struct rt_wlan_scan_result *_scan_result;
    struct sta_scan_res *scan_rst_table;
    char scan_rst_ap_num = 0;
    int i;

    scan_rst_ap_num = bk_wlan_get_scan_ap_result_numbers();
    if (scan_rst_ap_num == 0)
    {
        rt_kprintf("NULL AP \r\n");
        return -RT_ERROR;
    }

    scan_rst_table = (struct sta_scan_res *)rt_malloc(sizeof(struct sta_scan_res) * scan_rst_ap_num);
    if (scan_rst_table == RT_NULL)
    {
        rt_kprintf("scan_rst_table malloc failed!\r\n");
        return -RT_ERROR;
    }

    bk_wlan_get_scan_ap_result(scan_rst_table, scan_rst_ap_num);

    if (rt_wlan_malloc_scan_result(scan_result, scan_rst_ap_num) != RT_EOK)
    {
        rt_kprintf("malloc memory for scan failed \n");
        return -RT_ERROR;
    }
    _scan_result = *scan_result;

    rt_kprintf("\r\n");
    for (i = 0; i < scan_rst_ap_num; i++)
    {
        strncpy(_scan_result->ap_table[i].ssid, scan_rst_table[i].ssid, RT_WLAN_SSID_MAX_LEN);
        rt_memcpy(_scan_result->ap_table[i].bssid, scan_rst_table[i].bssid, 6);
        _scan_result->ap_table[i].channel = scan_rst_table[i].channel;

        rt_kprintf("\033[36;22m ssid: %-32.*s  security: %-s\r\n", 32, scan_rst_table[i].ssid, wlan_sec_type_string[scan_rst_table[i].security]);

        switch (scan_rst_table[i].security)
        {
        case SECURITY_TYPE_NONE:
            _scan_result->ap_table[i].security = SECURITY_OPEN;
            break;

        case SECURITY_TYPE_WEP:
            _scan_result->ap_table[i].security = SECURITY_WEP_PSK;
            break;

        case SECURITY_TYPE_WPA_TKIP:
            _scan_result->ap_table[i].security = SECURITY_WPA_TKIP_PSK;
            break;

        case SECURITY_TYPE_WPA_AES:
            _scan_result->ap_table[i].security = SECURITY_WPA_AES_PSK;
            break;

        case SECURITY_TYPE_WPA2_TKIP:
            _scan_result->ap_table[i].security = SECURITY_WPA2_TKIP_PSK;
            break;

        case SECURITY_TYPE_WPA2_AES:
            _scan_result->ap_table[i].security = SECURITY_WPA2_AES_PSK;
            break;

        case SECURITY_TYPE_WPA2_MIXED:
            _scan_result->ap_table[i].security = SECURITY_WPA2_MIXED_PSK;
            break;

        case SECURITY_TYPE_AUTO:
            // _scan_result.ap_table[i]->security = SECURITY_WEP_PSK;
            break;
        default:
            break;
        }
        _scan_result->ap_table[i].rssi = scan_rst_table[i].level;
    }
    rt_kprintf("\033[0m\r\n");

    if (scan_rst_table != NULL)
    {
        os_free(scan_rst_table);
        scan_rst_table = NULL;
    }
}

static rt_err_t _wifi_easyjoin(rt_device_t dev, void *passwd)
{
    network_InitTypeDef_st wNetConfig;
    struct rt_wlan_device *wlan = RT_NULL;
    const char *ssid = RT_NULL;
    int len;

    rt_kprintf("[wifi_connect]: normal connect \n");
    _g_sta_info.mode = NORMAL_MODE;
    _g_sta_info.state = CONNECT_DOING;
    wlan = RT_WLAN_DEVICE(dev);
    ssid = wlan->info->ssid;
    rt_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    if (ssid == NULL)
    {
        rt_kprintf("ssid is null\r\n");
        return -RT_ERROR;
    }

    len = rt_strlen(ssid);
    if (SSID_MAX_LEN < len)
    {
        rt_kprintf("ssid name more than 32 Bytes\r\n");
        return -RT_ERROR;
    }

    rt_strncpy((char *)wNetConfig.wifi_ssid, ssid, sizeof(wNetConfig.wifi_ssid));

    if (passwd == NULL)
    {
        rt_memset(wNetConfig.wifi_key, 0, sizeof(wNetConfig.wifi_key));
    }
    else
    {
        if (sizeof(wNetConfig.wifi_key) < rt_strlen(passwd))
        {
            rt_kprintf("wifi key is more than %d Bytes\r\n", sizeof(wNetConfig.wifi_key));
            return -RT_ERROR;
        }
        rt_strncpy((char *)wNetConfig.wifi_key, passwd, sizeof(wNetConfig.wifi_key));
    }

    wNetConfig.wifi_mode = STATION;
    wNetConfig.dhcp_mode = DHCP_CLIENT;
    wNetConfig.wifi_retry_interval = 100;

    rt_kprintf("_wifi_easyjoin: ssid:%.*s key:%.*s\r\n", sizeof(wNetConfig.wifi_ssid), wNetConfig.wifi_ssid, sizeof(wNetConfig.wifi_key), wNetConfig.wifi_key);
    bk_wlan_start(&wNetConfig);

    return RT_EOK;
}

static rt_err_t _wifi_softap(rt_device_t dev, void *passwd)
{
    network_InitTypeDef_st wNetConfig;
    struct rt_wlan_device *wlan = RT_NULL;
    const char *ssid = RT_NULL;
    int len;

    wlan = RT_WLAN_DEVICE(dev);
    ssid = wlan->info->ssid;
    rt_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    if (ssid == NULL)
    {
        rt_kprintf("ssid is null\r\n");
        return -RT_ERROR;
    }

    len = rt_strlen(ssid);
    if (SSID_MAX_LEN < len)
    {
        rt_kprintf("ssid name more than 32 Bytes\r\n");
        /* continue to use 32 bytes ssid, do not return err */
        len = SSID_MAX_LEN;
    }
    rt_strncpy((char *)wNetConfig.wifi_ssid, ssid, sizeof(wNetConfig.wifi_ssid));

    if (passwd == NULL)
    {
        rt_memset(wNetConfig.wifi_key, 0, sizeof(wNetConfig.wifi_key));
    }
    else
    {
        if (sizeof(wNetConfig.wifi_key) < rt_strlen(passwd))
        {
            rt_kprintf("wifi key is more than %d Bytes\r\n", sizeof(wNetConfig.wifi_key));
            return -RT_ERROR;
        }
        rt_strncpy((char *)wNetConfig.wifi_key, passwd, sizeof(wNetConfig.wifi_key));
    }

    wNetConfig.wifi_mode = SOFT_AP;
    wNetConfig.dhcp_mode = DHCP_SERVER;
    wNetConfig.wifi_retry_interval = 100;

    rt_kprintf("_wifi_softap: ssid:%.*s key:%.*s\r\n", sizeof(wNetConfig.wifi_ssid), wNetConfig.wifi_ssid, sizeof(wNetConfig.wifi_key), wNetConfig.wifi_key);
    bk_wlan_start(&wNetConfig);

    return RT_EOK;
}

static int _wifi_disconnect(rt_device_t dev)
{
    struct rt_wlan_device *wlan = RT_NULL;
    rt_wlan_mode_t mode;

    wlan = RT_WLAN_DEVICE(dev);
    mode = wlan->info->mode;

    if (mode == WIFI_STATION)
    {
        bk_wlan_stop(STATION);
    }
    else if (mode == WIFI_AP)
    {
        bk_wlan_stop(SOFT_AP);
    }

    return RT_EOK;
}

static int _wifi_cfg_monitor(rt_device_t dev, void *opition)
{
    struct rt_wlan_device *wlan = RT_NULL;
    rt_wlan_monitor_opition_t *opt = RT_NULL;
    int result = RT_EOK;

    RT_ASSERT(opition != RT_NULL);

    wlan = RT_WLAN_DEVICE(dev);
    opt = (rt_wlan_monitor_opition_t *)opition;
    if (*opt == WIFI_MONITOR_START)
    {
        DRV_WLAN_DBG("start monitor \n");
        bk_wlan_start_monitor();
    }
    else if (*opt == WIFI_MONITOR_STOP)
    {
        DRV_WLAN_DBG("stop monitor \n");
        bk_wlan_stop_monitor();
    }

    return result;
}

static int _wifi_advanced_connect(rt_device_t dev, void *passwd)
{
    struct rt_wlan_device *wlan = RT_NULL;
    struct wlan_fast_connect ap_info;
    const char *ssid = RT_NULL;

    wlan = RT_WLAN_DEVICE(dev);
    ssid = wlan->info->ssid;
    memset(&ap_info, 0, sizeof(struct wlan_fast_connect));

    if (wlan_fast_connect_info_read(&ap_info) == RT_EOK)
    {
        /* adv connect */
        if (rt_strcmp(ssid, ap_info.ssid) == 0)
        {
            network_InitTypeDef_adv_st  wNetConfigAdv;

            rt_kprintf("[wifi_connect]: fast connect \n");
            _g_sta_info.mode = ADVANCED_MODE;
            _g_sta_info.state = CONNECT_DOING;
            os_memset(&wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st));
            rt_strncpy(wNetConfigAdv.ap_info.ssid, ap_info.ssid, 32);
            memcpy(wNetConfigAdv.ap_info.bssid, ap_info.bssid, 6);
            wNetConfigAdv.ap_info.security = ap_info.security;
            wNetConfigAdv.ap_info.channel = ap_info.channel;
            {
                int i = 0;
                char tmp[68];
                memset(tmp, 0, sizeof(tmp));
                for (i = 0; i < 32; i ++)
                {
                    sprintf(&tmp[i * 2], "%02x",  ap_info.psk[i]);
                }
                rt_kprintf("psk = %s \n", tmp);
                memcpy(wNetConfigAdv.key, tmp, 64);
                wNetConfigAdv.key_len =  64; // strnlen(passwd, 32);
            }
            wNetConfigAdv.dhcp_mode = DHCP_CLIENT;
            wNetConfigAdv.wifi_retry_interval = 100;
            bk_wlan_start_sta_adv(&wNetConfigAdv);
        }
        else
        {
            rt_kprintf("[wifi_connect]: ssid != ap_info.ssid \n");
            _wifi_easyjoin(dev, passwd);
        }
    }
    else
    {
        /* normal connect */
        rt_kprintf("[wifi_connect]: read ap_info is empty \n");
        _wifi_easyjoin(dev, passwd);
    }
}

static int _wifi_connect(rt_device_t dev, void *passwd)
{
    struct rt_wlan_device *wlan = RT_NULL;
    struct beken_wifi_info *wifi_info = RT_NULL;

    wlan = RT_WLAN_DEVICE(dev);
    wifi_info = (struct beken_wifi_info *)wlan->user_data;

    if (wifi_info->state == CONNECT_DONE)
    {
        _wifi_advanced_connect(dev, passwd);
    }
    else
    {
        _wifi_easyjoin(dev, passwd);
    }

    return 0;
}

extern int wpa_get_psk(char *psk);
int _wifi_connect_done(void *ctx)
{
    LinkStatusTypeDef link_status;
    struct wlan_fast_connect ap_info;

    memset(&link_status, 0, sizeof(LinkStatusTypeDef));
    memset(&ap_info, 0, sizeof(struct wlan_fast_connect));
    if (_g_sta_info.mode == NORMAL_MODE)
    {
        if ((bk_wlan_get_link_status(&link_status) == kNoErr) && (SECURITY_TYPE_WEP != link_status.security))
        {
            memcpy(ap_info.ssid, link_status.ssid, strnlen(link_status.ssid, 32));
            memcpy(ap_info.bssid, link_status.bssid, 6);
            ap_info.channel = link_status.channel;
            ap_info.security = link_status.security;
            wpa_get_psk(ap_info.psk);
            wlan_fast_connect_info_write(&ap_info);
        }
    }
    _g_sta_info.state = CONNECT_DONE;

    return 0;
}

extern int bk_wlan_dtim_rf_ps_timer_start(void);
extern int bk_wlan_dtim_rf_ps_timer_pause(void);
static int _wifi_power_manager(int level)
{
 switch (level)
 {
    case 0:
    {
        #if CFG_USE_MCU_PS
        /* disable cpu sleep */
        bk_wlan_mcu_ps_mode_disable();
        #endif
        #if CFG_USE_STA_PS
        /* disable rf sleep */
        bk_wlan_dtim_rf_ps_disable_send_msg();
        /* pause rf timer */
        bk_wlan_dtim_rf_ps_timer_pause();
        #endif
        break;
    }

    case 1:
    {
        #if CFG_USE_MCU_PS
        /* enable cpu sleep */
        bk_wlan_mcu_ps_mode_enable();
        #endif
        #if CFG_USE_STA_PS
        /* disable rf sleep */
        bk_wlan_dtim_rf_ps_disable_send_msg();
        /* pause rf timer */
        bk_wlan_dtim_rf_ps_timer_pause();
        #endif
        break;
    }

	case 2:
    {
        #if CFG_USE_MCU_PS
        /* disable cpu sleep */
        bk_wlan_mcu_ps_mode_disable();
        #endif
        #if CFG_USE_STA_PS
        /* enable rf sleep */
        bk_wlan_dtim_rf_ps_mode_enable();
        /* start rf timer */
        bk_wlan_dtim_rf_ps_timer_start();
        #endif
        break;
    }

    case 3:
    {
        #if CFG_USE_MCU_PS
    	/* enable cpu sleep */
        bk_wlan_mcu_ps_mode_enable();
        #endif
        #if CFG_USE_STA_PS
        /* enable rf sleep */
        bk_wlan_dtim_rf_ps_mode_enable();
        /* start rf timer */
        bk_wlan_dtim_rf_ps_timer_start();
        #endif
        break;
    }


    default:
        break;
 }
}

#if LWIP_IPV4 && LWIP_IGMP
static err_t igmp_mac_filter(struct netif *netif, const ip4_addr_t *ip4_addr, u8_t action)
{
    uint8_t mac[6];
    const uint8_t *p = (const uint8_t *)ip4_addr;

    mac[0] = 0x01;
    mac[1] = 0x00;
    mac[2] = 0x5E;
    mac[3] = *(p + 1) & 0x7F;
    mac[4] = *(p + 2);
    mac[5] = *(p + 3);

    if (1)
    {
        rt_kprintf("%s %s %s ", __FUNCTION__, (action == NETIF_ADD_MAC_FILTER) ? "add" : "del", ip4addr_ntoa(ip4_addr));
        rt_kprintf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    //wifi_add_mcast_filter(mac);

    return 0;
}
#endif /* LWIP_IPV4 && LWIP_IGMP */

#if LWIP_IPV6 && LWIP_IPV6_MLD
static err_t mld_mac_filter(struct netif *netif, const ip6_addr_t *ip6_addr, u8_t action)
{
    uint8_t mac[6];
    const uint8_t *p = (const uint8_t *)&ip6_addr->addr[3];

    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = *(p + 0);
    mac[3] = *(p + 1);
    mac[4] = *(p + 2);
    mac[5] = *(p + 3);

    if (1)
    {
        rt_kprintf("%s %s %s ", __FUNCTION__, (action == NETIF_ADD_MAC_FILTER) ? "add" : "del", ip6addr_ntoa(ip6_addr));
        rt_kprintf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    //wifi_add_mcast_filter(mac);

    return 0;
}
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

static rt_err_t beken_wlan_init(rt_device_t dev)
{
    struct eth_device *eth = (struct eth_device *)dev;

#if LWIP_IPV4 && LWIP_IGMP
    netif_set_igmp_mac_filter(eth->netif, igmp_mac_filter);
#endif /* LWIP_IPV4 && LWIP_IGMP */

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif_set_mld_mac_filter(eth->netif, mld_mac_filter);
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    /* Initialize semaphore for scan */
    _g_scan_done_sem = rt_sem_create("scan_done", 0, RT_IPC_FLAG_FIFO);

    return RT_EOK;
}

static rt_err_t beken_wlan_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t beken_wlan_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t beken_wlan_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_size_t beken_wlan_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t beken_wlan_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_wlan_device *wlan = RT_NULL;
    struct beken_wifi_info *wifi_info = RT_NULL;
    rt_err_t result = RT_EOK;

    wlan = RT_WLAN_DEVICE(dev);
    wifi_info = (struct beken_wifi_info *)wlan->user_data;
    RT_ASSERT(wlan != RT_NULL);
    RT_ASSERT(wifi_info != RT_NULL);

    switch (cmd)
    {
    case NIOCTL_GADDR:
    {
        /* get MAC address */
        if (args)
            rt_memcpy(args, wifi_info->mac, 6);
        else
            result =  -RT_ERROR;
        break;
    }

    case WIFI_INIT:
    {
        DRV_WLAN_DBG("%s L%d %s cmd: case WIFI_INIT!\r\n", __FILE__, __LINE__, __FUNCTION__);
        result = RT_EOK;
        break;
    }

    case WIFI_SCAN:
    {
        DRV_WLAN_DBG("%s L%d %s cmd: case WIFI_SCAN!\r\n", __FILE__, __LINE__, __FUNCTION__);

        bk_wlan_scan_ap_reg_cb(scan_ap_callback);
        bk_wlan_start_scan();
        if (rt_sem_take(_g_scan_done_sem, rt_tick_from_millisecond(SCAN_WAIT_OUT_TIME)) != RT_EOK)
        {
            DRV_WLAN_DBG("Wait scan_done semaphore timeout \n");
        }
        wlan_scan_done_handler((struct rt_wlan_scan_result **)args);
        rt_wlan_indicate_event_handle(wlan, WIFI_EVT_SCAN_DONE, args);
        break;
    }

    case WIFI_GET_RSSI:
    {
        LinkStatusTypeDef linkStatus;

        if (sta_ip_is_start())
        {
            os_memset(&linkStatus, 0x0, sizeof(LinkStatusTypeDef));
    	    bk_wlan_get_link_status(&linkStatus);
            *(int *)args = linkStatus.wifi_strength;
        }
        else
        {
            /* wifi disconnect */
            *(int *)args = 0;
        }
        break;
    }

    case WIFI_EASYJOIN:
    {
        start_connect_tick = rt_tick_get();
        result = _wifi_connect(dev, args);
        break;
    }

    case WIFI_SOFTAP:
    {
        _wifi_softap(dev, args);
        break;
    }

    case WIFI_DISCONNECT:
    {
        _wifi_disconnect(dev);
        break;
    }

    case WIFI_CFG_MONITOR:
    {
        _wifi_cfg_monitor(dev, args);
        break;
    }

    case WIFI_SET_CHANNEL:
    {
        int channel = *(int *)args;

        bk_wlan_set_channel_sync(channel);
        break;
    }

    case WIFI_ENTER_POWERSAVE:
    {
        int level;

        if (args == RT_NULL)
            return -RT_EIO;

        level =  *(int *)args;
        _wifi_power_manager(level);
        break;
    }

    case WIFI_SET_MONITOR_CALLBACK:
    {
        DRV_WLAN_DBG("set monitor callback \n");
        bk_wlan_register_monitor_cb(args);
        break;
    }

    default :
        break;
    }
    return result;
}

/* Ethernet device interface */
/* transmit packet. */
rt_err_t beken_wlan_tx(rt_device_t dev, struct pbuf *p)
{
    rt_err_t result = RT_EOK;

    return result;
}

/* reception packet. */
struct pbuf *beken_wlan_rx(rt_device_t dev)
{
    struct pbuf *p = RT_NULL;

    return p;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops wlan_ops =
{
    beken_wlan_init,
    beken_wlan_open,
    beken_wlan_close,
    beken_wlan_read,
    beken_wlan_write,
    beken_wlan_control
};
#endif /* RT_USING_DEVICE_OPS */

/**
 * register wlan device
 */
static rt_err_t rt_wlan_register(struct rt_wlan_device *device, const char *name, void *data)
{
    struct eth_device *eth;
    rt_err_t result = RT_EOK;

    RT_ASSERT(device != RT_NULL);
    eth = &(device->parent);

    /* initialize  ETH */
#ifdef RT_USING_DEVICE_OPS
    eth->parent.ops        = &wlan_ops;
#else
    eth->parent.init       = beken_wlan_init;
    eth->parent.open       = beken_wlan_open;
    eth->parent.close      = beken_wlan_close;
    eth->parent.read       = beken_wlan_read;
    eth->parent.write      = beken_wlan_write;
    eth->parent.control    = beken_wlan_control;
#endif /* RT_USING_DEVICE_OPS */

    eth->parent.user_data  = RT_NULL;

    eth->eth_rx            = beken_wlan_rx;
    eth->eth_tx            = beken_wlan_tx;

    device->user_data      = data;

    /* register ETH device */
    result = eth_device_init(eth, (char *)name);
    eth->netif->linkoutput = low_level_output;

    return result;
}

int beken_wlan_hw_init(void)
{
    struct rt_wlan_device *wlan = RT_NULL;
    struct eth_device *eth = RT_NULL;
    struct beken_wifi_info *wifi_info = RT_NULL;
    rt_err_t result = RT_EOK;

#ifdef RT_USING_WLAN_STA
    wlan = &_g_sta_device;
    wifi_info = &_g_sta_info;
    wifi_get_mac_address(wifi_info->mac, CONFIG_ROLE_STA);
    result = rt_wlan_register(wlan, WIFI_DEVICE_STA_NAME, wifi_info);
    if (result != RT_EOK)
    {
        rt_kprintf("register station wlan device failed! \n");
    }
    rt_kprintf("register station wlan device sucess! \n");
    rt_wlan_mgnt_attach(wlan, RT_NULL);
#endif

#ifdef RT_USING_WLAN_AP
    wlan = &_g_ap_device;
    wifi_info = &_g_ap_info;
    wifi_get_mac_address(wifi_info->mac, CONFIG_ROLE_AP);
    result = rt_wlan_register(wlan, WIFI_DEVICE_AP_NAME, wifi_info);
    if (result != RT_EOK)
    {
        rt_kprintf("register soft-ap wlan device failed! \n");
    }
    rt_kprintf("register soft-ap wlan device sucess! \n");
    rt_wlan_mgnt_attach(wlan, RT_NULL);
#endif

    rw_evt_set_callback(RW_EVT_STA_CONNECTED, wlan_event_handle);
    rw_evt_set_callback(RW_EVT_STA_DISCONNECTED, wlan_event_handle);
    rw_evt_set_callback(RW_EVT_STA_CONNECT_FAILED, wlan_event_handle);
    rw_evt_set_callback(RW_EVT_AP_CONNECTED, wlan_event_handle);
    rw_evt_set_callback(RW_EVT_AP_DISCONNECTED, wlan_event_handle);
    rw_evt_set_callback(RW_EVT_AP_CONNECT_FAILED, wlan_event_handle);

    rt_kprintf("beken wlan hw init\r\n");

    return 0;
}

INIT_DEVICE_EXPORT(beken_wlan_hw_init);

// eof
