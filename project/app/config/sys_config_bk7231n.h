#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

/*SUMMARY: macro--1: OPEN; --0:CLOSE*/

/* uart2 for debug, and generally, uart1 is used for communication.
   what is more, uart1 maybe is not bound out*/
#define CFG_USE_UART1                              1
#define CFG_JTAG_ENABLE                            0
#define OSMALLOC_STATISTICAL                       0

/*section 0-----app macro config-----*/
#define CFG_IEEE80211N                             1

/*section 1-----OS macro config-----*/
#define THD_APPLICATION_PRIORITY                   3
#define THD_CORE_PRIORITY                          2
#define THD_UMP3_PRIORITY                          4
#define THD_UBG_PRIORITY                           5
#define THD_LWIP_PRIORITY                          4
#define THD_INIT_PRIORITY                          4
#define THD_RECONNECT_PRIORITY                     4
#define THD_MEDIA_PRIORITY                         4
#define THD_WPAS_PRIORITY                          5
#define THD_EXTENDED_APP_PRIORITY                  5
#define THD_HOSTAPD_PRIORITY                       5
#define THDD_KEY_SCAN_PRIORITY                     7

/*section 2-----function macro config-----*/
#define CFG_TX_EVM_TEST                            1
#define CFG_RX_SENSITIVITY_TEST                    1
#define CFG_AP_MONITOR_COEXIST                     0
#define CFG_ROLE_LAUNCH                            0
#define CFG_USE_WPA_29                             1
#define CFG_WPA_CTRL_IFACE                         1
#define CFG_RWNX_QOS_MSDU                          1
#define CFG_WLAN_FAST_CONNECT                      0
/* PMF */
#define CFG_IEEE80211W                             0
#if CFG_WPA_CTRL_IFACE
#undef CFG_ROLE_LAUNCH
#define CFG_ROLE_LAUNCH                            0
#endif
#define CFG_WPA3                                   0
#if CFG_WPA3
#undef CFG_USE_WPA_29
#define CFG_USE_WPA_29                             1
#undef CFG_IEEE80211W
#define CFG_IEEE80211W                             1
#define CFG_SME                                    0
#endif
//#define CFG_MESH                                 0
#define CFG_WFA_CERT                               0
#define CFG_ENABLE_BUTTON                          0
#define CFG_UDISK_MP3                              0
#define CFG_EASY_FLASH                             0
#define CFG_AP_SUPPORT_HT_IE                       0
#define CFG_SUPPORT_BSSID_CONNECT                  0
#define CFG_BK_AWARE                               0
#define CFG_BK_AWARE_OUI                           "\xC8\x47\x8C"

/*section 3-----driver macro config-----*/
#define CFG_MAC_PHY_BAPASS                         1

#define CFG_SDIO                                   0
#define CFG_SDIO_TRANS                             0
#define CFG_REAL_SDIO                              0
#if CFG_REAL_SDIO
#define FOR_SDIO_BLK_512                           0
#endif

#define CFG_MSDU_RESV_HEAD_LEN                    96
#define CFG_MSDU_RESV_TAIL_LEN                    16

#define CFG_USE_USB_HOST                           0

#define CFG_USB                                    0
#if CFG_USB
#define CFG_SUPPORT_MSD                            1
#define CFG_SUPPORT_HID                            0
#define CFG_SUPPORT_CCD                            0
#define CFG_SUPPORT_UVC                            0
#endif
#define CFG_USE_USB_CHARGE                         0

/*section 4-----DEBUG macro config-----*/
#define CFG_UART_DEBUG                             0
#define CFG_UART_DEBUG_COMMAND_LINE                1
#define CFG_BACKGROUND_PRINT                       0
#define CFG_SUPPORT_BKREG                          1
#define CFG_ENABLE_WPA_LOG                         0
#define CFG_IPERF_TEST                             0
#define CFG_TCP_SERVER_TEST                        0
#define CFG_AIRKISS_TEST                           0
#define CFG_ENABLE_DEMO_TEST                       0
#define CFG_WIFI_SENSOR                            0
#define CFG_WIFI_RAW_TX_CMD                        0

#define ASSERT_HALT                       1
#define ASSERT_IGNORE                   2
#define ASSERT_REBOOT                   3
#define CFG_ASSERT_OPTION           ASSERT_IGNORE

/*section 5-----PRODUCT macro config-----*/
#define CFG_RELEASE_FIRMWARE                       0

/*section 6-----for platform*/
#define SOC_PLATFORM                               1
#define FPGA_PLATFORM                              0
#define CFG_RUNNING_PLATFORM                       SOC_PLATFORM

#define SOC_BK7231                                 1
#define SOC_BK7231U                                2
#define SOC_BK7221U                                3
#define SOC_BK7271                                 4
#define SOC_BK7231N                                5
#define CFG_SOC_NAME                               SOC_BK7231N

/*section 7-----calibration*/
#if (CFG_RUNNING_PLATFORM == FPGA_PLATFORM)
#define CFG_SUPPORT_CALIBRATION                    0
#define CFG_SUPPORT_MANUAL_CALI                    0
#else
#define CFG_SUPPORT_CALIBRATION                    1
#define CFG_SUPPORT_MANUAL_CALI                    1
//tpc rf pa map power for bk7231u
#define CFG_SUPPORT_TPC_PA_MAP                     1
#endif

/*section 8-----for netstack*/
#define CFG_USE_LWIP_NETSTACK                      1

/*section 9-----for DHCP servicers and client*/
#define CFG_USE_DHCP                               1

