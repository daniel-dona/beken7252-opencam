#include "include.h"
#include <stdio.h>
#include <string.h>

#include <lwip/inet.h>

#include "netif/etharp.h"
#include "lwip/netif.h"
#include <lwip/netifapi.h>
#include <lwip/tcpip.h>
#include <lwip/dns.h>
#include <lwip/dhcp.h>
#include "lwip/prot/dhcp.h"

#include <lwip/sockets.h>
#include "ethernetif.h"


#include "sa_station.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#include "common.h"

#include "rw_pub.h"
#include "power_save_pub.h"

#include "lwip_netif_address.h"
#include "bk_rtos_pub.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "drv_wlan.h"
#include <sys/socket.h> 

#if CFG_ROLE_LAUNCH
#include "role_launch.h"
#endif

#define NET_DEBUG   0
#if NET_DEBUG
#define NET_DBG(...)     rt_kprintf("[NET]"),rt_kprintf(__VA_ARGS__)
#else
#define NET_DBG(...)
#endif


struct ipv4_config sta_ip_settings;
struct ipv4_config uap_ip_settings;
static int up_iface;
uint32_t sta_ip_start_flag = 0;
uint32_t uap_ip_start_flag = 0;

#ifdef CONFIG_IPV6
#define IPV6_ADDR_STATE_TENTATIVE       "Tentative"
#define IPV6_ADDR_STATE_PREFERRED       "Preferred"
#define IPV6_ADDR_STATE_INVALID         "Invalid"
#define IPV6_ADDR_STATE_VALID           "Valid"
#define IPV6_ADDR_STATE_DEPRECATED      "Deprecated"
#define IPV6_ADDR_TYPE_LINKLOCAL        "Link-Local"
#define IPV6_ADDR_TYPE_GLOBAL           "Global"
#define IPV6_ADDR_TYPE_UNIQUELOCAL      "Unique-Local"
#define IPV6_ADDR_TYPE_SITELOCAL        "Site-Local"
#define IPV6_ADDR_UNKNOWN               "Unknown"
#endif


#define net_e warning_prf
#define net_d warning_prf

typedef void (*net_sta_ipup_cb_fn)(void *data);

struct interface
{
    struct netif *netif;
    ip_addr_t ipaddr;
    ip_addr_t nmask;
    ip_addr_t gw;
};
FUNCPTR sta_connected_func;

static struct interface g_mlan;
static struct interface g_uap;
net_sta_ipup_cb_fn sta_ipup_cb = NULL;

void *net_get_sta_handle(void);
void *net_get_uap_handle(void);
err_t lwip_netif_init(struct netif *netif);
err_t lwip_netif_uap_init(struct netif *netif);
void handle_data_packet(const u8_t interface, const u8_t *rcvdata, const u16_t datalen);
int net_get_if_ip_addr(uint32_t *ip, void *intrfc_handle);
int net_get_if_ip_mask(uint32_t *nm, void *intrfc_handle);
int net_configure_address(struct ipv4_config *addr, void *intrfc_handle);
extern int dhcp_server_start(void *intrfc_handle);
extern void dhcp_server_stop(void);
extern void dhcpd_start(char *netif_name);
// extern const ip_addr_t* dns_getserver(u8_t numdns);
// extern void dns_setserver(u8_t numdns, ip_addr_t *dnsserver);

#ifdef CONFIG_IPV6
char *ipv6_addr_state_to_desc(unsigned char addr_state)
{
    if (ip6_addr_istentative(addr_state))
        return IPV6_ADDR_STATE_TENTATIVE;
    else if (ip6_addr_ispreferred(addr_state))
        return IPV6_ADDR_STATE_PREFERRED;
    else if (ip6_addr_isinvalid(addr_state))
        return IPV6_ADDR_STATE_INVALID;
    else if (ip6_addr_isvalid(addr_state))
        return IPV6_ADDR_STATE_VALID;
    else if (ip6_addr_isdeprecated(addr_state))
        return IPV6_ADDR_STATE_DEPRECATED;
    else
        return IPV6_ADDR_UNKNOWN;
}

