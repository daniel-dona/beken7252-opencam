#include "include.h"
#include "arm_arch.h"

#if CFG_USE_CAMERA_INTF
//#include "process.h"
#include "llc.h"

#include "video_transfer.h"

#include "jpeg_encoder_pub.h"
#include "i2c_pub.h"

#include "camera_intf.h"
#include "camera_intf_pub.h"

#include "drv_model_pub.h"
#include "general_dma_pub.h"
#include "mem_pub.h"
#include "bk_timer_pub.h"
#include "gpio_pub.h"

//Add for using rt i2c
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_iic.h"
#define CAMERA_I2CBUS_NAME "i2c2"
static struct rt_i2c_bus_device *camera_i2c_bus;


#define CAMERA_INTF_DEBUG
#include "uart_pub.h"
#ifdef CAMERA_INTF_DEBUG
#define CAMERA_INTF_PRT             os_printf
#define CAMERA_INTF_WPRT            warning_prf
#define CAMERA_INTF_FATAL           fatal_prf
#else
#define CAMERA_INTF_PRT             null_prf
#define CAMERA_INTF_WPRT            null_prf
#define CAMERA_INTF_FATAL           null_prf
#endif
#define CAMERA_RESET_GPIO_INDEX		GPIO16
#define CAMERA_RESET_HIGH_VAL       1          
#define CAMERA_RESET_LOW_VAL        0         
extern void delay100us(INT32 num);	

#define EJPEG_DMA_CHNAL             GDMA_CHANNEL_5
#define EJPEG_DELAY_HTIMER_CHNAL    3
#define EJPEG_DELAY_HTIMER_VAL      (2)  // 2ms
#define USE_JTAG_FOR_DEBUG          1

DJPEG_DESC_ST ejpeg_cfg;
DD_HANDLE i2c_hdl = DD_HANDLE_UNVALID, ejpeg_hdl = DD_HANDLE_UNVALID;
I2C_OP_ST i2c_operater;


int camera_hw_init(void)
{
    rt_uint8_t res;
    camera_i2c_bus = rt_device_find(CAMERA_I2CBUS_NAME);

    if (camera_i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find camera %s device\r\n",CAMERA_I2CBUS_NAME);
        return -RT_ERROR;
    }
    rt_kprintf("open camera %s device success!!!\r\n",CAMERA_I2CBUS_NAME);
    return RT_EOK;
}

static void camera_intf_sccb_write(UINT8 addr, UINT8 data)
{
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[2] = {addr,data};
    
    msgs.addr = i2c_operater.salve_id;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;

    if(rt_i2c_transfer(camera_i2c_bus,&msgs,1)==1)
    {

    }
    else
    {
        rt_kprintf("camera transer err %s %d\r\n",__FUNCTION__,__LINE__);
    }    
}


void camera_intf_sccb_read(UINT8 addr, UINT8 *data)
{
    struct rt_i2c_msg msgs[2];
    
    msgs[0].addr = i2c_operater.salve_id;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &addr;
    msgs[0].len = 1;

    msgs[1].addr = i2c_operater.salve_id;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = data;
    msgs[1].len = 1;
    if(rt_i2c_transfer(camera_i2c_bus,msgs,2)==2)
    {
        
    }
    else
    {
        rt_kprintf("camera read transfer err!!!\r\n");
    }
    
}

void camera_intf_delay_timer_hdl(UINT8 param)
{
    #if CFG_GENERAL_DMA
    GDMA_CFG_ST en_cfg;
    UINT16 already_len = ejpeg_cfg.rx_read_len;
    UINT32 channel = ejpeg_cfg.dma_channel;
    GLOBAL_INT_DECLARATION();

    if(ejpeg_hdl == DD_HANDLE_UNVALID)
        return;

    //REG_WRITE((0x00802800+(16*4)), 0x02);

    int left_len = sddev_control(GDMA_DEV_NAME, CMD_GDMA_GET_LEFT_LEN, (void*)channel);
    int rec_len = ejpeg_cfg.node_len - left_len;
    UINT32 frame_len = 0;
    frame_len = ddev_control(ejpeg_hdl, EJPEG_CMD_GET_FRAME_LEN, NULL);

    if((ejpeg_cfg.node_full_handler != NULL) && (rec_len > 0)) {
        ejpeg_cfg.node_full_handler(ejpeg_cfg.rxbuf + already_len, rec_len, 1, frame_len);
    }

    already_len += rec_len;
    if(already_len >= ejpeg_cfg.rxbuf_len) {
        already_len -= ejpeg_cfg.rxbuf_len;
    }

    GLOBAL_INT_DISABLE();
    ejpeg_cfg.rx_read_len = already_len;
    GLOBAL_INT_RESTORE();

    // turn off dma, so dma can start from first configure. for easy handler
    en_cfg.channel = ejpeg_cfg.dma_channel;
    en_cfg.param = 0;   
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);

    ejpeg_cfg.rx_read_len = 0;
    en_cfg.param = 1;   
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
    #endif
    
    if((ejpeg_cfg.data_end_handler)) {
        ejpeg_cfg.data_end_handler();
    }

    channel = EJPEG_DELAY_HTIMER_CHNAL;
    sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_DISABLE, &channel);

    //REG_WRITE((0x00802800+(16*4)), 0x00);
}

