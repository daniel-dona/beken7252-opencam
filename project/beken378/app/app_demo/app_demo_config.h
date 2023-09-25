#ifndef __APP_DEMO_CONFIG_H__
#define __APP_DEMO_CONFIG_H__

#include "include.h"

// for softap configuration
#if 0

#else
#define APP_DEMO_SOFTAP_DEF_SSID          "BEKEN_WIFI_000000"
#define APP_DEMO_SOFTAP_DEF_NET_IP        "192.168.1.1"
#define APP_DEMO_SOFTAP_DEF_NET_MASK      "255.255.255.0"
#define APP_DEMO_SOFTAP_DEF_NET_GW        "192.168.1.1"
#define APP_DEMO_SOFTAP_DEF_CHANNEL       1   

#define APP_DEMO_UDP_CMD_PORT             7090 
#define APP_DEMO_UDP_IMG_PORT             7080 
#define CMD_IMG_HEADER                    0x20
#define CMD_START_IMG                     0x36
#define CMD_STOP_IMG                      0x37
#define CMD_START_OTA                     0x38
  
#define APP_DEMO_TCP_SERVER_PORT          7050
#define APP_DEMO_TCP_SERVER_PORT_VICOE    7040 

#define APP_DEMO_UDP_VOICE_PORT           7070 
#define CMD_VOICE_HEADER                  0x30
#define CMD_VOICE_START                   0x66
#define CMD_VOICE_STOP                    0x67
#endif 

#define APP_DEMO_CFG_USE_TCP              1
#define APP_DEMO_CFG_USE_UDP              1

#define CMD_HEADER_CODE                   0x66
#define CMD_TAIL_CODE                     0x99
#define CMD_LEN                           8
#define APP_DEMO_TCP_LISTEN_MAX           1

#if (CFG_USE_AUDIO && CFG_USE_AUD_ADC)
#define APP_DEMO_EN_VOICE_TRANSFER        1
#else
#define APP_DEMO_EN_VOICE_TRANSFER        0
#endif // (CFG_SOC_NAME == SOC_BK7221U)

#endif // __APP_DEMO_CONFIG_H__