char *ipv6_addr_type_to_desc(struct ipv6_config *ipv6_conf)
{
    if (ip6_addr_islinklocal((ip6_addr_t *)ipv6_conf->address))
        return IPV6_ADDR_TYPE_LINKLOCAL;
    else if (ip6_addr_isglobal((ip6_addr_t *)ipv6_conf->address))
        return IPV6_ADDR_TYPE_GLOBAL;
    else if (ip6_addr_isuniquelocal((ip6_addr_t *)ipv6_conf->address))
        return IPV6_ADDR_TYPE_UNIQUELOCAL;
    else if (ip6_addr_issitelocal((ip6_addr_t *)ipv6_conf->address))
        return IPV6_ADDR_TYPE_SITELOCAL;
    else
        return IPV6_ADDR_UNKNOWN;
}
#endif /* CONFIG_IPV6 */

int net_dhcp_hostname_set(char *hostname)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    netif_set_hostname(g_mlan.netif, hostname);
    return 0;
}

void net_ipv4stack_init()
{
    static bool tcpip_init_done = 0;
    if (tcpip_init_done)
        return;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    net_d("Initializing TCP/IP stack\r\n");
    tcpip_init(NULL, NULL);
    tcpip_init_done = true;
}

#ifdef CONFIG_IPV6
void net_ipv6stack_init(struct netif *netif)
{
    uint8_t mac[6];

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    netif->flags |= NETIF_IPV6_FLAG_UP;

    /* Set Multicast filter for IPV6 link local address
     * It contains first three bytes: 0x33 0x33 0xff (fixed)
     * and last three bytes as last three bytes of device mac */
    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = 0xff;
    mac[3] = netif->hwaddr[3];
    mac[4] = netif->hwaddr[4];
    mac[5] = netif->hwaddr[5];
    wifi_add_mcast_filter(mac);

    netif_create_ip6_linklocal_address(netif, 1);
    netif->ip6_autoconfig_enabled = 1;

    /* IPv6 routers use multicast IPv6 ff02::1 and MAC address
       33:33:00:00:00:01 for router advertisements */
    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = 0x00;
    mac[3] = 0x00;
    mac[4] = 0x00;
    mac[5] = 0x01;
    wifi_add_mcast_filter(mac);
}

static void wm_netif_ipv6_status_callback(struct netif *n)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    /*  TODO: Implement appropriate functionality here*/
    net_d("Received callback on IPv6 address state change");
    wlan_wlcmgr_send_msg(WIFI_EVENT_NET_IPV6_CONFIG,
                         WIFI_EVENT_REASON_SUCCESS, NULL);
}
#endif /* CONFIG_IPV6 */

void net_set_sta_ipup_callback(void *fn)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    sta_ipup_cb = (net_sta_ipup_cb_fn)fn;
}

void user_connected_callback(FUNCPTR fn)
{
    sta_connected_func = fn;
}

extern int start_connect_tick;
int ip_up_tick;
extern int _wifi_connect_done(void *ctx);
static void wm_netif_status_callback(struct netif *n)
{
    struct dhcp *dhcp;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);

    if (n->flags & NETIF_FLAG_UP)
    {
        dhcp = netif_dhcp_data(n);
        if (dhcp != NULL)
        {
            if (dhcp->state == DHCP_STATE_BOUND)
            {
                char str[IP4ADDR_STRLEN_MAX];

                memset(str, 0, IP4ADDR_STRLEN_MAX);
                rt_enter_critical();
                memcpy(str, ipaddr_ntoa(&n->ip_addr), IP4ADDR_STRLEN_MAX);
                rt_exit_critical();

                os_printf("IP UP: %s \r\n", ipaddr_ntoa(&n->ip_addr));

                netif_set_default(n); // rtthread TODO

#if CFG_ROLE_LAUNCH
                rl_pre_sta_set_status(RL_STATUS_STA_LAUNCHED);
#endif

                mhdr_set_station_status(MSG_GOT_IP);/* dhcp success*/
                ip_up_tick = rt_tick_get();
                rt_kprintf("[ip_up]:start tick =  %d, ip_up tick = %d, total = %d \n", start_connect_tick, ip_up_tick, ip_up_tick - start_connect_tick);
                _wifi_connect_done(NULL);
                if (sta_ipup_cb != NULL)
                    sta_ipup_cb(NULL);
                if (sta_connected_func != NULL)
                    (*sta_connected_func)();
            }
            else
            {
                // dhcp fail
            }
        }
        else
        {
            // static IP success;
        }
    }
    else
    {
        // dhcp fail;
    }
}

