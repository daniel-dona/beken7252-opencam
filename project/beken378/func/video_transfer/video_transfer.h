#ifndef __VIDEO_TRANS_H__
#define __VIDEO_TRANS_H__

typedef enum {
    QVGA_320_240    = 0,
    VGA_640_480,
    PPI_MAX
} PPI_TYPE; // Pixel per inch

typedef enum {
    TYPE_5FPS            = 0,
    TYPE_10FPS,
    TYPE_20FPS,
    FPS_MAX
} FPS_TYPE; // frame per second

#define PPI_POSI        0
#define PPI_MASK        0xFF
#define FPS_POSI        8
#define FPS_MASK        0xFF

#define CMPARAM_SET_PPI(p, x)   (p = ((p & ~(PPI_MASK << PPI_POSI)) | ((x & PPI_MASK) << PPI_POSI)))
#define CMPARAM_GET_PPI(p)      ((p >> PPI_POSI) & PPI_MASK)

#define CMPARAM_SET_FPS(p, x)   (p = ((p & ~(FPS_MASK << FPS_POSI)) | ((x & FPS_MASK) << FPS_POSI)))
#define CMPARAM_GET_FPS(p)      ((p >> FPS_POSI) & FPS_MASK)

typedef enum {
    TVIDEO_SND_NONE         = 0LU,
    TVIDEO_SND_UDP,
    TVIDEO_SND_TCP,
    TVIDEO_SND_INTF,
    TVIDEO_SND_BUFFER,
} TVIDEO_SND_TYPE;

typedef struct tvideo_desc
{
    UINT8 *rxbuf;
    
    void (*node_full_handler)(void *curptr, UINT32 newlen, UINT32 is_eof, UINT32 frame_len);
    void (*data_end_handler)(void);   

    UINT16 rxbuf_len;
    UINT16 rx_read_len;
    UINT32 node_len;
    UINT32 sener_cfg;
} TVIDEO_DESC_ST, *TVIDEO_DESC_PTR;

typedef struct tvideo_hdr_param
{
    UINT8* ptk_ptr;
    UINT32 ptklen;
    UINT32 frame_id;
    UINT32 is_eof;
    UINT32 frame_len;
}TV_HDR_PARAM_ST, *TV_HDR_PARAM_PTR;

typedef void (*tvideo_add_pkt_header)(TV_HDR_PARAM_PTR param); 
typedef int (*video_transfer_send_func)(UINT8 *data, UINT32 pos, UINT32 len, UINT8 is_stop);
typedef void (*video_transfer_start_cb)(void); 
typedef void (*video_transfer_end_cb)(void); 

typedef struct tvideo_setup_desc
{
    UINT32 send_type;
    video_transfer_send_func send_func;
    video_transfer_start_cb start_cb;
    video_transfer_start_cb end_cb;
    
    UINT32 pkt_header_size;
    tvideo_add_pkt_header add_pkt_header;
} TVIDEO_SETUP_DESC_ST, *TVIDEO_SETUP_DESC_PTR;

#if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)
void tvideo_intfer_send_msg(UINT32 new_msg);
UINT32 video_transfer_init(TVIDEO_SETUP_DESC_PTR setup_cfg);
UINT32 video_transfer_deinit(void);
UINT32 video_transfer_set_video_param(UINT32 ppi, UINT32 fps);
void tvideo_capture(UINT32 action);

void video_buffer_open (void);
void video_buffer_close (void);
UINT32 video_buffer_read_frame(UINT8 *buf, UINT32 buf_len);
#endif

#endif // __VIDEO_TRANS_H__
