/*
利用 IO 模拟 SPI 注册到总线
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>

#include "typedef.h"
#include "spi_pub.h"

/* 增加 I2S 相关的头文件 */
#include "arm_arch.h"
#include "sys_ctrl_pub.h"
#include "drv_model_pub.h"
#include "mem_pub.h"
#include "sys_config.h"
#include "error.h"
#include "bk_rtos_pub.h"
#include "typedef.h"
#include "icu_pub.h"
#include "i2s.h"
#include "i2s_pub.h"
#include "gpio_pub.h"
#include "gpio.h"
//定义模拟 SPI 的 GPIO 引脚
#define SOFT_SPI_MISO   (4)
#define SOFT_SPI_MOSI   (5)
#define SOFT_SPI_CS     (3)
#define SOFT_SPI_SCLK   (2)

// 引用外部 i2s_ctrl 函数声明
extern UINT32 i2s_ctrl(UINT32 cmd, void *param);

struct soft_spi_dev
{
    struct rt_spi_bus *spi_bus;
};

static struct soft_spi_dev *spi_dev;

/* 设置 SPI 端口初始化 */
static void soft_spi_init()
{
    //设置为输出模式
    gpio_config(SOFT_SPI_CS, GMODE_OUTPUT);
    gpio_config(SOFT_SPI_SCLK, GMODE_OUTPUT);
    gpio_config(SOFT_SPI_MOSI, GMODE_OUTPUT);
    gpio_config(SOFT_SPI_MISO,GMODE_INPUT);

    //设置 CPOL = 0
    gpio_output(SOFT_SPI_SCLK,0);
    gpio_output(SOFT_SPI_MOSI,0);
    gpio_output(SOFT_SPI_CS,1);
}

/*设置从设备使能 */
static void soft_spi_cs_enable(int enable)
{
    UINT32 val;
    volatile UINT32 *gpio_cfg_addr_cs;
    gpio_cfg_addr_cs = (volatile UINT32 *)(REG_GPIO_CFG_BASE_ADDR + SOFT_SPI_CS * 4);
    val = 0x00;   //设置为输出
    REG_WRITE(gpio_cfg_addr_cs, val);
    //设置为输出模式
    
    val = REG_READ(gpio_cfg_addr_cs);

    if (enable)
    {
        //gpio_output(SOFT_SPI_CS,0);
        val &= ~GCFG_OUTPUT_BIT;
        val |= (0 & 0x01) << GCFG_OUTPUT_POS;
        REG_WRITE(gpio_cfg_addr_cs, val);
    }
    else
    {
        //gpio_output(SOFT_SPI_CS,1);
        val &= ~GCFG_OUTPUT_BIT;
        val |= (1 & 0x01) << GCFG_OUTPUT_POS;
        REG_WRITE(gpio_cfg_addr_cs, val);
    }
}

static uint8 soft_spi_ReadWriteByte(uint8 byte)
{
    uint8 rdata = 0;
    uint8 i = 0;
    UINT32 reg_val,reg_val2,reg_val3;
    volatile UINT32 *gpio_cfg_addr_sck,*gpio_cfg_addr_mosi,*gpio_cfg_addr_miso;
    gpio_cfg_addr_sck = (volatile UINT32 *)(REG_GPIO_CFG_BASE_ADDR + SOFT_SPI_SCLK * 4);
    gpio_cfg_addr_mosi = (volatile UINT32 *)(REG_GPIO_CFG_BASE_ADDR + SOFT_SPI_MOSI * 4);
    gpio_cfg_addr_miso =  (volatile UINT32 *)(REG_GPIO_CFG_BASE_ADDR + SOFT_SPI_MISO * 4);
    reg_val = REG_READ(gpio_cfg_addr_sck);
    reg_val2 = REG_READ(gpio_cfg_addr_mosi);
    
    
    for(i=8;i>0;i--)
    {   
        // 拉低 SCK 进入发送数据状态
        reg_val &= ~GCFG_OUTPUT_BIT;
        reg_val |= (0) << GCFG_OUTPUT_POS;
        REG_WRITE(gpio_cfg_addr_sck, reg_val);
        if(byte&0x80)
        {
            //设置 MOSI 输出1
            reg_val2 &= ~GCFG_OUTPUT_BIT;
            reg_val2 |= (0x01) << GCFG_OUTPUT_POS;
            REG_WRITE(gpio_cfg_addr_mosi, reg_val2);
        }
        else
        {
             //设置 MOSI 输出0
            reg_val2 &= ~GCFG_OUTPUT_BIT;
            reg_val2 |= (0) << GCFG_OUTPUT_POS;
            REG_WRITE(gpio_cfg_addr_mosi, reg_val2);
        }
        byte<<=1;
        rdata = rdata<<1;
       
        if(REG_READ(gpio_cfg_addr_miso) & GCFG_INPUT_BIT )  //读取数据
        {
            rdata |= 0x01;
        }
        // 发送完数据 拉高 SCK 
        reg_val &= ~GCFG_OUTPUT_BIT;
        reg_val |= (0x01) << GCFG_OUTPUT_POS;
        REG_WRITE(gpio_cfg_addr_sck, reg_val);
    }
    //空闲状态拉低 SCK
    reg_val &= ~GCFG_OUTPUT_BIT;
    reg_val |= (0) << GCFG_OUTPUT_POS;
    REG_WRITE(gpio_cfg_addr_sck, reg_val);
    return rdata;
}


