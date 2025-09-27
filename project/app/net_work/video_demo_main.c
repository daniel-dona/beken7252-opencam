#include "include.h"
#include "video_demo_config.h"

#if (APP_DEMO_VIDEO_TRANSFER == 1)
#include "common.h"
#include "uart_pub.h"
#include "mem_pub.h"

#include "rtos_pub.h"

#define VIDE_MODE_STATION        (0x1)
#define VIDE_MODE_SOFTAP         (0x2)

extern void app_demo_sta_start(char *oob_ssid, char *connect_key);
extern void app_demo_sta_exit(void);

extern void app_demo_softap_start(char *oob_ssid, char *connect_key);
extern void app_demo_softap_exit(void);


void video_transfer_usage(void)
{
    os_printf("Usage: video_transfer [-s|-a ssid key]\n");
    os_printf("       video_transfer [-h|-stop]\n");
    os_printf("\n");
    os_printf("Sation mode:\n");
    os_printf("  -s           connect to specific AP with ssid & key\n");
    os_printf("\n");
    os_printf("Softap mode:\n");
    os_printf("  -a           startup softap with specific ssid & key\n");
    os_printf("\n");
    os_printf("Miscellaneous:\n");
    os_printf("  -h           print this message and quit\n");
    os_printf("  --stop       stop \n");

    return;
}

static int video_transfer(int argc, char **argv)
{
    char *oob_ssid = NULL;
    char *connect_key = NULL;
    static UINT32 video_transfer_mode = 0;

    if (argc == 1)
    {
        goto __usage;
    }
    else if (os_strcmp(argv[1], "-h") == 0)
    {
        goto __usage;
    }
    else if (os_strcmp(argv[1], "-stop") == 0)
    {
        os_printf("not implement!\n");
    }
    else if (os_strcmp(argv[1], "-a") == 0)
    {
        #if (APP_VIDEO_TRANSFER_MODE & VIDEO_TRANSFER_AP_MODE)
        if (argc < 3)
        {
            goto __usage;
        }

        if ((video_transfer_mode & VIDE_MODE_SOFTAP) == 0)
        {
            oob_ssid = argv[2];
            if (argc >= 4)
            {
                /* video_transfer -s ssid key [options] */
                connect_key = argv[3];
            }

            app_demo_softap_start(oob_ssid, connect_key);
            video_transfer_mode |= VIDE_MODE_SOFTAP;

        }
        else
        {
            os_printf("softap mode aready started\n");
        }
        #else
        os_printf("APP_VIDEO_TRANSFER_MODE not support softap mode \n");
        #endif
        return 1;
    }
    else if (os_strcmp(argv[1], "-s") == 0)
    {
        #if (APP_VIDEO_TRANSFER_MODE & VIDEO_TRANSFER_STA_MODE)
        if (argc < 3)
        {
            goto __usage;
        }

        if ((video_transfer_mode & VIDE_MODE_STATION) == 0)
        {
            oob_ssid = argv[2];
            if (argc >= 4)
            {
                /* video_transfer -s ssid key [options] */
                connect_key = argv[3];
            }

            app_demo_sta_start(oob_ssid, connect_key);
            video_transfer_mode |= VIDE_MODE_STATION;

        }
        else
        {
            os_printf("station mode aready started\n");
        }
        #else
        os_printf("APP_VIDEO_TRANSFER_MODE not support station mode \n");
        #endif
        return 1;
    }
    else
    {
        goto __usage;
    }

__usage:
    video_transfer_usage();
    return 0;
}

#if CFG_SUPPORT_RTT
FINSH_FUNCTION_EXPORT_ALIAS(video_transfer, __cmd_video_transfer, video transfer);
#else
void cmd_video_transfer(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    video_transfer(argc, argv);
}
#endif
#endif

#ifndef CFG_SUPPORT_RTT
#ifdef CFG_SUPPORT_ALIOS
#include <aos/aos.h>
#else
#include "wlan_cli_pub.h"
#endif
#if ((CFG_USE_CAMERA_INTF) && (APP_DEMO_CFG_USE_VIDEO_BUFFER))
extern void video_buffer_cmd(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
#endif

static const struct cli_command video_transfer_clis[] =
{
    #if (APP_DEMO_VIDEO_TRANSFER == 1)
    {"video_transfer", "video_transfer --help",    cmd_video_transfer},
    #endif
    #if ((CFG_USE_CAMERA_INTF) && (APP_DEMO_CFG_USE_VIDEO_BUFFER))
    {"video_buffer",   "open / close / read len",  video_buffer_cmd},
    #endif
};

int video_demo_register_cmd(void)
{
    #ifdef CFG_SUPPORT_ALIOS
    return aos_cli_register_commands(video_transfer_clis, sizeof(video_transfer_clis) / sizeof(struct cli_command));
    #else
    return cli_register_commands(video_transfer_clis, sizeof(video_transfer_clis) / sizeof(struct cli_command));
    #endif
}
#endif


