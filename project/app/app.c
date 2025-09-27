/**
 ****************************************************************************************
 *
 * @file app.c
 *
 *
 * Copyright (C) Beken Corp 2011-2016
 *
 ****************************************************************************************
 */
#include "include.h"
#include "mem_pub.h"
#include "rwnx_config.h"
#include "app.h"

#if (NX_POWERSAVE)
#include "ps.h"
#endif //(NX_POWERSAVE)

#include "sa_ap.h"
#include "sa_station.h"
#include "main_none.h"
#include "sm.h"
#include "uart_pub.h"
#include "rtos_pub.h"
#include "error.h"
#include "param_config.h"
#include "rxl_cntrl.h"
#include "lwip/pbuf.h"
#include "rw_pub.h"
#include "rw_msg_rx.h"
#include "hostapd_intf_pub.h"
#include "wlan_ui_pub.h"
#include "ps_debug_pub.h"
#include "power_save_pub.h"
#include "mcu_ps_pub.h"
#include "rw_msdu.h"
#include "txu_cntrl.h"
#include "power_save.h"

// Dani
#include "i2c_pub.h"
#include "gpio_pub.h"
#include "jpeg_encoder_pub.h"

#define CAMERA_RESET_GPIO_INDEX		GPIO16
#define CAMERA_RESET_HIGH_VAL       1
#define CAMERA_RESET_LOW_VAL        0
extern void delay100us(INT32 num);

#include "msh.h"

//#include "camera_intf.h"


#include "ap_idle_pub.h"
#include "arbitrate.h"
#include "ke_event.h"

beken_thread_t  init_thread_handle;
beken_thread_t  app_thread_handle;
beken_thread_t  user_thread_handle;


uint32_t  init_stack_size = 2000;
uint32_t  app_stack_size = 4096;

beken_semaphore_t app_sema = NULL;
WIFI_CORE_T g_wifi_core = {0};
volatile int32_t bmsg_rx_count = 0;

extern void net_wlan_initial(void);
extern void wpas_thread_start(void);

void bk_wlan_app_init(void)
{
#if (!CFG_SUPPORT_RTT)
    net_wlan_initial();
#endif

    wpas_thread_start();
}

void app_set_sema(void)
{
    OSStatus ret;
    ret = rtos_set_semaphore(&app_sema);

    (void)ret;
}

static void kmsg_bk_thread_main( void *arg )
{
    OSStatus ret;

    mr_kmsg_init();
    while(1)
    {
        ret = rtos_get_semaphore(&app_sema, BEKEN_WAIT_FOREVER);
        ASSERT(kNoErr == ret);

        rwnx_recv_msg();
        ke_evt_none_core_scheduler();
    }
}

static void init_thread_main( void *arg ){
    GLOBAL_INT_START();

    bk_wlan_app_init();
    os_printf("bk_wlan_app_init finished\r\n");

    rtos_delete_thread( NULL );
}

/** @brief  When in dtim rf off mode,user can manual wakeup before dtim wakeup time.
 *          this function must be called in "core_thread" context
 */
int bmsg_ps_handler_rf_ps_mode_real_wakeup(void)
{
#if CFG_USE_STA_PS
    power_save_rf_dtim_manual_do_wakeup();
#endif
    return 0;
}

void bmsg_rx_handler(BUS_MSG_T *msg)
{
    GLOBAL_INT_DECLARATION();

    GLOBAL_INT_DISABLE();
    if(bmsg_rx_count > 0)
    {
        bmsg_rx_count -= 1;
    }
    GLOBAL_INT_RESTORE();

    rxl_cntrl_evt((int)msg->arg);
}

void bmsg_skt_tx_handler(BUS_MSG_T *msg)
{
    hapd_intf_ke_rx_handle(msg->arg);
}

