#ifndef __MULTI_BUTTON_H_
#define __MULTI_BUTTON_H_

#include "stdint.h"
#include "string.h"

//According to your need to modify the constants.
#define TICKS_INTERVAL    5 //ms
#define DEBOUNCE_TICKS    3 //MAX 8
#define SHORT_TICKS       (300  / TICKS_INTERVAL)
#define LONG_TICKS        (1000 / TICKS_INTERVAL)

typedef void (*BtnCallback)(void*);

typedef enum {
    PRESS_DOWN = 0,
    PRESS_UP,
    PRESS_REPEAT,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    LONG_RRESS_START,
    LONG_PRESS_HOLD,
    number_of_event,
    NONE_PRESS
}PressEvent;

typedef struct button {
    uint16_t ticks;
    uint8_t  repeat       : 4;
    uint8_t  event        : 4;
    uint8_t  state        : 3;
    uint8_t  debounce_cnt : 3; 
    uint8_t  active_level : 1;
    uint8_t  button_level : 1;
    uint8_t  (*hal_button_Level)(void);
    BtnCallback  cb[number_of_event];
    struct button* next;
}button;

#ifdef __cplusplus  
extern "C" {  
#endif  

void button_init(struct button* handle, uint8_t(*pin_level)(void), uint8_t active_level);
void button_attach(struct button* handle, PressEvent event, BtnCallback cb);
PressEvent get_button_event(struct button* handle);
int  button_start(struct button* handle);
void button_stop(struct button* handle);
void button_ticks(void);

#ifdef __cplusplus
} 
#endif

#endif
