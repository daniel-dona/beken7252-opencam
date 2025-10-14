#include <stdbool.h>

#include "rtos_pub.h"
#include "gpio_pub.h"
#include "target_util_pub.h"

#define LEDS_STACK_SIZE 1024
#define LEDS_THREAD_PRIORITY 10

#define LED_MODE_OFF            0
#define LED_MODE_ON             1 
#define LED_MODE_BLINK_1_SLOW   2
#define LED_MODE_BLINK_2_SLOW   3
#define LED_MODE_BLINK_1_FAST   4
#define LED_MODE_BLINK_2_FAST   5


beken_thread_t keys_thread_handle;
rt_mq_t keys_mq_queue = NULL;

int keys_thread(void *arg){

    bool blue_led_status = 0;

    uint32_t received_value;

    while(1){

        if(keys_mq_queue != NULL){
            /* Receive a message from the queue, waiting forever if it's empty. */
            if (rt_mq_recv(keys_mq_queue, &received_value, sizeof(received_value), 0) == RT_EOK){
                rt_kprintf("Receiver thread got: %d\n", received_value);
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

            case LED_MODE_ON:
                blue_led_status = 1;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);

            case LED_MODE_BLINK_1_SLOW:
                blue_led_status = !blue_led_status;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);
                delay_ms(1000);

            case LED_MODE_BLINK_1_FAST:
                blue_led_status = !blue_led_status;
                bk_gpio_config_output((GPIO_INDEX) 26);
                bk_gpio_output((GPIO_INDEX)26, blue_led_status);
                delay_ms(250);

        }

    }

    return 0;

}

void isr_test(unsigned char x){
    bk_printf("press %u\r\n", x);

    /*if(x == 2){
        rt_hw_cpu_reset();
    }*/
}

int keys_service(uint8_t argc, char **argv){

    keys_mq_queue = rt_mq_create("keys_mq", sizeof(uint32_t), 1, RT_IPC_FLAG_FIFO);


    //bk_gpio_config_input_pup((GPIO_INDEX) 2);
    //bk_gpio_config_input_pup((GPIO_INDEX) 7);


	/*int_param.id = SIMU_UART_GPIO_RX;
	int_param.mode = GMODE_INPUT_PULLDOWN;
	int_param.phandler = GPIO_Simu_Isr;*/

	//sddev_control((GPIO_INDEX) 2, CMD_GPIO_INT_ENABLE, &int_param);

    //gpio_int_enable((GPIO_INDEX) 2, GMODE_INPUT_PULLUP, isr_test);
    //gpio_int_enable((GPIO_INDEX) 7, GMODE_INPUT_PULLUP, isr_test);


    /*OSStatus ret = rtos_create_thread(&keys_thread_handle,
                             LEDS_THREAD_PRIORITY,
                             "keys_service",
                             (beken_thread_function_t)keys_thread,
                             (unsigned short)LEDS_STACK_SIZE,
                             (beken_thread_arg_t)0);*/
    //ASSERT(kNoErr == ret);

    return 0;

}


MSH_CMD_EXPORT(keys_service, Start keys service);
