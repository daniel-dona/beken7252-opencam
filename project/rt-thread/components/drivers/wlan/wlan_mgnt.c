/*
 *  RT-Thread Wi-Fi Device
 *
 * COPYRIGHT (C) 2014 - 2018, Shanghai Real-Thread Technology Co., Ltd
 *
 *  This file is part of RT-Thread (http://www.rt-thread.org)
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-27     EvalZero     the first verion
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <lwip/netifapi.h>

#include "wlan_dev.h"
#include "wlan_cmd.h"

#define WLAN_MGNT_DEBUG   1

#if WLAN_MGNT_DEBUG
#define WLAN_MGNT_DBG(...)     rt_kprintf("[WLAN_MGNT]"),rt_kprintf(__VA_ARGS__)
#else
#define WLAN_MGNT_DBG(...)
#endif

#ifndef WIFI_DEVICE_STA_NAME
#define WIFI_DEVICE_STA_NAME    "w0"
#endif
#ifndef WIFI_DEVICE_AP_NAME
#define WIFI_DEVICE_AP_NAME    "ap"
#endif

static void wlan_mgnt_init_done_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan init done  event callback \n");
}

static void wlan_mgnt_link_up_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan link up event callback \n");
}

static void wlan_mgnt_link_down_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan link down event callback \n");
}

static void wlan_mgnt_sta_connected_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan sta connected event callback \n");
}

static void wlan_mgnt_sta_disconnected_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan sta disconnected event callback \n");
}

static void wlan_mgnt_sta_connect_failed_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan sta connect failed event callback \n");
}

static void wlan_mgnt_ap_start_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan ap start event callback \n");
}

static void wlan_mgnt_ap_associated_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan ap associated event callback \n");
}

static void wlan_mgnt_ap_disassociated_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan ap disassociated event callback \n");
}

static void wlan_mgnt_ap_associate_failed_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan ap disassociated event callback \n");
}

static void wlan_mgnt_scan_done_event(struct rt_wlan_device *device, rt_wlan_event_t event, void *user_data)
{
    WLAN_MGNT_DBG("wlan scan done event callback \n");
}

int rt_wlan_mgnt_attach(struct rt_wlan_device *device, void *user_data)
{
    RT_ASSERT(device != RT_NULL);

    rt_wlan_register_event_handler(device, WIFI_EVT_INIT_DONE, wlan_mgnt_init_done_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_LINK_DOWN, wlan_mgnt_link_up_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_LINK_UP, wlan_mgnt_link_down_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_STA_CONNECTED, wlan_mgnt_sta_connected_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_STA_DISCONNECTED, wlan_mgnt_sta_disconnected_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_STA_CONNECT_FAILED, wlan_mgnt_sta_connect_failed_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_AP_ASSOCIATED, wlan_mgnt_ap_associated_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_AP_DISASSOCIATED, wlan_mgnt_ap_disassociated_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_AP_ASSOCIATE_FAILED, wlan_mgnt_ap_associate_failed_event);
    rt_wlan_register_event_handler(device, WIFI_EVT_SCAN_DONE, wlan_mgnt_scan_done_event);

    return RT_EOK;
}