void bmsg_tx_handler(BUS_MSG_T *msg)
{
    struct pbuf *p = (struct pbuf *)msg->arg;
    struct pbuf *q = p;
    uint8_t vif_idx = (uint8_t)msg->len;

    if(p->next)
    {
        q = pbuf_coalesce(p, PBUF_RAW);
        if(q == p)
        {
            // must be out of memory
            goto tx_handler_exit;
        }
    }

    ps_set_data_prevent();

#if CFG_USE_STA_PS
    bmsg_ps_handler_rf_ps_mode_real_wakeup();
    bk_wlan_dtim_rf_ps_mode_do_wakeup();
#endif

    rwm_transfer(vif_idx, q->payload, q->len, 0, 0);

tx_handler_exit:
    pbuf_free(q);
}


void bmsg_tx_raw_cb_handler(BUS_MSG_T *msg)
{
	rwm_raw_frame_with_cb((uint8_t *)msg->arg, msg->len, msg->cb, msg->param);
}

int bmsg_tx_raw_cb_sender(uint8_t *buffer, int length, void *cb, void *param)
{
	OSStatus ret;
	BUS_MSG_T msg;

	msg.type = BMSG_TX_RAW_CB_TYPE;
	msg.arg = (uint32_t)buffer;
	msg.len = length;
	msg.sema = NULL;
	msg.cb = cb;
	msg.param = param;

	ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, 1*SECONDS);
	if(ret != kNoErr)
	{
		APP_PRT("bmsg_tx_sender failed\r\n");
	}

	return ret;
}

void bmsg_tx_raw_handler(BUS_MSG_T *msg)
{
	uint8_t *pkt = (uint8_t *)msg->arg;
	uint16_t len = msg->len;
	MSDU_NODE_T *node;
	UINT8 *content_ptr;
	UINT32 queue_idx = AC_VI;
	struct txdesc *txdesc_new;
	struct umacdesc *umac;

	node = rwm_tx_node_alloc(len);
	if (node == NULL) {
		goto exit;
	}

	rwm_tx_msdu_renew(pkt, len, node->msdu_ptr);
	content_ptr = rwm_get_msdu_content_ptr(node);

	txdesc_new = tx_txdesc_prepare(queue_idx);
	if(txdesc_new == NULL || TXDESC_STA_USED == txdesc_new->status) {
		rwm_node_free(node);
		goto exit;
	}

	txdesc_new->status = TXDESC_STA_USED;
	txdesc_new->host.flags = TXU_CNTRL_MGMT;
	txdesc_new->host.msdu_node = (void *)node;
	txdesc_new->host.orig_addr = (UINT32)node->msdu_ptr;
	txdesc_new->host.packet_addr = (UINT32)content_ptr;
	txdesc_new->host.packet_len = len;
	txdesc_new->host.status_desc_addr = (UINT32)content_ptr;
	txdesc_new->host.tid = 0xff;

	umac = &txdesc_new->umac;
	umac->payl_len = len;
	umac->head_len = 0;
	umac->tail_len = 0;
	umac->hdr_len_802_2 = 0;

	umac->buf_control = &txl_buffer_control_24G;

	txdesc_new->lmac.agg_desc = NULL;
	txdesc_new->lmac.hw_desc->cfm.status = 0;

    ps_set_data_prevent();
#if CFG_USE_STA_PS
    bmsg_ps_handler_rf_ps_mode_real_wakeup();
    bk_wlan_dtim_rf_ps_mode_do_wakeup();
#endif

	txl_cntrl_push(txdesc_new, queue_idx);

exit:
	os_free(pkt);
}

void bmsg_ioctl_handler(BUS_MSG_T *msg)
{
    ke_msg_send((void *)msg->arg);
}

void bmsg_music_handler(BUS_MSG_T *msg)
{
#if (CONFIG_APP_MP3PLAYER == 1)
    media_msg_sender((void *)msg->arg);
#endif
}

