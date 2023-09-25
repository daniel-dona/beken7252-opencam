/**
* iperf-liked network performance tool
*
*/

#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include "netdb.h"

#define IPERF_PORT          5001
#define IPERF_BUFSZ         (4 * 1024)

#define IPERF_MODE_STOP     0
#define IPERF_MODE_SERVER   1
#define IPERF_MODE_CLIENT   2

typedef struct 
{
    int mode;

    char *host;
    int port;
}IPERF_PARAM;
static IPERF_PARAM param = {IPERF_MODE_STOP, NULL, IPERF_PORT};

static void iperf_client(void* thread_param)
{
    int i;
    int sock;
    int ret;

    uint8_t *send_buf;
    int sentlen;
    rt_tick_t tick1, tick2;
    struct sockaddr_in addr;

    send_buf = (uint8_t *) malloc (IPERF_BUFSZ);
    if (!send_buf) return ;

    for (i = 0; i < IPERF_BUFSZ; i ++)
        send_buf[i] = i & 0xff;

    while (param.mode != IPERF_MODE_STOP) 
    {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) 
        {
            printf("create socket failed!\n");
            rt_thread_delay(RT_TICK_PER_SECOND);
            continue;
        }

        addr.sin_family = PF_INET;
        addr.sin_port = htons(param.port);
        addr.sin_addr.s_addr = inet_addr((char*)param.host);

        ret = connect(sock, (const struct sockaddr*)&addr, sizeof(addr));
        if (ret == -1) 
        {
            printf("Connect failed!\n");
            closesocket(sock);

            rt_thread_delay(RT_TICK_PER_SECOND);
            continue;
        }

        printf("Connect to iperf server successful!\n");

        {
            int flag = 1;

            setsockopt(sock,
                IPPROTO_TCP,     /* set option at TCP level */
                TCP_NODELAY,     /* name of option */
                (void *) &flag,  /* the cast is historical cruft */
                sizeof(int));    /* length of option value */
        }

        sentlen = 0;

        tick1 = rt_tick_get();
        while(param.mode != IPERF_MODE_STOP) 
        {
            tick2 = rt_tick_get();
            if (tick2 - tick1 >= RT_TICK_PER_SECOND * 5)
            {
                float f;

                f = sentlen*RT_TICK_PER_SECOND/125/(tick2-tick1);
                f /= 1000;
                printf("%2.2f Mbps!\n", f);
                tick1 = tick2;
                sentlen = 0;
            }

            ret = send(sock, send_buf, IPERF_BUFSZ, 0);
            if (ret > 0) 
            {
                sentlen += ret;
            }

            if (ret < 0) break;
        }

        closesocket(sock);

        rt_thread_delay(RT_TICK_PER_SECOND*2);
        printf("disconnected!\n");
    }
}

void iperf_server(void* thread_param)
{
    uint8_t *recv_data;
    rt_uint32_t sin_size;
    rt_tick_t tick1, tick2;
    int sock = -1, connected, bytes_received, recvlen;
    struct sockaddr_in server_addr, client_addr;

    recv_data = (uint8_t *)malloc(IPERF_BUFSZ);
    if (recv_data == RT_NULL)
    {
        printf("No memory\n");
        goto __exit;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("Socket error\n");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(param.port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0x0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Unable to bind\n");
        goto __exit;
    }

    if (listen(sock, 5) == -1)
    {
        printf("Listen error\n");
        goto __exit;
    }

    while(param.mode != IPERF_MODE_STOP)
    {
        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

        printf("new client connected from (%s, %d)\n",
                  inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        {
            int flag = 1;

            setsockopt(connected,
                IPPROTO_TCP,     /* set option at TCP level */
                TCP_NODELAY,     /* name of option */
                (void *) &flag,  /* the cast is historical cruft */
                sizeof(int));    /* length of option value */
        }

        recvlen = 0;
        tick1 = rt_tick_get();
        while (param.mode != IPERF_MODE_STOP)
        {
            bytes_received = recv(connected, recv_data, IPERF_BUFSZ, 0);
            if (bytes_received <= 0) break;

            recvlen += bytes_received;

            tick2 = rt_tick_get();
            if (tick2 - tick1 >= RT_TICK_PER_SECOND * 5)
            {
                float f;

                f = recvlen*RT_TICK_PER_SECOND/125/(tick2-tick1);
                f /= 1000;
                printf("%2.2f Mbps!\n", f);
                tick1 = tick2;
                recvlen = 0;
            }
        }

        if (connected >= 0) closesocket(connected);
        connected = -1;
    }

__exit:
    if (sock >= 0) closesocket(sock);
    if (recv_data) free(recv_data);
}

void iperf_usage(void)
{
    printf("Usage: iperf [-s|-c host] [options]\n");
    printf("       iperf [-h|--stop]\n");
    printf("\n");
    printf("Client/Server:\n");
    printf("  -p #         server port to listen on/connect to\n");
    printf("\n");
    printf("Server specific:\n");
    printf("  -s           run in server mode\n");
    printf("\n");
    printf("Client specific:\n");
    printf("  -c <host>    run in client mode, connecting to <host>\n");
    printf("\n");
    printf("Miscellaneous:\n");
    printf("  -h           print this message and quit\n");
    printf("  --stop       stop iperf program\n");

    return ;
}

int iperf(int argc, char** argv)
{
    int mode = 0; /* server mode */
    char *host = NULL;
    int port = IPERF_PORT;

    if (argc == 1) goto __usage;
    else 
    {
        if (strcmp(argv[1], "-h") ==0) goto __usage;
        else if (strcmp(argv[1], "--stop") ==0)
        {
            /* stop iperf */
            param.mode = IPERF_MODE_STOP;
            return 0;
        }
        else if (strcmp(argv[1], "-s") ==0)
        {
            mode = IPERF_MODE_SERVER; /* server mode */

            /* iperf -s -p 5000 */
            if (argc == 4)
            {
                if (strcmp(argv[2], "-p") == 0)
                {
                    port = atoi(argv[3]);
                }
                else goto __usage;
            }
        }
        else if (strcmp(argv[1], "-c") ==0)
        {
            mode = IPERF_MODE_CLIENT; /* client mode */
            if (argc < 3) goto __usage;

            host = argv[2];
            if (argc == 5)
            {
                /* iperf -c host -p port */
                if (strcmp(argv[3], "-p") == 0)
                {
                    port = atoi(argv[4]);
                }
                else goto __usage;
            }
        }
        else if (strcmp(argv[1], "-h") ==0)
        {
            goto __usage;
        }
        else goto __usage;
    }

    /* start iperf */
    if (param.mode == IPERF_MODE_STOP)
    {
        rt_thread_t tid = RT_NULL;

        param.mode = mode;
        param.port = port;
        if (param.host)
        {
            rt_free(param.host);
            param.host = NULL;
        }
        if (host) param.host = rt_strdup(host);

        if (mode == IPERF_MODE_CLIENT)
            tid = rt_thread_create("iperfc", iperf_client, RT_NULL, 
                2048, 20, 20);
        else if (mode == IPERF_MODE_SERVER)
            tid = rt_thread_create("iperfd", iperf_server, RT_NULL, 
                2048, 20, 20);

        if (tid) rt_thread_startup(tid);
    }
    else
    {
        printf("Please stop iperf firstly, by:\n");
        printf("iperf --stop\n");
    }

    return 0;

__usage:
    iperf_usage();
    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(iperf, - the network bandwidth measurement tool);
#endif