/* 模拟SPI 配置 */
rt_err_t _soft_spi_configure(struct rt_spi_device *dev,struct rt_spi_configuration *cfg)
{
    return RT_EOK;
}
/*  */
rt_uint32_t _soft_spi_xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{

    struct rt_spi_configuration *config = &device->config;
    rt_uint32_t size = message->length;

        /* 设置 CS  */
        if (message->cs_take)
        {
            soft_spi_cs_enable(1);
        }

        const rt_uint8_t *send_ptr = message->send_buf;
        rt_uint8_t *recv_ptr = message->recv_buf;
        while (size--)
        {
            rt_uint8_t data = 0xFF;

            if (send_ptr != RT_NULL)
            {
                data = *send_ptr++;
                //rt_kprintf("send_ptr:%02x\n",data);
                // 发送数据
                soft_spi_ReadWriteByte(data);
            }

            //Wait until the transmit buffer is empty

            if (recv_ptr != RT_NULL)
            {
                // 接收数据
                data = soft_spi_ReadWriteByte(data);
                *recv_ptr++ = data;
                //rt_kprintf("recv_ptr:%02x\n",data);
            }
        }
        /* 设置 release CS   */
        if (message->cs_release)
        {
            soft_spi_cs_enable(0);
        }

    //rt_kprintf("len=%d \n",message->length);
    return message->length;
}

static struct rt_spi_ops soft_spi_ops = 
{
    .configure = _soft_spi_configure,
    .xfer = _soft_spi_xfer
};

int rt_soft_spi_bus_register(char *name)
{
    int result = RT_EOK;
    struct rt_spi_bus *spi_bus = RT_NULL;

    if(spi_dev)
    {
       return RT_EOK; 
    }

    spi_dev = rt_malloc(sizeof(struct soft_spi_dev));
    if(!spi_dev)
    {
        rt_kprintf("[soft spi]:malloc memory for spi_dev failed\n");
        result = -RT_ENOMEM;
        goto _exit;
    }
    memset(spi_dev,0,sizeof(struct soft_spi_dev));

    spi_bus = rt_malloc(sizeof(struct rt_spi_bus));
    if(!spi_bus)
    {
        rt_kprintf("[soft spi]:malloc memory for spi_bus failed\n");
        result = -RT_ENOMEM;
        goto _exit;
    }
    memset(spi_bus,0,sizeof(struct rt_spi_bus));

    spi_bus->parent.user_data = spi_dev;
    rt_spi_bus_register(spi_bus, name, &soft_spi_ops);
    
    return result;



_exit:
    if (spi_dev)
    {
        rt_free(spi_dev);
        spi_dev = RT_NULL;
    }

    if (spi_bus)
    {
        rt_free(spi_bus);
        spi_bus = RT_NULL;
    }
    return result;
}

static struct rt_spi_device *soft_spi_device = RT_NULL;
int rt_soft_spi_device_init(void)
{
    int result = RT_EOK;

    rt_kprintf("[soft spi]:rt_soft_spi_device_init \n");

    if(soft_spi_device)
    {
        return RT_EOK;
    }
    soft_spi_device = rt_malloc(sizeof(struct rt_spi_device));
    if(!soft_spi_device)
    {
        rt_kprintf("[soft spi]:malloc memory for soft spi_device failed\n");
        result = -RT_ENOMEM;
    }
    memset(soft_spi_device,0,sizeof(struct rt_spi_device));

    /* 注册 SPI BUS */
    result = rt_soft_spi_bus_register("soft_spi");
    if(result != RT_EOK)
    {
        rt_kprintf("[soft spi]:register soft spi bus error : %d !!!\n",result);
        goto _exit;
    }

    /* 绑定 CS */
    result = rt_spi_bus_attach_device(soft_spi_device,"spi3","soft_spi",NULL);
    if(result != RT_EOK)
    {
        rt_kprintf("[soft spi]:attact spi bus error :%d !!!\n",result);
        goto _exit;
    }
    rt_kprintf("[soft spi]:rt_soft_spi_device init ok\n");
    return RT_EOK;

_exit:
    if(soft_spi_device)
    {
        rt_free(soft_spi_device);
        soft_spi_device = RT_NULL;
    }
    return result;
}
INIT_PREV_EXPORT(rt_soft_spi_device_init);
INIT_DEVICE_EXPORT(soft_spi_init);
MSH_CMD_EXPORT(soft_spi_init,soft_spi_init);