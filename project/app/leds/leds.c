#include <stdbool.h>

#include "rtos_pub.h"
#include "gpio_pub.h"
#include "target_util_pub.h"

#define LEDS_STACK_SIZE 1024
#define LEDS_THREAD_PRIORITY 10

#include "leds.h"

beken_thread_t leds_thread_handle;
rt_mq_t leds_mq_queue = NULL;

int leds_thread(void *arg){

    bool blue_led_status = 0;

    uint32_t received_value;

    while(1){

        if(leds_mq_queue != NULL){
            /* Receive a message from the queue, waiting forever if it's empty. */
            if (rt_mq_recv(leds_mq_queue, &received_value, sizeof(received_value), 50) == RT_EOK){
                //rt_kprintf("Receiver thread got: %u\n", received_value);
            }

        }else{
            // Queue expected to be init
            return -1;
        }

        switch(received_value){

            case LED_MODE_OFF:
                blue_led_status = 0;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);
                break;

            case LED_MODE_ON:
                blue_led_status = 1;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);
                break;

            case LED_MODE_BLINK_1_SLOW:
                blue_led_status = !blue_led_status;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);
                delay_ms(1000);
                break;

            case LED_MODE_BLINK_1_FAST:
                blue_led_status = !blue_led_status;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);
                delay_ms(250);
                break;

        }

    }

    return 0;

}

int leds_service(uint8_t argc, char **argv){

    leds_mq_queue = rt_mq_create("leds_mq", sizeof(uint32_t), 1, RT_IPC_FLAG_FIFO);

    OSStatus ret = rtos_create_thread(&leds_thread_handle,
                             LEDS_THREAD_PRIORITY,
                             "leds_service",
                             (beken_thread_function_t)leds_thread,
                             (unsigned short)LEDS_STACK_SIZE,
                             (beken_thread_arg_t)0);
    //ASSERT(kNoErr == ret);

    return (int) ret;

}


MSH_CMD_EXPORT(leds_service, Start leds service);