static int check_iface_mask(void *handle, uint32_t ipaddr)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    uint32_t interface_ip, interface_mask;
    net_get_if_ip_addr(&interface_ip, handle);
    net_get_if_ip_mask(&interface_mask, handle);
    if (interface_ip > 0)
        if ((interface_ip & interface_mask) ==
                (ipaddr & interface_mask))
            return 0;
    return -1;
}

void *net_ip_to_interface(uint32_t ipaddr)
{
    int ret;
    struct interface *handle;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    /* Check mlan handle */
    handle = (struct interface *)net_get_sta_handle();
    ret = check_iface_mask(handle->netif, ipaddr);
    if (ret == 0)
        return handle;

    /* Check uap handle */
    handle = net_get_uap_handle();
    ret = check_iface_mask(handle, ipaddr);
    if (ret == 0)
        return handle;

    /* If more interfaces are added then above check needs to done for
     * those newly added interfaces
     */
    return NULL;
}

void *net_sock_to_interface(int sock)
{
    struct sockaddr_in peer;
    unsigned long peerlen = sizeof(struct sockaddr_in);
    void *req_iface = NULL;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    lwip_getpeername(sock, (struct sockaddr *)&peer, &peerlen);
    req_iface = net_ip_to_interface(peer.sin_addr.s_addr);
    return req_iface;
}

void *net_get_sta_handle(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    return &g_mlan;
}

void *net_get_uap_handle(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    return &g_uap;
}

void *net_get_netif_handle(uint8_t iface)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    return NULL;
}

void net_interface_up(void *intrfc_handle)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    struct interface *if_handle = (struct interface *)intrfc_handle;
    netifapi_netif_set_up(if_handle->netif);
}

void net_interface_down(void *intrfc_handle)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    struct interface *if_handle = (struct interface *)intrfc_handle;
    netifapi_netif_set_down(if_handle->netif);
}

#ifdef CONFIG_IPV6
void net_interface_deregister_ipv6_callback(void *intrfc_handle)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    struct interface *if_handle = (struct interface *)intrfc_handle;
    if (intrfc_handle == &g_mlan)
        netif_set_ipv6_status_callback(if_handle->netif, NULL);
}
#endif

void net_interface_dhcp_stop(void *intrfc_handle)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    struct interface *if_handle = (struct interface *)intrfc_handle;
    netifapi_dhcp_stop(if_handle->netif);
    netif_set_status_callback(if_handle->netif, NULL);
}

void sta_ip_down(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    if (sta_ip_start_flag)
    {
        os_printf("sta_ip_down\r\n");

        sta_ip_start_flag = 0;

        netifapi_netif_set_down(g_mlan.netif);
        netif_set_status_callback(g_mlan.netif, NULL);
        netifapi_dhcp_stop(g_mlan.netif);
    }
}

void sta_ip_start(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    if (!sta_ip_start_flag)
    {
        os_printf("sta_ip_start\r\n");
        sta_ip_start_flag = 1;
        net_configure_address(&sta_ip_settings, net_get_sta_handle());
    }
}

void sta_set_vif_netif(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    rwm_mgmt_set_vif_netif(g_mlan.netif);
}

void ap_set_vif_netif(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    rwm_mgmt_set_vif_netif(g_uap.netif);
}

void sta_set_default_netif(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    struct interface *if_handle = (struct interface *)net_get_sta_handle();
    netifapi_netif_set_default(if_handle->netif);
}

