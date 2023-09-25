/*
 * File: stream_pipe.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __STREAM_PIPE_H__
#define __STREAM_PIPE_H__

#define PIPE_START_OF_STREAM    0
#define PIPE_END_OF_STREAM      1

enum STREAM_PIPE_EVENT
{
    STREAM_PIPE_EMPTY, 
    STREAM_PIPE_RESUME, 
};

/* ring buffer */
struct rb_buffer
{
    rt_uint8_t *buffer_ptr;

    rt_uint32_t read_mirror : 1;
    rt_uint32_t read_index : 31;

    rt_uint32_t write_mirror : 1;
    rt_uint32_t write_index : 31;
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    rt_int32_t buffer_size;
};

struct stream_pipe
{
    /* ring buffer in the stream pipe */
    struct rb_buffer ringbuffer;

    uint32_t writer_resume_wm;  /* water marker to resume writer */
    uint32_t reader_ready_wm;   /* water marker for reader ready */

    uint16_t eos;               /* end of stream */
    uint16_t reserv;            /* reserved field */

    /* suspended reader and writer */
    rt_list_t suspended_reader;
    rt_list_t suspended_writer;

    /* mutex lock */
    rt_mutex_t mutex;
};

struct stream_pipe *stream_pipe_create(uint8_t *buffer, size_t size);
void stream_pipe_destroy(struct stream_pipe *pipe);

void stream_pipe_reset(struct stream_pipe *pipe);

void stream_pipe_set_writer_wm(struct stream_pipe *pipe, uint32_t wm);
void stream_pipe_set_reader_wm(struct stream_pipe *pipe, uint32_t wm);
void stream_pipe_set_eos(struct stream_pipe *pipe, int eof);

int stream_pipe_write(struct stream_pipe *pipe, const void *buf, uint32_t size, int timeout);
int stream_pipe_read(struct stream_pipe *pipe, void *buf, uint32_t size, int timeout);

rt_uint32_t rb_buffer_data_len(struct rb_buffer *rb); 
int stream_pipe_set_event_callback(void (*callback)(int event, void *user_data), void *user_data); 

#endif
