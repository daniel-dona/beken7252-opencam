/*
本例程实现摄像头拍摄图片解码后显示到 LCD 屏上
*/

#include "rtthread.h"
#include "include.h"

#include "app_demo_softap.h"
#include "video_transfer.h"

#include <sys/socket.h>
#include "netdb.h"
#include "lwip/netif.h"

//添加图片解码的头文件
#include "tjpgd.h"

//添加文件操作使用需要的头文件
#include <dfs_posix.h>

static void hexdump(const rt_uint8_t *p, rt_size_t len)
{
    unsigned char *buf = (unsigned char*)p;
    int i, j;
    
    rt_kprintf("Dump 0x%.8x %dBytes\n", (int)p, len); 
    rt_kprintf("Offset    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n"); 

    for (i=0; i<len; i+=16)
    {
        rt_kprintf("%08X: ", i+(int)p);

        for (j=0; j<16; j++)
        {
            if (i+j < len)
            {
                rt_kprintf("%02X ", buf[i+j]);
            }
            else
            {
                rt_kprintf("   ");
            }
        } 
        rt_kprintf(" ");

        for (j=0; j<16; j++)
        {
            if (i+j < len)
            {
                rt_kprintf("%c", ((unsigned int)((buf[i+j]) - ' ') < 127u - ' ') ? buf[i+j] : '.');
            }
        }
        rt_kprintf("\n");
    }
}

// 创建 mjpeg 会话的结构体
static struct mjpeg_session
{
    int sock;
    int connected;
    char* buf;
    int total_len;
    int last_frame_id;
    int recv_frame_flag;
    rt_tick_t old_tick;
    rt_event_t event;
    TVIDEO_SETUP_DESC_ST setup;
};

//初始化 
static struct mjpeg_session session = {0};

//定义一帧图片的大小
#define PIC_BUF_SIZE    		(1024 * 50)
#define SEND_FRAME_EVENT     	(1 << 0)
#define SEND_FRAME_COMPLETE 	(1 << 1)


static void camera_start(void);

static void start_cb(void)
{
    rt_kprintf("start cb\n");
}

static void stop_cb(void)
{
    rt_kprintf("stop cb\n");
}

static void pkt_header_cb(TV_HDR_PARAM_PTR param)
{
    HDR_PTR elem_tvhdr = (HDR_PTR)param->ptk_ptr;

    elem_tvhdr->id = (UINT8)param->frame_id;
    elem_tvhdr->is_eof = param->is_eof;
    elem_tvhdr->pkt_cnt = param->frame_len;
    elem_tvhdr->size = 0;
}

static int tvideo_capture_cb(UINT8 *data, UINT32 pos, UINT32 len, UINT8 is_stop)
{
    rt_err_t ret = 0;
	int temp = 0;

    session.total_len = 0;
    session.buf = data;
    session.total_len += len;

    rt_event_send(session.event, SEND_FRAME_EVENT);
    return len;
}

static void camera_start(void)
{
    static rt_uint8_t is_start = 0;

    if(is_start == 1) 
    {
        rt_kprintf("camera is already start\n");
        return;
    }

    session.setup.send_type = TVIDEO_SND_INTF;
    session.setup.send_func = tvideo_capture_cb;
    session.setup.start_cb = NULL;
    session.setup.end_cb = NULL;

    session.setup.pkt_header_size = sizeof(HDR_ST);
    session.setup.add_pkt_header = pkt_header_cb;

    video_transfer_init(&session.setup);

    is_start = 1;
}
static int start_flag = false;
static char file_name[15] = {0};
static int file_count = 0;

static void showphoto_thread(void* param)
{
    int ret;
    //创建摄像头接收一帧图片的事件
    session.event = rt_event_create("vt_event", RT_IPC_FLAG_FIFO);

    
    camera_start(); //开启摄像头传输照片

    while(1)
    {
        tvideo_capture(1);
        rt_event_recv(session.event, SEND_FRAME_EVENT, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, RT_NULL);

        //实时显示图片到 LCD 屏上
        //添加保存照片到本地文件的功能
        if (start_flag)
        {
            //保存文件到 sd 卡指定路径
            int fd, res;
            rt_sprintf(file_name, "/sd/temp.jpg", file_count);
            rt_kprintf("name = %s \n", file_name);
            fd = open(file_name, O_WRONLY | O_CREAT);
            if (fd >= 0)
            {
                write(fd, session.buf, session.total_len);
                close(fd);
                rt_kprintf("save %s ok!!!\n", file_name);
                res = Decode_Jpg(file_name);
                rt_kprintf("res = %d\n", res);
            }
            else
            {
                rt_kprintf("save pic failed!!!\n");
            }
           
        }
        tvideo_capture(0);
        
    }
    return;
    

}

void start_photo(void)
{
    start_flag = true;
}

void exit_photo(void)
{
    start_flag = false;
}
MSH_CMD_EXPORT(start_photo,start_photo);
MSH_CMD_EXPORT(exit_photo,exit_photo);

void show_photo_test(void)
{
    rt_thread_t tid1;
    tid1 = rt_thread_create("showphoto_thread", showphoto_thread, RT_NULL, 1024*4, 10, 10);
    rt_thread_startup(tid1);
}

MSH_CMD_EXPORT(show_photo_test,show camera photo);