void ap_set_default_netif(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    // as the default netif is sta's netif, so ap need to send
    // boardcast or not sub net packets, need set ap netif before
    // send those packets, after finish sending, reset default netif
    // to sat's netif.
    struct interface *if_handle = (struct interface *)net_get_uap_handle();
    netifapi_netif_set_default(if_handle->netif);
}

void reset_default_netif(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    netifapi_netif_set_default(NULL);
}

uint32_t sta_ip_is_start(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    return sta_ip_start_flag;
}

void uap_ip_down(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    if (uap_ip_start_flag)
    {
        os_printf("uap_ip_down\r\n");
        uap_ip_start_flag = 0;
        netifapi_netif_set_down(g_uap.netif);
        netif_set_status_callback(g_uap.netif, NULL);
        rt_kprintf("please use rtthread dncp stop server\n");
        // dhcp_server_stop();
    }
}

void uap_ip_start(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    if (!uap_ip_start_flag)
    {
        os_printf("uap_ip_start\r\n");
        uap_ip_start_flag = 1;
        net_configure_address(&uap_ip_settings, net_get_uap_handle());
    }
}

uint32_t uap_ip_is_start(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    return uap_ip_start_flag;
}

#define DEF_UAP_IP  0xc0a80a01UL /* 192.168.10.1 */

void ip_address_set(int iface, int dhcp, char *ip, char *mask, char *gw, char *dns)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    uint32_t tmp;
    struct ipv4_config addr;

    memset(&addr, 0, sizeof(struct ipv4_config));
    if (dhcp == 1)
    {
        addr.addr_type = ADDR_TYPE_DHCP;
    }
    else
    {
        addr.addr_type = ADDR_TYPE_STATIC;
        tmp = inet_addr((char *)ip);
        addr.address = (tmp);
        tmp = inet_addr((char *)mask);
        if (tmp == 0xFFFFFFFF)
            tmp = 0x00FFFFFF;// if not set valid netmask, set as 255.255.255.0
        addr.netmask = (tmp);
        tmp = inet_addr((char *)gw);
        addr.gw = (tmp);

        tmp = inet_addr((char *)dns);
        addr.dns1 = (tmp);
    }
    if (iface == 1) // Station
        memcpy(&sta_ip_settings, &addr, sizeof(addr));
    else
        memcpy(&uap_ip_settings, &addr, sizeof(addr));
}

int net_configure_address(struct ipv4_config *addr, void *intrfc_handle)
{
    if (!intrfc_handle)
        return -1;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    struct interface *if_handle = (struct interface *)intrfc_handle;

    net_d("\r\nconfiguring interface %s (with %s)",
          (if_handle == &g_mlan) ? "mlan" : "uap",
          (addr->addr_type == ADDR_TYPE_DHCP)
          ? "DHCP client" : "Static IP");
    os_printf("\n");
    netifapi_netif_set_down(if_handle->netif);

    /* De-register previously registered DHCP Callback for correct
     * address configuration.
     */
    netif_set_status_callback(if_handle->netif, NULL);
#ifdef CONFIG_IPV6
    if (if_handle == &g_mlan)
    {
        netif_set_ipv6_status_callback(&if_handle->netif,
                                       wm_netif_ipv6_status_callback);
        /* Explicitly call this function so that the linklocal address
         * gets updated even if the interface does not get any IPv6
         * address in its lifetime */
        wm_netif_ipv6_status_callback(&if_handle->netif);
    }
#endif
    switch (addr->addr_type)
    {
    case ADDR_TYPE_STATIC:
        if_handle->ipaddr.addr = addr->address;
        if_handle->nmask.addr = addr->netmask;
        if_handle->gw.addr = addr->gw;
        netifapi_netif_set_addr(if_handle->netif, &if_handle->ipaddr,
                                &if_handle->nmask, &if_handle->gw);
        netifapi_netif_set_up(if_handle->netif);
        break;

    case ADDR_TYPE_DHCP:
        NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
        /* Reset the address since we might be transitioning from static to DHCP */
        memset(&if_handle->ipaddr, 0, sizeof(ip_addr_t));
        memset(&if_handle->nmask, 0, sizeof(ip_addr_t));
        memset(&if_handle->gw, 0, sizeof(ip_addr_t));
        netifapi_netif_set_addr(if_handle->netif, &if_handle->ipaddr,
                                &if_handle->nmask, &if_handle->gw);

        netif_set_status_callback(if_handle->netif,
                                  wm_netif_status_callback);
        netifapi_netif_set_up(if_handle->netif);
        netifapi_dhcp_start(if_handle->netif);
        break;

    default:
        break;
    }
    /* Finally this should send the following event. */
    if (if_handle == &g_mlan)
    {
        // static IP up;

        /* XXX For DHCP, the above event will only indicate that the
         * DHCP address obtaining process has started. Once the DHCP
         * address has been obtained, another event,
         * WD_EVENT_NET_DHCP_CONFIG, should be sent to the wlcmgr.
         */
        up_iface = 1;

        // we always set sta netif as the default.
        sta_set_default_netif();
    }
    else
    {
        #if (CFG_USE_APP_DEMO_VIDEO_TRANSFER)
        // softap IP up, start dhcp server;
		dhcp_server_start(net_get_uap_handle());
        #else
        // softap IP up, start dhcp server;
        rt_kprintf("please use rtthread dncp start server\n");
        dhcpd_start(WIFI_DEVICE_AP_NAME);
        #endif

        up_iface = 0;

        // as the default netif is sta's netif, so ap need to send
        // boardcast or not sub net packets, need set ap netif before
        // send those packets, after finish sending, reset default netif
        // to sta's netif.
        os_printf("def netif is no ap's netif, sending boardcast or no-subnet ip packets may failed\r\n");
    }

    return 0;
}

