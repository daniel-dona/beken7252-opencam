#ifndef __VOICE_TRANS_H__
#define __VOICE_TRANS_H__

typedef int (*video_transfer_send_func)(UINT8 *data, UINT32 len); 

UINT32 tvoice_transfer_init(video_transfer_send_func send_func);
UINT32 tvoice_transfer_deinit(void);

#endif // __VOICE_TRANS_H__