void bmsg_skt_tx_sender(void *arg)
{
    OSStatus ret;
    BUS_MSG_T msg;

    msg.type = BMSG_SKT_TX_TYPE;
    msg.arg = (uint32_t)arg;
    msg.len = 0;
    msg.sema = NULL;

    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
    if(kNoErr != ret)
    {
        os_printf("bmsg_rx_sender_failed\r\n");
    }
}

extern void power_save_wait_timer_real_handler(void );
extern void power_save_wait_timer_start(void);

void ps_msg_process(UINT8 ps_msg)
{
    switch(ps_msg)
    {
#if CFG_USE_STA_PS
    case PS_BMSG_IOCTL_RF_ENABLE:
        power_save_dtim_enable();
        break;

    case PS_BMSG_IOCTL_RF_USER_WKUP:
        bmsg_ps_handler_rf_ps_mode_real_wakeup();
        break;

    case PS_BMSG_IOCTL_RF_DISANABLE:
        bmsg_ps_handler_rf_ps_mode_real_wakeup();
        power_save_dtim_disable();
        break;
#endif
#if CFG_USE_MCU_PS
    case PS_BMSG_IOCTL_MCU_ENABLE:
        mcu_ps_init();
        break;

    case PS_BMSG_IOCTL_MCU_DISANABLE:
        mcu_ps_exit();
        break;
#endif
#if CFG_USE_STA_PS
    case PS_BMSG_IOCTL_RF_TD_SET:
        ps_set_td_timer();
        break;

        #if PS_USE_KEEP_TIMER
        case PS_BMSG_IOCTL_RF_KP_HANDLER:
            power_save_keep_timer_real_handler();
            break;

        case PS_BMSG_IOCTL_RF_KP_SET:
            power_save_keep_timer_set();
            break;

        case PS_BMSG_IOCTL_RF_KP_STOP:
            power_save_keep_timer_stop();
            break;
		#endif

		#if PS_USE_WAIT_TIMER
        case PS_BMSG_IOCTL_WAIT_TM_HANDLER:
            power_save_wait_timer_real_handler();
            break;
        case PS_BMSG_IOCTL_WAIT_TM_SET:
            power_save_wait_timer_start();
            break;
		#endif
        case PS_BMSG_IOCTL_RF_PS_TIMER_INIT:
            power_save_set_keep_timer_time(20);
            break; 
			
        case PS_BMSG_IOCTL_RF_PS_TIMER_DEINIT:
            power_save_set_keep_timer_time(0);
            break; 
#endif
#if CFG_USE_AP_IDLE
    case PS_BMSG_IOCTL_AP_PS_RUN:
        ap_bcn_timer_real_handler();
        break;

    case PS_BMSG_IOCTL_AP_PS_STOP:
        stop_global_ap_bcn_timer();
        break;

    case PS_BMSG_IOCTL_AP_PS_START:
        start_global_ap_bcn_timer();
        break;
#endif
        default:
            break;
    }
}

void bmsg_null_sender(void)
{
    OSStatus ret;
    BUS_MSG_T msg;

    msg.type = BMSG_NULL_TYPE;
    msg.arg = 0;
    msg.len = 0;
    msg.sema = NULL;

    if(!rtos_is_queue_empty(&g_wifi_core.io_queue))
    {
        return;
    }

    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
    if(kNoErr != ret)
    {
        os_printf("bmsg_null_sender_failed\r\n");
    }
}

void bmsg_rx_sender(void *arg)
{
    OSStatus ret;
    BUS_MSG_T msg;
    GLOBAL_INT_DECLARATION();

    msg.type = BMSG_RX_TYPE;
    msg.arg = (uint32_t)arg;
    msg.len = 0;
    msg.sema = NULL;

    GLOBAL_INT_DISABLE();
    if(bmsg_rx_count >= 2)
    {
        GLOBAL_INT_RESTORE();
        return;
    }

    bmsg_rx_count += 1;
    GLOBAL_INT_RESTORE();

    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
    if(kNoErr != ret)
    {
        APP_PRT("bmsg_rx_sender_failed\r\n");
    }
}

