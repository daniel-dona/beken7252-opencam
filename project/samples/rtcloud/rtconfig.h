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
#define RT_TIMER_THREAD_STACK_SIZE 2048

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
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
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 2
#define DFS_FILESYSTEM_TYPES_MAX 2
#define DFS_FD_MAX 16
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_0
#define RT_DFS_ELM_USE_LFN 0
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 512
#define RT_DFS_ELM_REENTRANT
#define RT_USING_DFS_DEVFS
#define RT_USING_DFS_NET
#define HAVE_SYS_SELECT_H

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_USING_SERIAL
#define RT_USING_I2C
#define RT_USING_I2C_BITOPS
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
#define RT_MEMP_NUM_NETCONN 8
#define RT_LWIP_PBUF_NUM 16
#define RT_LWIP_RAW_PCB_NUM 4
#define RT_LWIP_UDP_PCB_NUM 4
#define RT_LWIP_TCP_PCB_NUM 4
#define RT_LWIP_TCP_SEG_NUM 40
#define RT_LWIP_TCP_SND_BUF 8196
#define RT_LWIP_TCP_WND 8196
#define RT_LWIP_TCPTHREAD_PRIORITY 10
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
#define RT_LWIP_TCPTHREAD_STACKSIZE 4096
#define RT_LWIP_ETHTHREAD_PRIORITY 12
#define RT_LWIP_ETHTHREAD_STACKSIZE 1024
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
#define LWIP_NETIF_STATUS_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define LWIP_NETIF_LOOPBACK 0

/* Modbus master and slave stack */

#define LWIP_USING_DHCPD

/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* system packages */

/* RT-Thread GUI Engine */


/* IoT - internet of things */

#define PKG_USING_PAHOMQTT
#define PKG_USING_PAHOMQTT_EXAMPLE
#define PKG_PAHOMQTT_SUBSCRIBE_HANDLERS 1
#define PKG_USING_PAHOMQTT_LATEST_VERSION
#define PKG_USING_WEBCLIENT
#define WEBCLIENT_USING_TLS
#define PKG_USING_WEBCLIENT_LATEST_VERSION
#define PKG_USING_CJSON
#define PKG_USING_CJSON_V102

/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */

#define PKG_USING_NETUTILS
#define PKG_NETUTILS_PING
#define PKG_NETUTILS_NETIO
#define PKG_NETUTILS_NTP
#define NETUTILS_NTP_TIMEZONE 8
#define NETUTILS_NTP_HOSTNAME "cn.pool.ntp.org"
#define PKG_USING_NETUTILS_LATEST_VERSION

/* security packages */

#define PKG_USING_MBEDTLS
#define PKG_USING_MBEDTLS_EXAMPLE
#define PKG_USING_MBEDTLS_LATEST_VERSION
#define PKG_USING_TINYCRYPT
#define PKG_USING_TINYCRYPT_V100
#define TINY_CRYPT_MD5
#define TINY_CRYPT_AES
#define TINY_CRYPT_AES_ROM_TABLES

/* language packages */


/* multimedia packages */


/* tools packages */

#define PKG_USING_EASYFLASH
#define PKG_USING_EASYFLASH_LATEST_VERSION

/* miscellaneous packages */

#define PKG_USING_QUICKLZ
#define QLZ_COMPRESSION_LEVEL 3
#define PKG_USING_QUICKLZ_LATEST_VERSION

/* example package: hello */


/* Privated Packages of RealThread */


/* Network Utilities */

#define PKG_USING_CLOUDSDK
#define CLD_PRODUCT_ID "c0012be6b2b147da"
#define CLD_PRODUCT_KEY "63fb42d3-22e5-4161-840e-093ba0fbf245"
#define CLD_USING_SHELL
#define CLD_USING_OTA
#define PKG_USING_CLOUDSDK_LATEST_VERSION

/* Webnet: A web server package for rt-thread */

#define PKG_USING_RT_OTA
#define RT_OTA_USING_CRYPT
#define RT_OTA_CRYPT_ALGO_USING_AES256
#define RT_OTA_USING_CMPRS
#define RT_OTA_CMPRS_ALGO_USING_QUICKLZ
#define PKG_USING_RT_OTA_LATEST_VERSION

/* Hardware drivers config */

#define BEKEN_USING_UART1
#define BEKEN_USING_WDT
#define BEKEN_USING_FLASH
#define BEKEN_USING_PWM
#define BEKEN_USING_GPIO
#define BEKEN_USING_WLAN
#define LWIP_NETIF_HOSTNAME 1
#define RT_USING_STATION
#define RT_USING_SOTF_AP
#define RT_USING_CPU_FFS

/* Application Samples config */


/* RT-Thread OTA config */

#define SAMPLE_USING_CLOUDSDK

/* Test samples */

#define RT_USING_IPERF_TEST

#endif
