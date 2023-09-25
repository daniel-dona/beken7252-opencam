/*
 * File: netstream.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __NETSTREAM_H__
#define __NETSTREAM_H__

#define NS_EVENT_OPEN_DONE      (1 << 0)
#define NS_EVENT_OPEN_FAILED    (1 << 1)
#define NS_EVENT_CLOSED         (1 << 2)
#define NS_EVENT_SEEK_DONE      (1 << 3)

#define NS_EVENT_SEEK_REQ       (1 << 8)
#define NS_EVENT_CLOSE_REQ      (1 << 9)

struct webclient_session;

struct network_stream_job
{
    /* network stream URI */
    char *URI;
    struct stream_pipe *pipe;

    /* worker --ack_event --> streamer */
    struct rt_event ack_event;

    /* seek position */
    uint32_t seek_position;

    /* current position and content length */
    uint32_t position;
    int      content_length;
    uint32_t codec_type;

    uint32_t user_data;
};

struct audio_stream* network_stream_open(const char* URI);
struct audio_stream* network_stream_open_websession(struct webclient_session* session);
struct audio_stream *network_stream_open_data(int codec_type, int len); 
int network_stream_init(void);
int _readline(struct webclient_session* session, char *line_buf, int size); 

#endif
