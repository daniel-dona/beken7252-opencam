#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <webnet.h>
#include <module.h>

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/prot/dns.h"
#include "lwip/udp.h"

#ifndef PKG_USING_CJSON
    #error need cJSON
#endif

#include "cJSON.h"

/*
output format: json
{
    "e0":
    {
        "dhcp":"disable",
        "ip":"192.168.0.100",
        "netmask":"255.255.255.0",
        "gateway":"192.168.0.1",
    },

    "e1":
    {
        "dhcp":"disable",
        "ip":"192.168.1.101",
        "netmask":"255.255.255.0",
        "gateway":"192.168.1.1",
    },

    "dns0":"8.8.8.8",
    "dns1":"4.4.4.4",

    "default":"e0",
}
*/
static void get_ip_cfg_handler(struct webnet_session *session)
{
    struct webnet_request *request;
    struct netif *netif = netif_list;

    cJSON *json = RT_NULL;
    char *body = RT_NULL;

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    json = cJSON_CreateObject();
    if (!json)
        goto _exit;

    while (netif != RT_NULL)
    {
        cJSON *json_netif = 0;
        char str_buf[16]; /* 3*4+3+1 */

        json_netif = cJSON_CreateObject();
        if (!json_netif)
        {
            break;
        }

        rt_enter_critical();
        cJSON_AddStringToObject(json_netif, "ip", inet_ntoa(*((struct in_addr *) & (netif->ip_addr))));
        cJSON_AddStringToObject(json_netif, "netmask", inet_ntoa(*((struct in_addr *) & (netif->netmask))));
        cJSON_AddStringToObject(json_netif, "gateway", inet_ntoa(*((struct in_addr *) & (netif->gw))));
        rt_exit_critical();

        memcpy(str_buf, &netif->name[0], sizeof(netif->name));
        str_buf[sizeof(netif->name)] = '\0';
        cJSON_AddItemToObject(json, str_buf, json_netif);

        if (netif == netif_default)
        {
            cJSON_AddStringToObject(json, "default", str_buf);
        }

        netif = netif->next;
    } /**< while(netif != RT_NULL) */

#ifdef RT_LWIP_DNS
    {
        rt_ubase_t index;
        const ip_addr_t *ip_addr;
        extern const ip_addr_t *dns_getserver(u8_t numdns);

        char str_buf[16]; /* 3*4+3+1 */

        for (index = 0; index < DNS_MAX_SERVERS; index++)
        {
            sprintf(str_buf, "dns%d", index);

            ip_addr = dns_getserver(index);

            rt_enter_critical();
            cJSON_AddStringToObject(json, str_buf, ipaddr_ntoa(ip_addr));
            rt_exit_critical();
        }
    }
#endif /**< #ifdef RT_LWIP_DNS */

_exit:
    if (json)
    {
        body = cJSON_PrintUnformatted(json);
    }

    if (body)
    {
        /* set http header */
        session->request->result_code = 200;
        webnet_session_set_header(session, "application/json", 200, "Ok", -1);
        webnet_session_printf(session, body);
    }
    else
    {
        /* set http header */
        session->request->result_code = 500;
    }

    if (json)
        cJSON_Delete(json);

    if (body)
        rt_free(body);

    return;
}

#define ADDRESS_STR_LEN    (3*4+3+1) /* 255.255.255.255 */
static void set_ip_cfg_handler(struct webnet_session *session)
{
    const char *mimetype;
    struct webnet_request *request;
    struct netif *netif = netif_list;

    static const char *header = "<html><head>"
                                "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />"
                                "<meta http-equiv=\"refresh\" content=\"3;url=%s\"/>"
                                "<title> ip </title></head>";
    static const char *body = "<body>%s</body></html>\r\n";
    char error_message[32];

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    /* get mimetype */
    mimetype = mime_get_type(".html");
    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", -1);

    if (request->query_counter == 0)
    {
        rt_sprintf(error_message, "no parameter");
        goto invalid;
    }

    while (netif != RT_NULL)
    {
        char ip_buffer[ADDRESS_STR_LEN];
        char netmask_buffer[ADDRESS_STR_LEN];
        char gateway_buffer[ADDRESS_STR_LEN];

        char netif_name[sizeof(netif->name) + 1];
        char addr_name_str[16]; /* e0_gateway_0 */
        const char *addr[4];

        rt_uint32_t index;

        strncpy(netif_name, netif->name, sizeof(netif->name));
        netif_name[sizeof(netif_name) - 1] = '\0';

        /* get ip */
        for (index = 0; index < 4; index++)
        {
            sprintf(addr_name_str, "%s_ip_%d", netif_name, index);
            addr[index] = webnet_request_get_query(request, addr_name_str);
            if (addr[index] == RT_NULL)
            {
                sprintf(error_message, "%s error!", addr_name_str);
                goto invalid;
            }
        }
        sprintf(ip_buffer, "%s.%s.%s.%s", addr[0], addr[1], addr[2], addr[3]);

        /* get netmask */
        for (index = 0; index < 4; index++)
        {
            sprintf(addr_name_str, "%s_netmask_%d", netif_name, index);
            addr[index] = webnet_request_get_query(request, addr_name_str);
            if (addr[index] == RT_NULL)
            {
                sprintf(error_message, "%s error!", addr_name_str);
                goto invalid;
            }
        }
        sprintf(netmask_buffer, "%s.%s.%s.%s", addr[0], addr[1], addr[2], addr[3]);

        /* get netmask */
        for (index = 0; index < 4; index++)
        {
            sprintf(addr_name_str, "%s_gateway_%d", netif_name, index);
            addr[index] = webnet_request_get_query(request, addr_name_str);
            if (addr[index] == RT_NULL)
            {
                sprintf(error_message, "%s error!", addr_name_str);
                goto invalid;
            }
        }
        sprintf(gateway_buffer, "%s.%s.%s.%s", addr[0], addr[1], addr[2], addr[3]);

        {
            extern void set_if(char *netif_name, char *ip_addr, char *gw_addr, char *nm_addr);
            set_if(netif_name, ip_buffer, gateway_buffer, netmask_buffer);
        }

        netif = netif->next;
    }

    webnet_session_printf(session, header, (request->referer != RT_NULL) ? request->referer : "/");
    webnet_session_printf(session, body, "TCP/IP setting success!");

    return;

invalid:
    webnet_session_write(session, (const rt_uint8_t *)header, rt_strlen(header));
    webnet_session_printf(session, body, error_message);
    return;
}

void cgi_ipcfg_init(void)
{
    webnet_cgi_register("get_ip_cfg", get_ip_cfg_handler);
    webnet_cgi_register("set_ip_cfg", set_ip_cfg_handler);
}
