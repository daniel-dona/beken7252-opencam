/*
 * @File:    multi_button.c
 * @Author:  Zibin Zheng
 * @Date:    2018-01-23 20:36:01 
 * 
 * @LICENSE: Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 *           All rights reserved. 
 * 
 * @NOTE:    The original author of MultiButton is Zibin Zheng.
 *           Please contact the original author for authorization 
 *           before use.I(liu2guang) only adapt the library to 
 *           rt-thread and fix some bugs. I(liu2guang) am not 
 *           responsible for the authorization of the library.
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-01-23     liu2guang    Adapter rtthread and Fix the bug. 
 */ 
#include "multi_button.h" 

#define EVENT_CB(ev) if(handle->cb[ev]) handle->cb[ev]((button*)handle)
    
static struct button* head_handle = NULL;

/**
  * @brief  Initializes the button struct handle.
  * @param  handle: the button handle strcut.
  * @param  pin_level: read the pin of the connet button level.
  * @param  active_level: pin pressed level.
  * @retval None
  */
void button_init(struct button* handle, uint8_t(*pin_level)(void), uint8_t active_level)
{
    memset(handle, 0, sizeof(struct button));
    handle->event = (uint8_t)NONE_PRESS;
    handle->hal_button_Level = pin_level;
    handle->button_level = handle->hal_button_Level();
    handle->active_level = active_level;
}

/**
  * @brief  Attach the button event callback function.
  * @param  handle: the button handle strcut.
  * @param  event: trigger event type.
  * @param  cb: callback function.
  * @retval None
  */
void button_attach(struct button* handle, PressEvent event, BtnCallback cb)
{
    handle->cb[event] = cb;
}

/**
  * @brief  Inquire the button event happen.
  * @param  handle: the button handle strcut.
  * @retval button event.
  */
PressEvent get_button_event(struct button* handle)
{
    return (PressEvent)(handle->event);
}

/**
  * @brief  button driver core function, driver state machine.
  * @param  handle: the button handle strcut.
  * @retval None
  */
void button_handler(struct button* handle)
{
    uint8_t read_gpio_level = handle->hal_button_Level();

    //ticks counter working..
    if((handle->state) > 0) 
    {
        handle->ticks++;
    }

    /*------------button debounce handle---------------*/
    if(read_gpio_level != handle->button_level)
    { 
        //not equal to prev one
        //continue read 3 times same new level change
        if(++(handle->debounce_cnt) >= DEBOUNCE_TICKS) 
        {
            handle->button_level = read_gpio_level;
            handle->debounce_cnt = 0;
        }
    } 
    else 
    { 
        // leved not change ,counter reset.
        handle->debounce_cnt = 0;
    }

    /*-----------------State machine-------------------*/
    switch (handle->state) 
    {
    case 0:
        if(handle->button_level == handle->active_level) 
        { 
            handle->event = (uint8_t)PRESS_DOWN;
            EVENT_CB(PRESS_DOWN);
            handle->ticks  = 0;
            handle->repeat = 1;
            handle->state  = 1; 
        } 
        else 
        {
            handle->event = (uint8_t)NONE_PRESS;
        }
        break;

    case 1:
        if(handle->button_level != handle->active_level)
        { 
            handle->event = (uint8_t)PRESS_UP;
            EVENT_CB(PRESS_UP);
            handle->ticks = 0;
            handle->state = 2; 

        } 
        else if(handle->ticks > LONG_TICKS)
        {
            handle->event = (uint8_t)LONG_RRESS_START;
            EVENT_CB(LONG_RRESS_START);
            handle->state = 5; 
        }
        break;

    case 2:
        if(handle->button_level == handle->active_level) 
        { 
            handle->event = (uint8_t)PRESS_DOWN;
            EVENT_CB(PRESS_DOWN);
            handle->repeat++;
            
            handle->event = (uint8_t)PRESS_REPEAT;
            EVENT_CB(PRESS_REPEAT);
            handle->ticks = 0;
            handle->state = 3;
            
        } 
        else if(handle->ticks > SHORT_TICKS) 
        { 
            if(handle->repeat == 1) 
            {
                handle->event = (uint8_t)SINGLE_CLICK;
                EVENT_CB(SINGLE_CLICK);
            } 
            else if(handle->repeat == 2) 
            {
                handle->event = (uint8_t)DOUBLE_CLICK;
                EVENT_CB(DOUBLE_CLICK); 
            }
            handle->state = 0;
        }
        break;

    case 3:
        if(handle->button_level != handle->active_level) 
        {
            handle->event = (uint8_t)PRESS_UP;
            EVENT_CB(PRESS_UP);
            
            if(handle->ticks < SHORT_TICKS) 
            {
                handle->ticks = 0;
                handle->state = 2; 
            } 
            else 
            {
                handle->state = 0;
            }
        }
        break;

    case 5:
        if(handle->button_level == handle->active_level) 
        {
            handle->event = (uint8_t)LONG_PRESS_HOLD;
            EVENT_CB(LONG_PRESS_HOLD);
        } 
        else 
        {
            handle->event = (uint8_t)PRESS_UP;
            EVENT_CB(PRESS_UP);
            
            handle->state = 0; 
        }
        break;
    }
}

/**
  * @brief  Start the button work, add the handle into work list.
  * @param  handle: target handle strcut.
  * @retval 0: succeed. -1: already exist.
  */
int button_start(struct button* handle)
{
    struct button* target = head_handle;
    
    while(target) 
    {
        if(target == handle) 
        {
            return -1;  //already exist.
        }
        
        target = target->next;
    }
    
    handle->next = head_handle;
    head_handle = handle;
    
    return 0;
}

/**
  * @brief  Stop the button work, remove the handle off work list.
  * @param  handle: target handle strcut.
  * @retval None
  */
void button_stop(struct button* handle)
{
    struct button** curr;
    
    for(curr = &head_handle; *curr;) 
    {
        struct button* entry = *curr;
        
        if (entry == handle) 
        {
            *curr = entry->next;
        } 
        else
        {
            curr = &entry->next;
        }
    }
}

/**
  * @brief  background ticks, timer repeat invoking interval 5ms.
  * @param  None.
  * @retval None
  */
void button_ticks(void)
{
    struct button* target;
    
    for(target = head_handle; target != NULL; target = target->next)
    {
        button_handler(target);
    }
}
