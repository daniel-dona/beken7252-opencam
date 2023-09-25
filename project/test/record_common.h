#ifndef __RECORD_COMMON_H__
#define __RECORD_COMMON_H__

struct net_worker
{
    char *url;
    int port;
    int sock;
};

typedef struct record_msg
{
    uint32_t type;
    uint32_t arg;
    uint32_t len;
} record_msg_t;

enum record_msg_type
{
    RECORD_MSG_DATA = 0,
    RECORD_MSG_CMD = 1
};

rt_size_t rt_sound_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);

void int_to_char_little_endian(int i, unsigned char ch[4]);
void int_to_char_big_endian(int i, unsigned char ch[4]);

int tcp_client_init(struct net_worker *client);

#endif