static void camera_intf_start_delay_timer(void)
{
	timer_param_t param;

    if(ejpeg_hdl == DD_HANDLE_UNVALID)
        return;
    //REG_WRITE((0x00802800+(16*4)), 0x02);
    
	param.channel = EJPEG_DELAY_HTIMER_CHNAL;
	param.div = 1;
	param.period = EJPEG_DELAY_HTIMER_VAL;
	param.t_Int_Handler= camera_intf_delay_timer_hdl;
        
	sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM, &param);
    
    //REG_WRITE((0x00802800+(16*4)), 0x00);    
}

static void camera_intf_ejpeg_rx_handler(UINT32 dma)
{   
    UINT16 already_len = ejpeg_cfg.rx_read_len;
    UINT16 copy_len = ejpeg_cfg.node_len;
    GLOBAL_INT_DECLARATION();

    if(ejpeg_hdl == DD_HANDLE_UNVALID)
        return;

    //REG_WRITE((0x00802800+(17*4)), 0x02);

    if(ejpeg_cfg.node_full_handler != NULL) {
        ejpeg_cfg.node_full_handler(ejpeg_cfg.rxbuf + already_len, copy_len, 0, 0);
    }

    already_len += copy_len;
    
    if(already_len >= ejpeg_cfg.rxbuf_len)
        already_len = 0;

    GLOBAL_INT_DISABLE();
    ejpeg_cfg.rx_read_len = already_len;
    GLOBAL_INT_RESTORE();

    //REG_WRITE((0x00802800+(17*4)), 0x00);
}

static void camera_intf_ejpeg_end_handler(void)
{      
    camera_intf_start_delay_timer();     
}

static void camera_intf_init_ejpeg_pixel(UINT32 ppi_type)
{
    switch (ppi_type)
    {
        case QVGA_320_240:
            ejpeg_cfg.x_pixel = X_PIXEL_320;
            ejpeg_cfg.y_pixel = Y_PIXEL_240;
            break;

        case VGA_640_480:
            ejpeg_cfg.x_pixel = X_PIXEL_640;
            ejpeg_cfg.y_pixel = Y_PIXEL_480;
            break;

        default:
            CAMERA_INTF_WPRT("cm PPI unknown, use QVGA\r\n");
            ejpeg_cfg.x_pixel = X_PIXEL_640;
            ejpeg_cfg.y_pixel = Y_PIXEL_480;
            break;
    }
}

static void camera_intf_config_ejpeg(void* data)
{   
    os_memset(&ejpeg_cfg, 0, sizeof(DJPEG_DESC_ST));
    os_memcpy(&ejpeg_cfg, data, sizeof(TVIDEO_DESC_ST));

    camera_intf_init_ejpeg_pixel(CMPARAM_GET_PPI(ejpeg_cfg.sener_cfg));

    ejpeg_cfg.start_frame_handler = NULL;
    ejpeg_cfg.end_frame_handler = camera_intf_ejpeg_end_handler;

#if CFG_GENERAL_DMA
    ejpeg_cfg.dma_rx_handler = camera_intf_ejpeg_rx_handler;
    ejpeg_cfg.dma_channel = GDMA_CHANNEL_4;
#endif
}




#if (USE_CAMERA == GC0328C_DEV) 
static void camera_inf_cfg_gc0328c_ppi(UINT32 ppi_type)
{
    UINT32 i, size;
    UINT8 addr, data;
    
    switch (ppi_type)
    {
        case QVGA_320_240:
            size = sizeof(gc0328c_QVGA_320_240_talbe) / 2;
            for(i=0; i<size; i++) 
            {
                addr = gc0328c_QVGA_320_240_talbe[i][0];
                data = gc0328c_QVGA_320_240_talbe[i][1];
                camera_intf_sccb_write(addr, data);
            }
            break;

        case VGA_640_480:
            size = sizeof(gc0328c_VGA_640_480_talbe) / 2;
            for(i=0; i<size; i++) 
            {
                addr = gc0328c_VGA_640_480_talbe[i][0];
                data = gc0328c_VGA_640_480_talbe[i][1];
                camera_intf_sccb_write(addr, data);
            }
            break;

        default:
            CAMERA_INTF_WPRT("set PPI unknown\r\n");
            break;
    }
}

