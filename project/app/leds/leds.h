#include <stdint.h>

#define LED_MODE_OFF            0
#define LED_MODE_ON             1 
#define LED_MODE_BLINK_1_SLOW   2
#define LED_MODE_BLINK_2_SLOW   3
#define LED_MODE_BLINK_1_FAST   4
#define LED_MODE_BLINK_2_FAST   5


int leds_service(uint8_t argc, char **argv);