int net_get_if_addr(struct wlan_ip_config *addr, void *intrfc_handle)
{
    const ip_addr_t *tmp;
    struct interface *if_handle = (struct interface *)intrfc_handle;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    addr->ipv4.address = if_handle->netif->ip_addr.addr;
    addr->ipv4.netmask = if_handle->netif->netmask.addr;
    addr->ipv4.gw = if_handle->netif->gw.addr;

    tmp = dns_getserver(0);
    addr->ipv4.dns1 = tmp->addr;
    tmp = dns_getserver(1);
    addr->ipv4.dns2 = tmp->addr;

    return 0;
}

int net_get_if_macaddr(void *macaddr, void *intrfc_handle)
{
    struct interface *if_handle = (struct interface *)intrfc_handle;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    os_memcpy(macaddr, &if_handle->netif->hwaddr[0], if_handle->netif->hwaddr_len);

    return 0;
}

#ifdef CONFIG_IPV6
int net_get_if_ipv6_addr(struct wlan_ip_config *addr, void *intrfc_handle)
{
    struct interface *if_handle = (struct interface *)intrfc_handle;
    int i;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
    {
        memcpy(addr->ipv6[i].address,
               if_handle->netif.ip6_addr[i].addr, 16);
        addr->ipv6[i].addr_state = if_handle->netif.ip6_addr_state[i];
    }
    /* TODO carry out more processing based on IPv6 fields in netif */
    return 0;
}

int net_get_if_ipv6_pref_addr(struct wlan_ip_config *addr, void *intrfc_handle)
{
    int i, ret = 0;
    struct interface *if_handle = (struct interface *)intrfc_handle;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
    {
        if (if_handle->netif.ip6_addr_state[i] == IP6_ADDR_PREFERRED)
        {
            memcpy(addr->ipv6[ret++].address,
                   if_handle->netif.ip6_addr[i].addr, 16);
        }
    }
    return ret;
}
#endif /* CONFIG_IPV6 */

int net_get_if_ip_addr(uint32_t *ip, void *intrfc_handle)
{
    struct interface *if_handle = (struct interface *)intrfc_handle;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    *ip = if_handle->netif->ip_addr.addr;
    return 0;
}

int net_get_if_ip_mask(uint32_t *nm, void *intrfc_handle)
{
    struct interface *if_handle = (struct interface *)intrfc_handle;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    *nm = if_handle->netif->netmask.addr;
    return 0;
}