static void camera_inf_cfg_gc0328c_fps(UINT32 fps_type)
{
    UINT32 i, size;
    UINT8 addr, data;

    switch (fps_type)
    {
        case TYPE_5FPS:
            size = sizeof(gc0328c_5pfs_talbe) / 2;
            for(i=0; i<size; i++) 
            {
                addr = gc0328c_5pfs_talbe[i][0];
                data = gc0328c_5pfs_talbe[i][1];
                camera_intf_sccb_write(addr, data);
            }
            break;

        case TYPE_10FPS:
            size = sizeof(gc0328c_10pfs_talbe) / 2;
            for(i=0; i<size; i++) 
            {
                addr = gc0328c_10pfs_talbe[i][0];
                data = gc0328c_10pfs_talbe[i][1];
                camera_intf_sccb_write(addr, data);
            }
            break;
            
        case TYPE_20FPS:
            size = sizeof(gc0328c_20pfs_talbe) / 2;
            for(i=0; i<size; i++) 
            {
                addr = gc0328c_20pfs_talbe[i][0];
                data = gc0328c_20pfs_talbe[i][1];
                camera_intf_sccb_write(addr, data);
            }
            break;

        default:
            CAMERA_INTF_WPRT("set FPS unknown\r\n");
            break;
    }
}
#endif

static void camera_intf_config_senser(void)
{
    UINT32 i, size;
    UINT8 addr, data;
    
#if (USE_CAMERA == PAS6329_DEV)

    i2c_operater.salve_id = PAS6329_DEV_ID;

    size = sizeof(pas6329_page0)/2;
    PAS6329_SET_PAGE0;
    
    for(i=0; i<size; i++) {
        addr = pas6329_page0[i][0];
        data = pas6329_page0[i][1];
        camera_intf_sccb_write(addr, data);
    }

    size = sizeof(pas6329_page1)/2;
    PAS6329_SET_PAGE1;
    for(i=0; i<size; i++) {
        addr = pas6329_page1[i][0];
        data = pas6329_page1[i][1];
        camera_intf_sccb_write(addr, data);
    }

    size = sizeof(pas6329_page2)/2;
    PAS6329_SET_PAGE2;
    for(i=0; i<size; i++) {
        addr = pas6329_page2[i][0];
        data = pas6329_page2[i][1];
        camera_intf_sccb_write(addr, data);
    }

    PAS6329_SET_PAGE0;
    CAMERA_INTF_WPRT("PAS6329 init finish\r\n");
    
#elif (USE_CAMERA == OV_7670_DEV)

    i2c_operater.salve_id = OV_7670_DEV_ID;

    size = sizeof(ov_7670_init_talbe)/2;
    
    for(i=0; i<size; i++) {
        addr = ov_7670_init_talbe[i][0];
        data = ov_7670_init_talbe[i][1];
        camera_intf_sccb_write(addr, data);
    }
    CAMERA_INTF_WPRT("OV_7670 init finish\r\n");
    
#elif (USE_CAMERA == PAS6375_DEV)

    i2c_operater.salve_id = PAS6375_DEV_ID;

    size = sizeof(pas6375_init_talbe)/2;
    
    for(i=0; i<size; i++) {
        addr = pas6375_init_talbe[i][0];
        data = pas6375_init_talbe[i][1];
        camera_intf_sccb_write(addr, data);
    }
    CAMERA_INTF_WPRT("PAS6375 init finish\r\n");

#elif (USE_CAMERA == GC0328C_DEV) 
    i2c_operater.salve_id = GC0328C_DEV_ID;

    size = sizeof(gc0328c_init_talbe)/2;
    
    for(i=0; i<size; i++) {
        addr = gc0328c_init_talbe[i][0];
        data = gc0328c_init_talbe[i][1];
        camera_intf_sccb_write(addr, data);
    }
    camera_intf_sccb_write(0x17, 0x17);  //设置摄像头翻转180 
    camera_inf_cfg_gc0328c_ppi(CMPARAM_GET_PPI(ejpeg_cfg.sener_cfg));
    camera_inf_cfg_gc0328c_fps(CMPARAM_GET_FPS(ejpeg_cfg.sener_cfg));

    CAMERA_INTF_WPRT("GC0328C init finish\r\n");
#elif (USE_CAMERA == BF_2013_DEV)
    i2c_operater.salve_id = BF_2013_DEV_ID;

    size = sizeof(bf_2013_init_talbe)/2;
   
    for(i=0; i<size; i++) {
        addr = bf_2013_init_talbe[i][0];
        data = bf_2013_init_talbe[i][1];
        camera_intf_sccb_write(addr, data);
    }
    CAMERA_INTF_WPRT("BF_2013 init finish\r\n");
	
#elif (USE_CAMERA == GC0308C_DEV) 
    i2c_operater.salve_id = GC0308C_DEV_ID;

    size = sizeof(gc0308c_init_talbe)/2;
    
    for(i=0; i<size; i++) {
        addr = gc0308c_init_talbe[i][0];
        data = gc0308c_init_talbe[i][1];
        camera_intf_sccb_write(addr, data);
    }
    CAMERA_INTF_WPRT("GC0308C init finish\r\n");
#endif
    
}
void init_camera_resetpin(void)
{
    bk_gpio_config_output(CAMERA_RESET_GPIO_INDEX);  
    bk_gpio_output(CAMERA_RESET_GPIO_INDEX, CAMERA_RESET_HIGH_VAL);
}