/*section 11-----temperature detect*/
#define CFG_USE_TEMPERATURE_DETECT                 1

/*section 12-----for SPIDMA interface*/
#define CFG_USE_SPIDMA                             0
#define CFG_USE_CAMERA_INTF                        0

/*section 13-----for GENERRAL DMA */
#define CFG_GENERAL_DMA                            1

/*section 14-----for FTPD UPGRADE*/
#define CFG_USE_FTPD_UPGRADE                       0

/*section 15-----support customer macro*/
#define CFG_SUPPORT_TIANZHIHENG_DRONE              0

/*section 16-----support mcu & deep sleep*/
#define CFG_USE_MCU_PS                             1

#define CFG_USE_DEEP_PS                            1
#define CFG_USE_BLE_PS                             1
#define CFG_USE_AP_IDLE                            0
#define CFG_USE_FAKERTC_PS                         0

/*section 17-----support sta power sleep*/
#define CFG_USE_STA_PS                             1

/*section 18-----AP support stas in power save*/
#define CFG_USE_AP_PS                              0

/*section 19-----for SDCARD HOST*/
#define CFG_USE_SDCARD_HOST                        0

/*section 20 ----- support mp3 decoder*/
#define CONFIG_APP_MP3PLAYER                       0

/*section 21 ----- support ota*/
#if( ( CFG_SUPPORT_ALIOS ) || ( CFG_SUPPORT_RTT ) )
#define CFG_SUPPORT_OTA_HTTP                       0
#else
#define CFG_SUPPORT_OTA_HTTP                       1
#endif
#define CFG_SUPPORT_OTA_TFTP                       0

/*section 22 ----- support adc calibrate*/
#define CFG_SARADC_CALIBRATE                       0

/*section 23 ----- support reduce nomal power*/
#define CFG_SYS_REDUCE_NORMAL_POWER                0

/*section 24 ----- less memery in rwnx*/
#define CFG_LESS_MEMERY_IN_RWNX                    0

/*section 25 ----- use audio*/
#define CFG_USE_AUDIO                              0
#define CFG_USE_AUD_DAC                            0
#define CFG_USE_AUD_ADC                            0

/*section 25 ----- use tick time calibrate*/
#define CFG_USE_TICK_CAL                           1

#define CFG_SUPPORT_BLE                            1
#define CFG_SUPPORT_BLE_MESH                       0
#define CFG_USE_PTA                                0

#define BLE_VERSION_4_2                            1
#define BLE_VERSION_5_x                            2
#define CFG_BLE_VERSION                            BLE_VERSION_5_x

#define WIFI_DEFAULT_BLE_REQUEST                   1
#define BLE_DEFAULT_WIFI_REQUEST                   2
#define BLE_WIFI_CO_REQUEST                        3
#define RF_USE_POLICY                              WIFI_DEFAULT_BLE_REQUEST

#define CFG_BLE_ADV_NUM				1
#define CFG_BLE_SCAN_NUM			1

// 0 mean do not support ble master
#define CFG_BLE_INIT_NUM			0

#define CFG_BLE_CONN_NUM			1

#if (CFG_BLE_ADV_NUM == 0)
#error "ADV NUM should not be 0"
#endif

#if (CFG_BLE_CONN_NUM == 0)
#error "CONN NUM should not be 0"
#endif


#define CFG_SUPPOET_BSSID_CONNECT                  0

#define CFG_XTAL_FREQUENCE_40M                     40000000 //40MHz
#define CFG_XTAL_FREQUENCE_26M                     26000000 //26MHz
#if (CFG_SOC_NAME == SOC_BK7231N)
#define CFG_XTAL_FREQUENCE                         CFG_XTAL_FREQUENCE_26M//CFG_XTAL_FREQUENCE_40M
#else
#define CFG_XTAL_FREQUENCE                         CFG_XTAL_FREQUENCE_26M
#endif

#if (CFG_SUPPORT_ALIOS)
#undef  CFG_JTAG_ENABLE
#define CFG_JTAG_ENABLE                            0
#undef  CFG_ROLE_LAUNCH
#define CFG_ROLE_LAUNCH                            0
#undef  CFG_USE_MCU_PS
#define CFG_USE_MCU_PS                             RHINO_CONFIG_CPU_PWR_MGMT
#endif

#define LWIP_DEFAULT_MEM_POLICY                   1
#define LWIP_REDUCE_THE_PLAN                      2
#if CFG_IPERF_TEST
/* for iperf test, temporary enlarge lwip MEM_SIZE */
#define CFG_LWIP_MEM_POLICY                       LWIP_DEFAULT_MEM_POLICY
#else
#define CFG_LWIP_MEM_POLICY                       LWIP_REDUCE_THE_PLAN
#endif


/* watchdog, freertos only */
#define CFG_INT_WDG_ENABLED                        1
#define CFG_INT_WDG_PERIOD_MS                      10000
#define CFG_TASK_WDG_ENABLED                       1
#define CFG_TASK_WDG_PERIOD_MS                     60000

/*section 29 -----  peripheral interface open  */
#define CFG_USE_I2C1                                0
#define CFG_USE_I2C2                                1

#define CFG_USE_SPI_MASTER                         1
#define CFG_USE_SPI_SLAVE                          1
#define CFG_USE_SPI_DMA                            1

/*section 30 ----- peripheral interface test case */
#define CFG_PERIPHERAL_TEST							0
#define CFG_SUPPORT_SPI_TEST                        0


#define AT_SERVICE_CFG                             0

#endif // _SYS_CONFIG_H_
