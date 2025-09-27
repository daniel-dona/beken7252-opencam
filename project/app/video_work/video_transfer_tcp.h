#ifndef _APP_DEMO_TCP_H_
#define _APP_DEMO_TCP_H_

UINT32 app_demo_tcp_init(void);
void app_demo_tcp_deinit(void);
int app_demo_tcp_send_packet(UINT8 *data, UINT32 len);

#endif // _APP_DEMO_TCP_H_

