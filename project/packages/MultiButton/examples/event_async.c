#include <rtthread.h> 
#include <rtdevice.h> 
#include "multi_button.h"

static struct button btn;

#define BUTTON_PIN (13)

static uint8_t button_read_pin(void) 
{
    return rt_pin_read(BUTTON_PIN); 
}

void button_callback(void *btn)
{
    uint32_t btn_event_val; 
    
    btn_event_val = get_button_event((struct button *)btn); 
    
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

void btn_thread_entry(void* p)
{
    while(1)
    {
        /* 5ms */
        rt_thread_delay(RT_TICK_PER_SECOND/200); 
        button_ticks(); 
    }
}

int multi_button_test(void)
{
    rt_thread_t thread = RT_NULL;
    
    /* Create background ticks thread */
    thread = rt_thread_create("btn", btn_thread_entry, RT_NULL, 1024, 10, 10);
    if(thread == RT_NULL)
    {
        return RT_ERROR; 
    }
    rt_thread_startup(thread);

    /* low level drive */
    rt_pin_mode  (BUTTON_PIN, PIN_MODE_INPUT_PULLUP); 
    button_init  (&btn, button_read_pin, PIN_LOW);
    button_attach(&btn, PRESS_DOWN,       button_callback);
    button_attach(&btn, PRESS_UP,         button_callback);
    button_attach(&btn, PRESS_REPEAT,     button_callback);
    button_attach(&btn, SINGLE_CLICK,     button_callback);
    button_attach(&btn, DOUBLE_CLICK,     button_callback);
    button_attach(&btn, LONG_RRESS_START, button_callback);
    button_attach(&btn, LONG_PRESS_HOLD,  button_callback);
    button_start (&btn);

    return RT_EOK; 
}
//INIT_APP_EXPORT(multi_button_test); 
MSH_CMD_EXPORT(multi_button_test,multi_button_test);