int bmsg_tx_sender(struct pbuf *p, uint32_t vif_idx)
{
    OSStatus ret;
    BUS_MSG_T msg;

    msg.type = BMSG_TX_TYPE;
    msg.arg = (uint32_t)p;
    msg.len = vif_idx;
    msg.sema = NULL;

    pbuf_ref(p);
    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, 1 * SECONDS);
    if(kNoErr != ret)
    {
        APP_PRT("bmsg_tx_sender failed\r\n");
        pbuf_free(p);
    }

    return ret;
}

int bmsg_tx_raw_sender(uint8_t *payload, uint16_t length)
{
	OSStatus ret;
	BUS_MSG_T msg;

	msg.type = BMSG_TX_RAW_TYPE;
	msg.arg = (uint32_t)payload;
	msg.len = length;
	msg.sema = NULL;

	ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, 1*SECONDS);

	if(ret != kNoErr)
	{
		APP_PRT("bmsg_tx_sender failed\r\n");
		os_free(payload);
	}

	return ret;
}

#if (SUPPORT_LSIG_MONITOR)
void bmsg_rx_lsig_handler(BUS_MSG_T *msg)
{
	lsig_input((msg->arg&0xFFFF0000)>>16, msg->arg&0xFF, msg->len);
}

void bmsg_rx_lsig(uint16_t len, uint8_t rssi)
{
	BUS_MSG_T msg;

	msg.type = BMSG_RX_LSIG;
	msg.arg = (uint32_t)((len << 16) | rssi);
	msg.len = rtos_get_time();
	msg.sema = NULL;
	rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
}
#endif

int bmsg_ioctl_sender(void *arg)
{
    OSStatus ret;
    BUS_MSG_T msg;

    msg.type = BMSG_IOCTL_TYPE;
    msg.arg = (uint32_t)arg;
    msg.len = 0;
    msg.sema = NULL;

    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
    if(kNoErr != ret)
    {
        APP_PRT("bmsg_ioctl_sender_failed\r\n");
    }
    else
    {
        APP_PRT("bmsg_ioctl_sender\r\n");
    }

    return ret;
}

void bmsg_music_sender(void *arg)
{
    OSStatus ret;
    BUS_MSG_T msg;

    msg.type = BMSG_MEDIA_TYPE;
    msg.arg = (uint32_t)arg;
    msg.len = 0;
    msg.sema = NULL;

    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
    if(kNoErr != ret)
    {
        APP_PRT("bmsg_media_sender_failed\r\n");
    }
}

#if CFG_USE_AP_PS
void bmsg_txing_sender(uint8_t sta_idx)
{
    OSStatus ret;
    BUS_MSG_T msg;

    msg.type = BMSG_TXING_TYPE;
    msg.arg = (uint32_t)sta_idx;
    msg.len = 0;
    msg.sema = NULL;

    ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
    if(kNoErr != ret)
    {
        APP_PRT("bmsg_txing_sender failed\r\n");
    }
}

void bmsg_txing_handler(BUS_MSG_T *msg)
{
    OSStatus ret;
    UINT8 sta_idx = (UINT8)msg->arg;

    rwm_msdu_send_txing_node(sta_idx);
}
#endif

void bmsg_ps_sender(uint8_t arg)
{
    OSStatus ret;
    BUS_MSG_T msg;
    if(g_wifi_core.io_queue)
    {
        msg.type = BMSG_STA_PS_TYPE;
        msg.arg = (uint32_t)arg;
        msg.len = 0;
        msg.sema = NULL;

        ret = rtos_push_to_queue(&g_wifi_core.io_queue, &msg, BEKEN_NO_WAIT);
        if(kNoErr != ret)
        {
            os_printf("bmsg_ps_sender failed\r\n");
        }
    }
    else
    {
        os_printf("g_wifi_core.io_queue null\r\n");
    }
}
#if CFG_USE_STA_PS

