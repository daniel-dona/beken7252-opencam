#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "include.h"

#include "saradc_pub.h"
#include "drv_model_pub.h"
#include "drv_adc.h"

#ifdef RT_USING_ADC
static saradc_desc_t *adc_param = NULL;
static DD_HANDLE saradc_handle;
static UINT32 status;
#endif

static rt_err_t drv_adc_open(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{   
    return RT_EOK;
}

static rt_err_t drv_adc_read(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    float voltage = 0.0;

    adc_param->channel = channel;
    saradc_handle = ddev_open(SARADC_DEV_NAME, &status, (UINT32)adc_param);

    while (1)
    {
        if (adc_param->current_sample_data_cnt == adc_param->data_buff_size)
        {
            ddev_close(saradc_handle);
            break;
        }
    }
    
    {
        UINT32 sum = 0, sum1, sum2;
        UINT16 *pData = adc_param->pData;
        sum1 = pData[1] + pData[2];
        sum2 += pData[3] + pData[4];
        sum = sum1 / 2 + sum1 / 2;        
        sum = sum / 2;
        adc_param->pData[0] = sum;
    }    
    
    voltage = saradc_calculate(adc_param->pData[0]);
    *value = (UINT32)(voltage * 1000);

    return RT_EOK;
}

const static struct rt_adc_ops drv_ops =
{
    drv_adc_open,
    drv_adc_read
};

#ifdef RT_USING_ADC
void rt_hw_adc_init(void)
{
    adc_param = (saradc_desc_t *)rt_malloc(sizeof(saradc_desc_t));
    
    if(adc_param == RT_NULL) 
    {
        rt_kprintf("malloc adc param failed\n");
        return;
    }

    rt_memset(adc_param, 0x00, sizeof(saradc_desc_t));
    adc_param->channel = 1;
    adc_param->data_buff_size = 5;
    adc_param->mode = (ADC_CONFIG_MODE_CONTINUE << 0)
                            |(ADC_CONFIG_MODE_4CLK_DELAY << 2);
                            
    adc_param->has_data                = 0;
    adc_param->current_read_data_cnt   = 0;
    adc_param->current_sample_data_cnt = 0;
    adc_param->pre_div = 0x10;
    adc_param->samp_rate = 0x20;
    adc_param->pData = (UINT16 *)rt_malloc(adc_param->data_buff_size * sizeof(UINT16));
    rt_memset(adc_param->pData, 0x00, adc_param->data_buff_size * sizeof(UINT16));

    if(adc_param->pData == NULL)
    {
        rt_kprintf("malloc adc param failed!\r\n");
        rt_free(adc_param);
        return;
    }

    return;
}
#endif

int drv_adc_init(void)
{
#ifdef RT_USING_ADC
    rt_hw_adc_init();
    rt_hw_adc_register(rt_calloc(1, sizeof(struct rt_adc_device)), "adc", &drv_ops, RT_NULL);
#endif
}
INIT_DEVICE_EXPORT(drv_adc_init);
