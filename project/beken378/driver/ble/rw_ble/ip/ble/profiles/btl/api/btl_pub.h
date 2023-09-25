#ifndef _BTL_PUB_H
#define _BTL_PUB_H

typedef enum
{
    BLE_STACK_OK,
    BLE_STACK_FAIL,
    BLE_CONNECT,
    BLE_DISCONNECT,
    BLE_MTU_CHANGE
} event_t;

typedef void (*recieve_cb_t)(void *buf, uint16_t len);
typedef void (*event_cb_t)(event_t event, void *param);

extern recieve_cb_t recieve_cb;
extern event_cb_t event_cb;

#endif