void bmsg_ps_handler(BUS_MSG_T *msg)
{
    UINT8 arg;

    arg = (UINT8)msg->arg;
    ps_msg_process(arg);
}
#endif
static void core_thread_main( void *arg )
{
    OSStatus ret;
    BUS_MSG_T msg;
    uint8_t ke_skip = 0;
#if CFG_USE_STA_PS
    uint8_t ps_flag = 0;
#endif

    while(1)
    {
        ret = rtos_pop_from_queue(&g_wifi_core.io_queue, &msg, BEKEN_WAIT_FOREVER);
        if(kNoErr == ret)
        {
            switch(msg.type)
            {
#if CFG_USE_STA_PS
            case BMSG_STA_PS_TYPE:
                if(msg.arg == PS_BMSG_IOCTL_RF_DISANABLE)
                {
                    bmsg_ps_handler(&msg);
                }
                else
                {
                    ps_flag = 1;
                }
                break;
#endif

            case BMSG_RX_TYPE:
                APP_PRT("bmsg_rx_handler\r\n");
                bmsg_rx_handler(&msg);
                break;

            case BMSG_TX_TYPE:
                APP_PRT("bmsg_tx_handler\r\n");
                bmsg_tx_handler(&msg);
                break;

            case BMSG_SKT_TX_TYPE:
                APP_PRT("bmsg_skt_tx_handler\r\n");
                bmsg_skt_tx_handler(&msg);
                break;

            case BMSG_IOCTL_TYPE:
                APP_PRT("bmsg_ioctl_handler\r\n");
                bmsg_ioctl_handler(&msg);
                break;
            case BMSG_MEDIA_TYPE:
                ke_skip = 1;
                bmsg_music_handler(&msg);
                break;

#if CFG_USE_AP_PS
            case BMSG_TXING_TYPE:
                bmsg_txing_handler(&msg);
                break;
#endif

			case BMSG_TX_RAW_TYPE:
				bmsg_tx_raw_handler(&msg);
				break;

            case BMSG_TX_RAW_CB_TYPE:
                bmsg_tx_raw_cb_handler(&msg);
                break;

#if (SUPPORT_LSIG_MONITOR)
			case BMSG_RX_LSIG:
				bmsg_rx_lsig_handler(&msg);
				break;
#endif
            default:
                APP_PRT("unknown_msg\r\n");
                break;
            }

            if (msg.sema != NULL)
            {
                rtos_set_semaphore(&msg.sema);
            }
            if(!ke_skip)
                ke_evt_core_scheduler();
            else
                ke_skip = 0;
        }

#if CFG_USE_STA_PS
        if(ps_flag == 1)
        {
            bmsg_ps_handler(&msg);
            ps_flag = 0;
        }
        power_save_rf_sleep_check();
#endif

    }
}

void core_thread_init(void){
    OSStatus ret;

    g_wifi_core.queue_item_count = CORE_QITEM_COUNT;
    g_wifi_core.stack_size = CORE_STACK_SIZE;

    ret = rtos_init_queue(&g_wifi_core.io_queue,
                          "core_queue",
                          sizeof(BUS_MSG_T),
                          g_wifi_core.queue_item_count);
    if (kNoErr != ret)
    {
        os_printf("Create io queue failed\r\n");
        goto fail;
    }

    ret = rtos_create_thread(&g_wifi_core.handle,
                             THD_CORE_PRIORITY,
                             "core_thread",
                             (beken_thread_function_t)core_thread_main,
                             (unsigned short)g_wifi_core.stack_size,
                             (beken_thread_arg_t)0);
    if (kNoErr != ret)
    {
        os_printf("Create core thread failed\r\n");
        goto fail;
    }

    return;

fail:
    core_thread_uninit();

    return;
}

