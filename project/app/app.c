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


#include "i2c_pub.h"
#include "gpio_pub.h"
#include "jpeg_encoder_pub.h"
#include "target_util_pub.h"
#include "msh.h"
#include "easyflash.h"

#include "ap_idle_pub.h"
#include "arbitrate.h"
#include "ke_event.h"

// OpenCam includes
#include "leds.h"
#include "keys.h"


static struct dfs_fd fd;

beken_thread_t  vbat_thread_handle;

extern rt_mq_t vbat_mq_queue;
extern rt_mq_t leds_mq_queue;


void fancy_msg(void){

    bk_printf("\r\n");
    bk_printf(" ██████╗ ██████╗ ███████╗███╗   ██╗ ██████╗ █████╗ ███╗   ███╗\r\n");
    bk_printf("██╔═══██╗██╔══██╗██╔════╝████╗  ██║██╔════╝██╔══██╗████╗ ████║\r\n");
    bk_printf("██║   ██║██████╔╝█████╗  ██╔██╗ ██║██║     ███████║██╔████╔██║\r\n");
    bk_printf("██║   ██║██╔═══╝ ██╔══╝  ██║╚██╗██║██║     ██╔══██║██║╚██╔╝██║\r\n");
    bk_printf("╚██████╔╝██║     ███████╗██║ ╚████║╚██████╗██║  ██║██║ ╚═╝ ██║\r\n");
    bk_printf(" ╚═════╝ ╚═╝     ╚══════╝╚═╝  ╚═══╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚═╝\r\n");
    bk_printf("\r\n");

}





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


int mq_test_thread(void *arg){

    uint32_t received_value;

    while (1){

        if(vbat_mq_queue != NULL){
            /* Receive a message from the queue, waiting forever if it's empty. */
            if (rt_mq_recv(vbat_mq_queue, &received_value, sizeof(received_value), 0) == RT_EOK){

                float volts = 0.003334 * received_value - 9.295;

                char buff[32];

                snprintf(buff, 32, "volts: %.2f", volts);

                rt_kprintf("Receiver thread got: %d, volts: %s\n", received_value, buff);
            }

        }

        delay_ms(1000);
    }

}


void app_start(void){ //Execution from BDK


    leds_service(0, NULL);

    keys_service(0, NULL);

    if(leds_mq_queue != NULL){
        rt_mq_send(leds_mq_queue,  &(uint32_t){LED_MODE_BLINK_1_FAST}, sizeof(uint32_t));
    }

    //app_pre_start();

    easyflash_init();

    fancy_msg();

    delay_ms(1000);
    run_init_script();

    if(leds_mq_queue != NULL){
        rt_mq_send(leds_mq_queue, &(uint32_t){LED_MODE_ON}, sizeof(uint32_t));
    }

    // Board A9_B V1.3_220516

    OSStatus ret = rtos_create_thread(&vbat_thread_handle,
                             30,
                             "vbat_thread",
                             (beken_thread_function_t)mq_test_thread,
                             (unsigned short) 1024,
                             (beken_thread_arg_t)0);
    ASSERT(kNoErr == ret);
                                                                                                                                            
    //user_main_entry();

}

int leds_mode(int argc, char** argv){

    uint32_t mode = atoi(argv[1]);

    bk_printf("Mode: %u", mode);

    if(leds_mq_queue != NULL && argc == 2){
        rt_mq_send(leds_mq_queue, &mode, sizeof(mode));
    }

    return 0;
}

MSH_CMD_EXPORT(leds_mode, leds_mode);


