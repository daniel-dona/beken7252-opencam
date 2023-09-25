#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK
#define RT_DEBUG_INIT 0
#define RT_DEBUG_THREAD 0
#define RT_USING_HOOK
#define IDLE_THREAD_STACK_SIZE 512
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 4096

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_MEMHEAP_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart1"

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 4096

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 8192
#define FINSH_CMD_SIZE 256
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 16

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 4
#define DFS_FILESYSTEM_TYPES_MAX 4
#define DFS_FD_MAX 16
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_3
#define RT_DFS_ELM_USE_LFN 3
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 512
#define RT_DFS_ELM_REENTRANT
#define RT_USING_DFS_DEVFS
#define RT_USING_DFS_NET
#define RT_USING_DFS_ROMFS

/* Device Drivers */
//#define BEKEN_USING_IIC
#define RT_USING_DEVICE_IPC
#define RT_USING_SERIAL
#define RT_USING_I2C
#define RT_USING_I2C2
//#define RT_USING_I2C_BITOPS
#define RT_USING_PIN
#define RT_USING_RTC
#define RT_USING_SOFT_RTC
#define RTC_SYNC_USING_NTP
#define RTC_NTP_FIRST_SYNC_DELAY 30
#define RTC_NTP_SYNC_PERIOD 3600
#define RT_USING_WDT
#define RT_USING_WIFI
#define RT_USING_WLAN_STA
#define RT_USING_WLAN_AP
#define WIFI_DEVICE_STA_NAME "w0"
#define WIFI_DEVICE_AP_NAME "ap"
#define RT_USING_AUDIO
#define RT_USING_SPI
#define RT_USING_I2S

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Network stack */

/* light weight TCP/IP stack */

#define RT_USING_LWIP
#define RT_USING_LWIP202
#define RT_LWIP_IGMP
#define RT_LWIP_ICMP
#define RT_LWIP_DNS
#define RT_LWIP_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

#define RT_LWIP_IPADDR "192.168.1.30"
#define RT_LWIP_GWADDR "192.168.1.1"
#define RT_LWIP_MSKADDR "255.255.255.0"
#define RT_LWIP_UDP
#define RT_LWIP_TCP
#define RT_LWIP_RAW
#define RT_MEMP_NUM_NETCONN 12
#define RT_LWIP_PBUF_NUM 3
#define RT_LWIP_RAW_PCB_NUM 2
#define RT_LWIP_UDP_PCB_NUM 6
#define RT_LWIP_TCP_PCB_NUM 8
#define RT_LWIP_TCP_SEG_NUM 40
#define RT_LWIP_TCP_SND_BUF 4380
#define RT_LWIP_TCP_WND 4380
#define RT_LWIP_TCPTHREAD_PRIORITY 4
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
#define RT_LWIP_TCPTHREAD_STACKSIZE 2048
#define LWIP_NO_RX_THREAD
#define LWIP_NO_TX_THREAD
#define RT_LWIP_ETHTHREAD_PRIORITY 12
#define RT_LWIP_ETHTHREAD_STACKSIZE 512
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
#define LWIP_NETIF_STATUS_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define LWIP_NETIF_LOOPBACK 0

/* Modbus master and slave stack */

#define LWIP_USING_DHCPD
#define DHCPD_SERVER_IP "192.168.169.1"
#define DHCPD_USING_ROUTER

/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* IoT - internet of things */

#define PKG_USING_PAHOMQTT
#define PAHOMQTT_PIPE_MODE
#define PKG_USING_PAHOMQTT_EXAMPLE
#define RT_PKG_MQTT_THREAD_STACK_SIZE 4096
#define PKG_PAHOMQTT_SUBSCRIBE_HANDLERS 1
#define MQTT_DEBUG
#define PKG_USING_PAHOMQTT_LATEST
#define PKG_USING_WEBCLIENT
#define WEBCLIENT_HEADER_BUFSZ 4096
#define WEBCLIENT_RESPONSE_BUFSZ 4096
#define PKG_USING_WEBCLIENT_LATEST_VERSION
#define PKG_USING_CJSON
#define PKG_USING_CJSON_V102

#define PKG_USING_WEBCLIENT
#define WEBCLIENT_HEADER_BUFSZ 4096
#define WEBCLIENT_RESPONSE_BUFSZ 4096
#define PKG_USING_WEBCLIENT_LATEST_VERSION
#define PKG_USING_WEBNET
#define WEBNET_PORT 80
#define WEBNET_CONN_MAX 16
#define WEBNET_ROOT "/"

