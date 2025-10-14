#include <stdint.h>
#include <stdlib.h>

#include "target_util_pub.h"
#include "i2c_pub.h"
#include "gpio_pub.h"

#include "debug.h"

void gpio_read(int argc, char **argv){

    if (argc == 2){

        int pin = atoi(argv[1]);

        bk_gpio_config_input_pup((GPIO_INDEX) pin);

        for (int i = 0; i < 10; i ++){
            int value = bk_gpio_input((GPIO_INDEX)pin);   
            bk_printf("Reading pin %d: value %d\n", pin, value);
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

        os_printf("Testing GPIOS\r\n");

        uint8_t v;

        for(int n = 0; n < 100; n++){

            for(int i = 2; i < 40; i++){

                if(i != 10 && i != 11){

                    //bk_gpio_config_output((GPIO_INDEX) i);

                    bk_gpio_config_input_pup((GPIO_INDEX) i);

                    v = bk_gpio_input((GPIO_INDEX) i);

                    os_printf("%d: %d, ", i, v);

                    //bk_gpio_config_input((GPIO_INDEX) i);

                    
                } 

            }

            os_printf("\r\n");
            delay_ms(500);

        }

        

        

    }
}


//MSH_CMD_EXPORT(scan_camera_sensors , scan camera sensors);
MSH_CMD_EXPORT(gpio_write , Set GPIO <pin> <value>);
MSH_CMD_EXPORT(gpio_read , Set GPIO <pin> <value>);
MSH_CMD_EXPORT(gpio_test_loop, Loop over GPIOS);