void core_thread_uninit(void)
{
    if(g_wifi_core.handle)
    {
        rtos_delete_thread(&g_wifi_core.handle);
        g_wifi_core.handle = 0;
    }

    if(g_wifi_core.io_queue)
    {
        rtos_deinit_queue(&g_wifi_core.io_queue);
        g_wifi_core.io_queue = 0;
    }

    g_wifi_core.queue_item_count = 0;
    g_wifi_core.stack_size = 0;
}


static void init_app_thread( void *arg ){

    rtos_delete_thread( NULL );
}

/*
void scan_camera_sensors(int argc, char **argv){

    os_printf("argc %d", argc);

    DJPEG_DESC_ST ejpeg_cfg;

    UINT32 status;

    DD_HANDLE ejpeg_hdl = ddev_open(EJPEG_DEV_NAME, &status, (UINT32)&ejpeg_cfg);

    os_printf("open EJPEG %p\r\n", ejpeg_hdl);
    os_printf("status: %d\r\n", status);

    UINT32 i2c2_trans_mode = (0 & (~I2C2_MSG_WORK_MODE_MS_BIT)		// master
						 & (~I2C2_MSG_WORK_MODE_AL_BIT)) 	// 7bit address
						 | ( I2C2_MSG_WORK_MODE_IA_BIT);	

    //UINT32 oflag = 0;

    DD_HANDLE i2c_hdl = ddev_open(I2C2_DEV_NAME, &status, i2c2_trans_mode);
    os_printf("open I2C2\r\n");
    os_printf("status: %d\r\n", status);

    unsigned char data;
    I2C_OP_ST i2c_operater;


    //status = ddev_write(i2c_hdl, (char *)&data, 1, (UINT32)&i2c_operater);


    //i2c_operater.addr_width = ADDR_WIDTH_8;

    
    for(int i = 0; i < 128; i++){

        i2c_operater.salve_id = (i << 1) | 1;

        i2c_operater.op_addr = 0x0F0;

        data = 0x00;

        //status = ddev_write(i2c_hdl, (char *) data, 1, (UINT32)&i2c_operater);

        //os_printf("\nREAD -> Status: %d, Data: %d %d %d %d", status, data[0], data[1], data[2], data[3]);

        status = ddev_read(i2c_hdl, (char *) &data, 1, (UINT32)&i2c_operater);

        os_printf("READ -> Status: %d, Device: %x, Addr: %x, Data: %x \n\n", status, i2c_operater.salve_id, i2c_operater.op_addr, data);

    }


}

void gpio_read(int argc, char **argv){

    if (argc == 2){

        int pin = atoi(argv[1]);

        bk_gpio_config_input_pup((GPIO_INDEX) pin);

        for (int i = 0; i < 10; i ++){
            int value = bk_gpio_input((GPIO_INDEX)pin);   
            bk_printf("Reading pin %d: value %d\n", pin, value);
            delay100us(10000); // 1s
        }

    }
}

void gpio_write(int argc, char **argv){

    if (argc == 3){

        int pin = atoi(argv[1]);
        int value = atoi(argv[2]) == 1;

        bk_printf("Writting pin %d to value %d\n", pin, value);

        bk_gpio_config_output((GPIO_INDEX) pin);
        bk_gpio_output((GPIO_INDEX)pin, value);

    }
}

void gpio_test_loop(int argc, char **argv){

    if (argc == 1){

        os_printf("Testing GPIOS");

        for(int i = 2; i < 40; i++){

            if(i != 10 && i != 11){

                bk_gpio_config_output((GPIO_INDEX) i);

                os_printf("PIN TESTED: %d\n", i);

                bk_gpio_output((GPIO_INDEX)i, 1);
                delay100us(1000);
                bk_gpio_output((GPIO_INDEX)i, 0);
                delay100us(1000);
                bk_gpio_output((GPIO_INDEX)i, 1);
                delay100us(1000);
                bk_gpio_output((GPIO_INDEX)i, 0);

                bk_gpio_config_input((GPIO_INDEX) i);

                delay100us(1*10000); // 1s

            } 

        }

    }
}
*/