int net_get_if_gw_addr(uint32_t *ip, void *intrfc_handle)
{
	struct interface *if_handle = (struct interface *)intrfc_handle;

	*ip = if_handle->netif->gw.addr;
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    
	return 0;
}

void net_configure_dns(struct wlan_ip_config *ip)
{
    ip_addr_t tmp;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    if (ip->ipv4.addr_type == ADDR_TYPE_STATIC)
    {

        if (ip->ipv4.dns1 == 0)
            ip->ipv4.dns1 = ip->ipv4.gw;
        if (ip->ipv4.dns2 == 0)
            ip->ipv4.dns2 = ip->ipv4.dns1;

        tmp.addr = ip->ipv4.dns1;
        dns_setserver(0, &tmp);
        tmp.addr = ip->ipv4.dns2;
        dns_setserver(1, &tmp);
    }

    /* DNS MAX Retries should be configured in lwip/dns.c to 3/4 */
    /* DNS Cache size of about 4 is sufficient */
}

extern struct rt_wlan_device _g_wlan_device;

void net_wlan_initial(void)
{
    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    net_ipv4stack_init();

#ifdef CONFIG_IPV6
    net_ipv6stack_init(&g_mlan.netif);
#endif /* CONFIG_IPV6 */
}

void net_wlan_add_netif(void *mac)
{
    VIF_INF_PTR vif_entry = NULL;
    struct interface *wlan_if = NULL;
    err_t err;
    u8 vif_idx;
    u8 *b = (u8 *)mac;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);

    if (!b || (!(b[0] | b[1] | b[2] | b[3] | b[4] | b[5])))
        return;

    vif_idx = rwm_mgmt_vif_mac2idx(mac);
    if (vif_idx == 0xff)
    {
        os_printf("net_wlan_add_netif not vif idx found\r\n");
        return ;
    }
    vif_entry = rwm_mgmt_vif_idx2ptr(vif_idx);
    if (!vif_entry)
    {
        os_printf("net_wlan_add_netif not vif found, %d\r\n", vif_idx);
        return ;
    }

    if (vif_entry->type == VIF_AP)
    {
        g_uap.netif = wlan_get_uap_netif();
        wlan_if = &g_uap;
    }
    else if (vif_entry->type == VIF_STA)
    {
        g_mlan.netif = wlan_get_sta_netif();
        wlan_if = &g_mlan;
    }
    else
    {
        os_printf("net_wlan_add_netif with other role\r\n");
        return ;
    }

    wlan_if->ipaddr.addr = INADDR_ANY;
    wlan_if->netif->state = (void *)vif_entry;
    vif_entry->priv = wlan_if->netif;

    /* set link_up for this netif */
    netif_set_link_up(wlan_if->netif);
}

void net_wlan_remove_netif(void *mac)
{
    err_t err;
    u8 vif_idx;
    VIF_INF_PTR vif_entry = NULL;
    struct netif *netif = NULL;
    u8 *b = (u8 *)mac;

    NET_DBG("L%d, %s \r\n", __LINE__, __FUNCTION__);
    if (!b || (!(b[0] | b[1] | b[2] | b[3] | b[4] | b[5])))
        return;

    vif_idx = rwm_mgmt_vif_mac2idx(mac);
    if (vif_idx == 0xff)
    {
        os_printf("net_wlan_add_netif not vif idx found\r\n");
        return ;
    }
    vif_entry = rwm_mgmt_vif_idx2ptr(vif_idx);
    if (!vif_entry)
    {
        os_printf("net_wlan_add_netif not vif found, %d\r\n", vif_idx);
        return ;
    }

    netif = (struct netif *)vif_entry->priv;
    if (!netif)
    {
        os_printf("net_wlan_remove_netif netif is null\r\n");
        return;
    }

    netif_set_link_down(netif);
    memset(&netif->ip_addr, 0, sizeof(ip_addr_t));
    memset(&netif->netmask, 0, sizeof(ip_addr_t));
    memset(&netif->gw, 0, sizeof(ip_addr_t));
    os_printf("netif_set_link_down !, vif_idx:%d\r\n", vif_idx);
}