/* Select supported modules */
#define WEBNET_USING_LOG
#define WEBNET_USING_AUTH
#define WEBNET_USING_CGI
#define WEBNET_USING_ASP
#define WEBNET_USING_SSI
#define WEBNET_USING_INDEX
#define WEBNET_USING_ALIAS
#define WEBNET_USING_UPLOAD
#define WEBNET_CACHE_LEVEL 0
#define WEBNET_USING_SAMPLES
#define PKG_USING_WEBNET_V201

/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */

#define PKG_USING_NETUTILS
#define PKG_NETUTILS_PING
#define PKG_NETUTILS_NETIO
#define PKG_NETUTILS_NTP
#define NETUTILS_NTP_TIMEZONE 8
#define NETUTILS_NTP_HOSTNAME "cn.ntp.org.cn"
#define PKG_USING_NETUTILS_LATEST_VERSION

/* IoT Cloud */


/* security packages */

/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */
#define CONFIG_SOUND_MIXER                   0

#define PKG_USING_FAL
#define FAL_DEBUG_CONFIG
#define FAL_DEBUG 1
#define PKG_USING_FAL_LATEST_VERSION
#define FAL_PART_TABLE_END_OFFSET (60 * 1024UL) //speed up if need change to 64k
#define FAL_PART_TABLE_FLASH_DEV_NAME "beken_onchip_crc"

/* peripheral libraries and drivers */


/* miscellaneous packages */

#define PKG_USING_MULTIBUTTON
#define PKG_USING_MULTIBUTTON_V102
#define PKG_USING_ZLIB

/* MultiButton Options */


/* sample package */


/* example package: hello */


/* Privated Packages of RealThread */


/* Network Utilities */


/* RT-Thread Senior Membership Packages */

/* system packages */


/* IoT - internet of things */

/* Webnet: A web server package for rt-thread */


/* Hardware Drivers Config */
#define BEKEN_USING_UART1
#define BEKEN_USING_WDT
#define BEKEN_USING_FLASH
#define BEKEN_USING_PWM
#define BEKEN_USING_GPIO
#define BEKEN_USING_WLAN
#define LWIP_NETIF_HOSTNAME 1
#define BEKEN_USING_WLAN_STA
#define RT_USING_CPU_FFS
//#define BEKEN_USING_SPI_FLASH
//#define BEKEN_USING_SPI_PSRAM
//#define BEKEN_USING_SPI_HSLAVE

/* Application Samples Config */

#define RT_USING_AIRKISS

/* Peripheral Samples Config */

#define SAMPLE_USING_GPIO_SAMPLE

/* RT-Thread OTA Samples */

#define SAMPLE_USING_OTA_HTTP

/* Test samples */

#define PKG_USING_RT_OTA

#define RT_USING_PTHREADS

/* voice changer */
#define CONFIG_VOICE_CHANGER        0

/* tools packages */
#define PKG_USING_EASYFLASH

/* optparse */
#define PKG_USING_OPTPARSE

/* PLAYER 1.2.7 ************************************************************************** */ 
#define PKG_USING_PLAYER
#define PKG_USING_PLAYER_V127
#define PLAYER_ENABLE_NET_STREAM

/* PLAYER Example */
#define PLAYER_USING_EXAMPLE00
#define PLAYER_USING_EXAMPLE02
/* PLAYER END **************************************************************************** */ 


#define PKG_USING_MULTIBUTTON
//#define MULTIBUTTON_USING_EXAMPLE_ASYNC

// #define PKG_USING_LITTLEFS
// #define RT_USING_MTD_NOR
#define PKG_USING_WEBNET
#define RT_USING_LWIP
//#define PKG_USING_LSSDP
//#define LSSDP_USING_SAMPLES
//#define RT_USING_LSSDP_TEST
#define RT_USING_LCD_TEST
#define PKG_USING_TJPGD
#define TJPGD_INPUT_BUFFER_SIZE 512
#define TJPGD_USING_FORMAT_RGB565
#define TJPGD_USING_SCALE
#define TJPGD_USING_TBLCLIP
#define PKG_TJPGD_SAMPLE
#define PKG_USING_TJPGD_LATEST_VERSION
#define QSPI_TEST
#define TEST_DEEP_SLEEP
//#define PKG_USING_QRCODE
#define PKG_USING_LITTLEFS
#define RT_USING_MTD_NOR
#endif