void fancy_msg(void){
    os_printf("\r\n");
    os_printf("  ___ _  ______ ___ ___ ___    ___                 ___            \r\n");
    os_printf(" | _ ) |/ /__  |_  ) __|_  )  / _ \\ _ __  ___ _ _ / __|__ _ _ __  \r\n");
    os_printf(" | _ \\ ' <  / / / /|__ \\/ /  | (_) | '_ \\/ -_) ' \\ (__/ _` | '  \\ \r\n");
    os_printf(" |___/_|\\_\\/_/ /___|___/___|  \\___/| .__/\\___|_||_\\___\\__,_|_|_|_|\r\n");
    os_printf("\r\n");
}

void app_pre_start(void){

    OSStatus ret;

    ret = rtos_init_semaphore(&app_sema, 1);

    ASSERT(kNoErr == ret);

    ret = rtos_create_thread(&app_thread_handle,
                             THD_APPLICATION_PRIORITY,
                             "kmsgbk",
                             (beken_thread_function_t)kmsg_bk_thread_main,
                             (unsigned short)app_stack_size,
                             (beken_thread_arg_t)0);
    ASSERT(kNoErr == ret);

    ret = rtos_create_thread(&init_thread_handle,
                             THD_INIT_PRIORITY,
                             "init_thread",
                             (beken_thread_function_t)init_thread_main,
                             (unsigned short)init_stack_size,
                             (beken_thread_arg_t)0);
    ASSERT(kNoErr == ret);

    core_thread_init();

    rf_thread_init();


#if ((CFG_SUPPORT_BLE) && (CFG_BLE_VERSION == BLE_VERSION_5_x))
    extern void ble_entry(void);
    ble_entry();
#endif
}

void user_main_entry(void){

    os_printf("\r\nuser main start\r\n");

    rtos_create_thread(&user_thread_handle,
                       THD_INIT_PRIORITY,
                       "app",
                       (beken_thread_function_t)init_app_thread,
                       app_stack_size,
                       (beken_thread_arg_t)0);
}


static struct dfs_fd fd;
void run_init_script(void){

    char cmd[32];
    memset(cmd, 0, sizeof(cmd));
    
    char buf;
    uint8_t i = 0;

    int length;

    os_printf("Running init script: /sd/init.msh\r\n");

    if (dfs_file_open(&fd, "/sd/init.msh", O_RDONLY) < 0){
        os_printf("Init file open failed\r\n");
    }

    do {
        
        length = dfs_file_read(&fd, &buf, 1 );
        if (length > 0){
            if(buf != '\n' && buf != '\r'){
                //rt_kprintf("%c", buf);
                cmd[i] = buf;
                i++;

            }else{
                os_printf("\r\n ## Running command: %s\r\n", cmd);
                msh_exec(cmd, i);
                memset(cmd, 0, sizeof(cmd));
                i = 0;
            }
            
        }
    }while (length > 0);

    dfs_file_close(&fd);

    os_printf("Init script done.\r\n");

}


void app_start(void){

    app_pre_start();
    fancy_msg();

    delay100us(5*10000);
    run_init_script();
    
    
                                                                                                                                        
    //user_main_entry();

}


int bmsg_is_empty(void)
{
    if(!rtos_is_queue_empty(&g_wifi_core.io_queue))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*void arg_test(int argc, char **argv){
    os_printf("argc %d\n", argc);
}*/

//MSH_CMD_EXPORT(arg_test, arg test);
//MSH_CMD_EXPORT(scan_camera_sensors , scan camera sensors);
//MSH_CMD_EXPORT(gpio_write , Set GPIO <pin> <value>);
//MSH_CMD_EXPORT(gpio_read , Set GPIO <pin> <value>);
//MSH_CMD_EXPORT(gpio_test_loop, Loop over GPIOS)
//MSH_CMD_EXPORT(sdcard_intf_test, sdcard_intf_test);
// eof

