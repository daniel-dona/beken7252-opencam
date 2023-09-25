/* 长按 pwr 按键进入低功耗模式  */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include "multi_button.h"

#define DBG_ENABLE
#define DBG_LEVEL   DBG_LOG
#define DBG_COLOR
#define DBG_SECTION_NAME    "[PWR_key]"
#include <rtdbg.h>

static struct button pwr_btn;

#define BUTTON_PWR_BTN          (7)

static void pwr_btn_handler(void *btn);

extern void pwr_enter_sleepmode(void);

#define PWR_FLAG_INIT      (-1)
#define PWR_FLAG_START     (1)
#define PWR_FLAG_FINSH     (2)
static int pwr_flag = PWR_FLAG_INIT; //检测按键长按后松开，进入低功耗模式。 

static void button_callback(void *btn)
{
    if(btn == & pwr_btn)
    {
        pwr_btn_handler(btn);
    }
}


static void pwr_btn_handler(void *btn)
{
    rt_uint8_t btn_event_val;
    

    btn_event_val = get_button_event((struct button *)btn);

    /*btn --> event */
    switch (btn_event_val)
    {
    case PRESS_DOWN:
         LOG_I("pwr_btn press down \n");
        break;
    case PRESS_UP:
         if(pwr_flag == 1)
         {
            pwr_flag = PWR_FLAG_FINSH;
            LOG_I("pwr_btn enter deepsleep \n");
            pwr_enter_sleepmode();
         }
         LOG_I("pwr_btn press up \n");
        break;

    // case PRESS_REPEAT: 
    //     LOG_I("up_btn press repeat\n");
    //     break; 

    // case SINGLE_CLICK: 
    //     LOG_I("up_btn single click\n");
    //     break; 

    // case DOUBLE_CLICK: 
    //     LOG_I("up_btn double click\n");
    //     break; 

    case LONG_RRESS_START: 
        LOG_I("pwr_btn long press start\n");
        pwr_flag = PWR_FLAG_START;
        //pwr_enter_sleepmode();
        break; 

    case LONG_PRESS_HOLD: 
        LOG_I("pwr_btn long press hold\n");
        break; 
    
    default:
        break;
    }
}

static rt_uint8_t pwr_btn_read_pin(void)
{
    return rt_pin_read(BUTTON_PWR_BTN);
}

static void pwr_btn_thread_entry(void *p)
{
    while(1)
    {
        /* 20 ms */
        rt_thread_mdelay(10);
        button_ticks();
    }
}

int pwr_btn_test(int argc, char *argv[])
{
    rt_thread_t thread = RT_NULL;

    /* Create background ticks thread */
    thread = rt_thread_create("pwr_btn",pwr_btn_thread_entry,RT_NULL,1024 *2,10,10);
    if(thread == RT_NULL)
    {
        rt_kprintf("pwr_btn thread create failed !!!\n");
        return -RT_ERROR;
    }
    rt_thread_startup(thread);

    /* low level driver  */
    rt_pin_mode(BUTTON_PWR_BTN,PIN_MODE_INPUT_PULLUP);
    button_init(&pwr_btn,pwr_btn_read_pin,PIN_HIGH);
    button_attach(&pwr_btn, PRESS_DOWN,    button_callback);
    button_attach(&pwr_btn, PRESS_UP,         button_callback);
    // button_attach(&pwr_btn, PRESS_REPEAT,     button_callback);
    // button_attach(&pwr_btn, SINGLE_CLICK,     button_callback);
    // button_attach(&pwr_btn, DOUBLE_CLICK,     button_callback);
    button_attach(&pwr_btn, LONG_RRESS_START, button_callback);
    button_attach(&pwr_btn, LONG_PRESS_HOLD,  button_callback);
    button_start (&pwr_btn);

    return RT_EOK; 

}
INIT_APP_EXPORT(pwr_btn_test);
MSH_CMD_EXPORT(pwr_btn_test,pwr_btn_test);