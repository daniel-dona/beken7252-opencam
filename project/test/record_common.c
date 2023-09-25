#include <rtthread.h>
#include <rthw.h>
#include "record_common.h"

#include <sys/socket.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include "netdb.h"

rt_size_t rt_sound_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_size_t read_bytes = 0;

    while (read_bytes < size)
    {
        rt_size_t rb = rt_device_read(dev, pos, (void *)((char *)buffer + read_bytes), size - read_bytes);

        if (rb == 0)
            break;

        read_bytes += rb;
    }

    return read_bytes;
}

/* speex */
void int_to_char_little_endian(int i, unsigned char ch[4])
{
    ch[3] = i>>24;
    ch[2] = (i>>16)&0xFF;
    ch[1] = (i>>8)&0xFF;
    ch[0] = i&0xFF;
}

/* opus */
void int_to_char_big_endian(int i, unsigned char ch[4])
{
    ch[0] = i>>24;
    ch[1] = (i>>16)&0xFF;
    ch[2] = (i>>8)&0xFF;
    ch[3] = i&0xFF;
}

int tcp_client_init(struct net_worker *client)
{
    struct hostent *host;
    struct sockaddr_in server_addr;

    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = gethostbyname(client->url);

    /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
    if ((client->sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* 创建socket失败 */
        rt_kprintf("[tcp]:Socket error\n");;
        return -RT_ERROR;
    }

    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 连接到服务端 */
    if (connect(client->sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* 连接失败 */
        rt_kprintf("[tcp]:Connect fail!\n");
        closesocket(client->sock);
        return -RT_ERROR;
    }

    return RT_EOK;
}

