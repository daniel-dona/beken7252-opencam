#include <rtthread.h> 
#include <rtdevice.h> 
#include "multi_button.h"

static struct button btn;

#define BUTTON_PIN (10)

static uint8_t button_read_pin(void) 
{
    return rt_pin_read(BUTTON_PIN); 
}

void btn_test_thread_entry(void *p)
{
    uint32_t btn_event_val; 
    
    while(1)
    {
        if(btn_event_val != get_button_event(&btn)) 
        {
            btn_event_val = get_button_event(&btn);

            switch(btn_event_val)
            {
            case PRESS_DOWN:
                rt_kprintf("button press down\n"); 
            break; 

            case PRESS_UP: 
                rt_kprintf("button press up\n");
            break; 

            case PRESS_REPEAT: 
                rt_kprintf("button press repeat\n");
            break; 

            case SINGLE_CLICK: 
                rt_kprintf("button single click\n");
            break; 

            case DOUBLE_CLICK: 
                rt_kprintf("button double click\n");
            break; 

            case LONG_RRESS_START: 
                rt_kprintf("button long press start\n");
            break; 

            case LONG_PRESS_HOLD: 
                rt_kprintf("button long press hold\n");
            break; 
            }
        }
        
        button_ticks(); 
        rt_thread_delay(RT_TICK_PER_SECOND/200); 
    }
}

int multi_button_test(void)
{
    rt_thread_t thread = RT_NULL;

    thread = rt_thread_create("btn_test", btn_test_thread_entry, RT_NULL, 1024, 15, 10);
    if(thread == RT_NULL)
    {
        return RT_ERROR; 
    }
    rt_thread_startup(thread);

    /* low level drive */
    rt_pin_mode (BUTTON_PIN, PIN_MODE_INPUT); 
    button_init (&btn, button_read_pin, PIN_LOW);
    button_start(&btn);

    return RT_EOK; 
}
INIT_APP_EXPORT(multi_button_test); 
