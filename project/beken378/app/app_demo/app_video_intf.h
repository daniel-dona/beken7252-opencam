#ifndef _APP_VIDEO_INTF_H__
#define _APP_VIDEO_INTF_H__

void app_video_intf_open (void);
void app_video_intf_close (void);
int app_video_intf_send_packet (UINT8 *data, UINT32 len);
    
#endif // _APP_VIDEO_INTF_H__
