#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <rthw.h>
#include <rtthread.h>

int airkiss_lan_init(void);


#ifdef RT_USING_DFS_NET
    #include <sys/socket.h> // only dfs_net
    #include <sys/select.h> // only dfs_net
    #include "netdb.h"
    #include <dfs_posix.h>
#else
    #define read        lwip_read
    #define write       lwip_write
#endif /* RT_USING_DFS_NET */

#include "airkiss.h"
#include "weixin_config_custom.h"

#define DEBUG_PRINTF     rt_kprintf("[airkiss_lan] ");rt_kprintf

#define malloc  rt_malloc
#define realloc rt_realloc
#define free    rt_free

/**************************************************/

#define DEFAULT_LAN_PORT            (12476)
#define AIRKISS_LAN_BUF_LEN         (1024)

#if 0
extern const airkiss_config_t ak_conf;
#else
static const airkiss_config_t ak_conf =
{
    (airkiss_memset_fn) &rt_memset,
    (airkiss_memcpy_fn) &rt_memcpy,
    (airkiss_memcmp_fn) &rt_memcmp,
    (airkiss_printf_fn) &rt_kprintf,
};
#endif

/* airkiss lan. */
static void airkiss_lan_thread(void *parameter)
{
    int sock = -1;
    struct sockaddr_in bind_addr, client_addr;
    rt_uint32_t addr_len;
    char *lan_buf = 0;
    int bytes_read;

    uint16_t lan_buf_len;
    airkiss_lan_ret_t ret;
    airkiss_lan_ret_t packret;

    char DEVICE_ID[33]; /* 32+1 */

    {
        extern char flash_id_str[];

        sprintf(DEVICE_ID, "EF4017D1632048DF1F1B31");
    }

#if 0
    //extern const void *netif_default;
    while (netif_default == RT_NULL)
    {
        rt_thread_delay(rt_tick_from_millisecond(1000));
        //rt_kprintf("netif_default == RT_NULL, wait wifi startup!\n");
    }
#endif

    lan_buf = (char *)malloc(AIRKISS_LAN_BUF_LEN + 4);
    if (lan_buf == 0)
    {
        DEBUG_PRINTF("no memory for lan_buf!\n");
        goto _exit;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        DEBUG_PRINTF("notify create socket error!\n");
        goto _exit;
    }

    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(DEFAULT_LAN_PORT);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(bind_addr.sin_zero), 0, sizeof(bind_addr.sin_zero));

    /* SO_BROADCAST */
    {
        int broadcast = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0)
        {
            DEBUG_PRINTF("socket setsockopt SO_BROADCAST error\n");
            goto _exit;
        }
    }

    if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) != 0)
    {
        DEBUG_PRINTF("notify socket bind error\n");
        goto _exit;
    }

    while (1)
    {
        bytes_read = recvfrom(sock, lan_buf, AIRKISS_LAN_BUF_LEN, 0,
                              (struct sockaddr *)&client_addr, &addr_len);

        DEBUG_PRINTF("recv %d byte from(%s , %d)\n", bytes_read, inet_ntoa(client_addr.sin_addr),
                     ntohs(client_addr.sin_port));

        ret = airkiss_lan_recv(lan_buf, bytes_read, &ak_conf);

        switch (ret)
        {
        case AIRKISS_LAN_SSDP_REQ:
        {
            DEBUG_PRINTF("AIRKISS_LAN_SSDP_REQ!\n");

            //airkiss_udp.remote_port = DEFAULT_LAN_PORT;
            lan_buf_len = AIRKISS_LAN_BUF_LEN;

            packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD, (void *)WEIXIN_DEVICE_TYPE, (void *)DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &ak_conf);
            if (packret != AIRKISS_LAN_PAKE_READY)
            {
                DEBUG_PRINTF("Pack lan packet error!\n");
                return;
            }

            packret = sendto(sock, lan_buf, lan_buf_len, 0,
                             (struct sockaddr *)&client_addr, sizeof(struct sockaddr));

            DEBUG_PRINTF("pack %d byte to(%s , %d)\n", lan_buf_len, inet_ntoa(client_addr.sin_addr),
                         ntohs(client_addr.sin_port));

            //packret = espconn_sent(&ptrairudpconn, lan_buf, lan_buf_len);
            if (packret != lan_buf_len)
            {
                DEBUG_PRINTF("LAN UDP Send err! %d:%d", packret, lan_buf_len);
            }
        }
        break;

        default:
        {
            if (ret != 0)
            {
                DEBUG_PRINTF("Pack %d is not ssdq req!", ret);
            }
        }
        break;
        }


    }

    DEBUG_PRINTF("airkiss notification thread exit!\n");

_exit:
    if (sock >= 0)
    {
        lwip_close(sock);
    }

    if (lan_buf)
        free(lan_buf);
}

int airkiss_lan_init(void)
{
    rt_thread_t tid;
    rt_uint32_t stack_size;

    stack_size = 1024 * 3;
    tid = rt_thread_create("air_lan",
                           airkiss_lan_thread, 0, // fun, parameter
                           stack_size, // stack size
                           18, //priority
                           1);
    if (tid)
    {
        DEBUG_PRINTF("startup!\n");
        rt_thread_startup(tid);
    }

    return 0;
}
INIT_COMPONENT_EXPORT(airkiss_lan_init);

