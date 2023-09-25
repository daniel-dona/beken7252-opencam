#ifndef __CAMERA_INTF_PUB_H__
#define __CAMERA_INTF_PUB_H__

void camera_flip(UINT8 n);
void camera_intfer_init(void* data);
void camera_intfer_deinit(void);
void camera_intfer_set_video_param(UINT32 ppi_type, UINT32 pfs_type);

#endif // __CAMERA_INTF_PUB_H__
