#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtdef.h>

#include <webnet.h>
#include <module.h>

#include <msh.h>
#include <easyflash.h>

#include "wlan_ui_pub.h"
#include<dfs_posix.h>
#include <param_config.h>

static void get_ip(char* buf)
{
    rt_ubase_t index;
    struct netif * netif;

    rt_enter_critical();
    netif = netif_list;

    while(netif != RT_NULL)
    {
        if(netif->flags & NETIF_FLAG_LINK_UP)
        {
            rt_kprintf("ip address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
            memcpy(buf, ipaddr_ntoa(&(netif->ip_addr)), 16);
        }
        netif = netif->next;
    }
    rt_exit_critical();
}

void cgi_ssdp_xml_handler(struct webnet_session *session)
{
    const char *mimetype = RT_NULL;
    struct webnet_request *request;
    int ret;
    struct stat buf;
    rt_uint8_t *read_buffer = RT_NULL;
    rt_uint8_t *write_buffer = RT_NULL;
    int fd, size;
    //uuid:4d696e69-444c-164e-9d41-001ec92f0378
    uint8_t mac[6] = {0};
    char name[128] = {0};
    char uuid[12] = {0};
    char *device_name = NULL;


    wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);

    device_name = ef_get_env("device_name");
    if ((device_name == NULL) || (rt_strcmp("", device_name) == 0))
    {
        rt_sprintf(name, "%s-%02x-%02x", "DIY", mac[4], mac[5]);
    }
    else
    {
        rt_sprintf(name, "%s", device_name);
    }

    rt_sprintf(uuid, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    /* get mimetype */
    mimetype = mime_get_type(".xml");
    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", -1);

    //get  /ssdp/descriptor.xml file size
    ret = stat("/ssdp/descriptor.xml", &buf);
    if (ret != 0)
    {
        //rt_kprintf("/ssdp/descriptor.xml file size = %d \n",buf.st_size);
        rt_kprintf("/ssdp/descriptor.xml file not found... \n");
        goto __end;
    }
    read_buffer = rt_malloc(buf.st_size + 1);
    if (read_buffer == RT_NULL)
    {
        rt_kprintf("no memory for read buffer!\n");
        goto __end;
    }

    write_buffer = rt_malloc(buf.st_size + sizeof(uuid) + sizeof(name)+ 1);
    if (write_buffer == RT_NULL)
    {
        rt_kprintf("no memory for write buffer!\n");
        goto __end;
    }
    //read out xml file
    fd = open("/ssdp/descriptor.xml", O_RDONLY);
    if (fd > 0)
    {
        size = read(fd, read_buffer, buf.st_size);
        close(fd);

        if (size < 0)
        {
            rt_kprintf("read from xml ERROR\r\n");
            goto __end;
        }
    }
    read_buffer[buf.st_size] = '\0';

    rt_sprintf(write_buffer, read_buffer, name, uuid);
    //printf("[%s]",write_buffer);

    webnet_session_printf(session, write_buffer);

__end:
    if (read_buffer != RT_NULL)
    {
        rt_free(read_buffer);
    }
    if (write_buffer != RT_NULL)
    {
        rt_free(write_buffer);
    }
}