void camera_reset(void)
{
    bk_gpio_output(CAMERA_RESET_GPIO_INDEX, CAMERA_RESET_HIGH_VAL);
    delay100us(10);													// 1ms
    bk_gpio_output(CAMERA_RESET_GPIO_INDEX, CAMERA_RESET_LOW_VAL);
    delay100us(10);													// 1=1ms,
    bk_gpio_output(CAMERA_RESET_GPIO_INDEX, CAMERA_RESET_HIGH_VAL);
    delay100us(10);													// 1ms
    CAMERA_INTF_WPRT("Camera Reset\r\n");
}

void camera_flip(UINT8 n)
{    
    UINT8 addr, data, dt0, dt1;
    
    if(i2c_operater.salve_id == GC0328C_DEV_ID)
    {
        addr = 0x17;	
        dt0 = 0x14;
        dt1 = 0x17;	
    }
    else
    {
        addr = 0x17;	
        dt0 = 0x14;
        dt1 = 0x17;	
    }
    
    if(n)
    {
        data = dt1;		//flip 180
    }
    else
    {
        data = dt0;		//normal		
    }
    
    camera_intf_sccb_write(addr, data);	
}

/*---------------------------------------------------------------------------*/
void camera_intfer_init(void* data)
{
    UINT32 status, oflag;
    int ret = -1;
    // add 
    ret = camera_hw_init();
    if(ret == 0)
    {
        //open camera ok.
        rt_kprintf("open camera ok \r\n");
    }
    else
    {
        rt_kprintf("open camera error \r\n");
    }

    camera_intf_config_ejpeg(data);

    ejpeg_hdl = ddev_open(EJPEG_DEV_NAME, &status, (UINT32)&ejpeg_cfg);
    camera_reset();
    
    oflag = I2C_DEF_DIV;

    camera_intf_config_senser();
    
    CAMERA_INTF_FATAL("camera_intfer_init,%p\r\n", ejpeg_hdl);
    
}

void camera_intfer_deinit(void)
{
    GLOBAL_INT_DECLARATION();
    CAMERA_INTF_FATAL("camera_intfer_deinit,%p\r\n", ejpeg_hdl);
    
    ddev_close(ejpeg_hdl);
    //ddev_close(i2c_hdl);

    GLOBAL_INT_DISABLE();
    ejpeg_hdl = DD_HANDLE_UNVALID;
    GLOBAL_INT_RESTORE();

    os_memset(&ejpeg_cfg, 0, sizeof(DJPEG_DESC_ST));
}

void camera_intfer_set_video_param(UINT32 ppi_type, UINT32 pfs_type)
{
    #if (USE_CAMERA == GC0328C_DEV)
    if(ejpeg_hdl == DD_HANDLE_UNVALID)
        return;

    if(ppi_type < PPI_MAX)
    {
        UINT32 param;
        camera_intf_init_ejpeg_pixel(ppi_type);

        param = ejpeg_cfg.x_pixel;
        ddev_control(ejpeg_hdl, EJPEG_CMD_SET_X_PIXEL, &param);

        param = ejpeg_cfg.y_pixel;
        ddev_control(ejpeg_hdl, EJPEG_CMD_SET_Y_PIXEL, &param);
        
        camera_inf_cfg_gc0328c_ppi(ppi_type);
    }
    
    if(pfs_type < FPS_MAX) 
    {
        camera_inf_cfg_gc0328c_fps(pfs_type);
    }
    #endif
}
/*---------------------------------------------------------------------------*/

#endif // CFG_USE_CAMERA_INTF
