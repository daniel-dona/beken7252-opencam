#include "error.h"
#include "include.h"
#include "arm_arch.h"
#include "gpio_pub.h"
#include "uart_pub.h"

#include "bk7011_cal_pub.h"
#include "co_list.h"
#include "saradc_pub.h"
#include "temp_detect_pub.h"
#include "sys_rtos.h"
#include "bk_rtos_pub.h"
#include "sys_ctrl_pub.h"
#include "saradc_intf.h"

#define ADC_TAG                      "saradc_intf"
#define ADC_TASK_PRIORITY            (18)
#define ADC_TASK_STACK_SIZE          (512)

typedef struct _tadc_entity_
{
    struct rt_thread tadc_thread;
    char tadc_stack[ADC_TASK_STACK_SIZE];
    rt_sem_t sema_wait_end;
    ADC_OBJ *adc_obj_list;
    beken_mutex_t obj_list_mutex;

    DD_HANDLE adc_handle;
    saradc_desc_t adc_cfg;
    UINT16 adc_data[ADC_TEMP_BUFFER_SIZE];
} TADC_ENTITY_T;

static TADC_ENTITY_T *tadc_entity = NULL;

void adc_obj_init(ADC_OBJ* handle, adc_obj_callback cb, UINT32 channel, void *user_data)
{
	memset(handle, 0, sizeof(ADC_OBJ));
	
	handle->channel = channel;
	handle->user_data = user_data;
	handle->cb = cb;
}

int adc_obj_start(ADC_OBJ* handle)
{
	ADC_OBJ* target;
    
    if(!tadc_entity || (!tadc_entity->obj_list_mutex)) 
	{
        TADC_WARNING_PRINTF("adc obj start with no initial\r\n");
        return -1;
    }
	bk_rtos_lock_mutex(&tadc_entity->obj_list_mutex);
	target = tadc_entity->adc_obj_list;
	while(target) 
	{
		if(target == handle) 
		{
		    bk_rtos_unlock_mutex(&tadc_entity->obj_list_mutex);
		    return -1;	//already exist.
		}
		target = target->next;
	}
	handle->next = tadc_entity->adc_obj_list;
	tadc_entity->adc_obj_list = handle;
	bk_rtos_unlock_mutex(&tadc_entity->obj_list_mutex);
	return 0;
}

void adc_obj_stop(ADC_OBJ* handle)
{
	ADC_OBJ** curr;
	
	bk_rtos_lock_mutex(&tadc_entity->obj_list_mutex);
	for(curr = &tadc_entity->adc_obj_list; *curr; ) 
	{
		ADC_OBJ* entry = *curr;
		if (entry == handle) 
		{
			*curr = entry->next;
		} 
		else
		{
			curr = &entry->next;
		}
	}
	bk_rtos_unlock_mutex(&tadc_entity->obj_list_mutex);
}

static void sadc_detect_handler(void)
{
    saradc_desc_t *p_ADC_drv_desc = &tadc_entity->adc_cfg;
    if(!p_ADC_drv_desc)
        return;
    
    if(p_ADC_drv_desc->current_sample_data_cnt >= p_ADC_drv_desc->data_buff_size) 
    {
        UINT32 sum = 0, sum1, sum2;

        sum1 = p_ADC_drv_desc->pData[1] + p_ADC_drv_desc->pData[2];
        sum2 = p_ADC_drv_desc->pData[3] + p_ADC_drv_desc->pData[4];
        sum = sum1 + sum2;        
        sum = sum >> 2;
        p_ADC_drv_desc->pData[0] = sum;
        
        rt_sem_release(tadc_entity->sema_wait_end);
    }
}

void tadc_obj_handler(ADC_OBJ* handle)
{
    saradc_desc_t *p_ADC_drv_desc = NULL;
    UINT32 status, ret;

    p_ADC_drv_desc = &tadc_entity->adc_cfg;
    
    p_ADC_drv_desc->channel = handle->channel;
    p_ADC_drv_desc->filter                  = 0;
    p_ADC_drv_desc->mode                    = (ADC_CONFIG_MODE_CONTINUE << 0)
                                              |(ADC_CONFIG_MODE_4CLK_DELAY << 2);
    p_ADC_drv_desc->pre_div                 = 0x10;
    p_ADC_drv_desc->samp_rate               = 0x20;

    p_ADC_drv_desc->data_buff_size          = ADC_TEMP_BUFFER_SIZE;
    p_ADC_drv_desc->current_read_data_cnt   = 0;
    p_ADC_drv_desc->current_sample_data_cnt = 0;
    p_ADC_drv_desc->has_data                = 0;
    
    p_ADC_drv_desc->pData                   = &tadc_entity->adc_data[0];
    rt_memset(p_ADC_drv_desc->pData, 0x00, p_ADC_drv_desc->data_buff_size * sizeof(UINT16));


    p_ADC_drv_desc->p_Int_Handler           = sadc_detect_handler;
    tadc_entity->adc_handle = ddev_open(SARADC_DEV_NAME, &status, (UINT32)p_ADC_drv_desc);

    status = ADC_SCAN_INTERVALV; 
    ret = rt_sem_take(tadc_entity->sema_wait_end, rt_tick_from_millisecond(status));

    status = ddev_close(tadc_entity->adc_handle);
    if(DRV_FAILURE == status )
    {
        TADC_FATAL_PRINTF("tadc close failed %d\r\n", status);
    }
    tadc_entity->adc_handle = DD_HANDLE_UNVALID;

    if(ret == kNoErr) 
	{
        float voltage = 0.0;
        int mv;

        voltage = saradc_calculate(p_ADC_drv_desc->pData[0]);
        mv = voltage * 1000;

        if (handle->cb)
		{
            handle->cb(mv, handle->user_data);
        }
    }
	else 
    {
        TADC_WARNING_PRINTF("sema_wait_end timeout:%d\r\n", status);
    }
    
}

void tadc_entity_deinit(TADC_ENTITY_T **adc_entity)
{
    TADC_WARNING_PRINTF("tadc_entity_deinit\r\n");
    if(*adc_entity)
    {
        if((*adc_entity)->sema_wait_end)
        {
            rt_sem_delete((*adc_entity)->sema_wait_end);
            (*adc_entity)->sema_wait_end = NULL;
        }
        
        if((*adc_entity)->obj_list_mutex)
        {
            rt_mutex_delete((*adc_entity)->obj_list_mutex);
            (*adc_entity)->obj_list_mutex = NULL;
        }
        
        free((*adc_entity));
        *adc_entity = NULL;
    }
}

TADC_ENTITY_T *tadc_entity_init(void)
{
    TADC_ENTITY_T *adc_entity = NULL;
    adc_entity = (TADC_ENTITY_T *) malloc(sizeof(TADC_ENTITY_T));
    if (adc_entity == NULL)
    {
        TADC_FATAL_PRINTF("malloc TADC_ENTITY_T failed\r\n");
        return  NULL;
    }

    memset(adc_entity, 0, sizeof(TADC_ENTITY_T));

    adc_entity->obj_list_mutex = rt_mutex_create("tadc_obj_mutex", RT_IPC_FLAG_FIFO);
    if(adc_entity->obj_list_mutex == NULL)
    {
        TADC_FATAL_PRINTF("tc_entity->mutex error\r\n");
        tadc_entity_deinit(&adc_entity);
        return NULL;
    }

    adc_entity->sema_wait_end = rt_sem_create("tadc_sema", 0,  RT_IPC_FLAG_FIFO);
    if(adc_entity->sema_wait_end == NULL)
    {
        TADC_FATAL_PRINTF("tc_entity->sema_wait_end error\r\n");
        tadc_entity_deinit(&adc_entity);
        return NULL;
    }

    return adc_entity;
}
/*
vol:	PSRAM_VDD_1_8V
		PSRAM_VDD_2_5V
		PSRAM_VDD_3_3V
*/
void saradc_config_vddram_voltage(UINT32 vol)
{
    UINT32 param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);

    param = vol;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_VDDRAM_VOLTAGE, &param);
}

void saradc_disable_vddram_voltage(void)
{
    UINT32 param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_DISABLE, &param);

}

static void tadc_thread_entry(void *pv)
{
    ADC_OBJ* target;
	unsigned int scan_tick = (unsigned int)pv;
	scan_tick = rt_tick_from_millisecond(scan_tick);
    while(1)
    {
		rt_thread_delay(scan_tick);
		bk_rtos_lock_mutex(&tadc_entity->obj_list_mutex);
		for(target=tadc_entity->adc_obj_list; target; target=target->next)
		{
			tadc_obj_handler(target);
		}
		bk_rtos_unlock_mutex(&tadc_entity->obj_list_mutex);
    }

tadc_exit:
    tadc_entity_deinit(&tadc_entity);
    tadc_entity = NULL;
    bk_rtos_delete_thread(NULL);
}

void saradc_work_create(UINT32 scan_interval_ms)
{
	rt_err_t result;
	if(tadc_entity)
	{
		return;
	}

	tadc_entity = tadc_entity_init();
	if(NULL == tadc_entity)
	{
		return;
	}

	result = rt_thread_init(&tadc_entity->tadc_thread, ADC_TAG,
							tadc_thread_entry, (void*)scan_interval_ms,
							&tadc_entity->tadc_stack[0],
							sizeof(tadc_entity->tadc_stack),
							ADC_TASK_PRIORITY, 10);
	if(RT_EOK != result)
	{
		goto create_exit;
	}

    result = rt_thread_startup(&tadc_entity->tadc_thread);
    if(RT_EOK != result)
    {
		goto create_exit;
	}

    return;

create_exit:
    
    tadc_entity_deinit(&tadc_entity);
}

#if CFG_SARADC_CALIBRATE
static void adc_check(int argc, char **argv)
{
    UINT32 status;
    DD_HANDLE flash_handle;
    DD_HANDLE saradc_handle;
    saradc_cal_val_t p_ADC_cal;
	UINT32 sum = 0, sum1, sum2;
    UINT16 *pData;
    float voltage = 0.0;
    saradc_desc_t *p_ADC_drv_desc = NULL;
        
    if(argc < 2)
        goto IDLE_CMD_ERR;

    if(0 == strcmp(argv[1], "read"))
    {
        status = manual_cal_load_adc_cali_flash();
        if(status != 0)
        {
            os_printf("Can't read cali value, use default!\r\n");
            os_printf("calibrate low value:[%x]\r\n", saradc_val.low);
            os_printf("calibrate high value:[%x]\r\n", saradc_val.high);
        }
    }
    else if(0 == strcmp(argv[1], "set"))
    {
        p_ADC_drv_desc = (saradc_desc_t *)malloc(sizeof(saradc_desc_t));
        if (p_ADC_drv_desc == NULL)
        {
            os_printf("malloc1 failed!\r\n");
            return;
        }

        rt_memset(p_ADC_drv_desc, 0x00, sizeof(saradc_desc_t));
        p_ADC_drv_desc->channel = 1;
        p_ADC_drv_desc->data_buff_size = ADC_TEMP_BUFFER_SIZE;
        p_ADC_drv_desc->mode = (ADC_CONFIG_MODE_CONTINUE << 0)
                              |(ADC_CONFIG_MODE_4CLK_DELAY << 2);
                               
        p_ADC_drv_desc->has_data                = 0;
        p_ADC_drv_desc->current_read_data_cnt   = 0;
        p_ADC_drv_desc->current_sample_data_cnt = 0;
        p_ADC_drv_desc->pre_div = 0x10;
        p_ADC_drv_desc->samp_rate = 0x20;
        p_ADC_drv_desc->pData = (UINT16 *)malloc(p_ADC_drv_desc->data_buff_size * sizeof(UINT16));
        rt_memset(p_ADC_drv_desc->pData, 0x00, p_ADC_drv_desc->data_buff_size * sizeof(UINT16));

        if(p_ADC_drv_desc->pData == NULL)
        {
            os_printf("malloc1 failed!\r\n");
            free(p_ADC_drv_desc);
            return;
        }

        saradc_handle = ddev_open(SARADC_DEV_NAME, &status, (UINT32)p_ADC_drv_desc);

        while (1)
        {
            if (p_ADC_drv_desc->current_sample_data_cnt == p_ADC_drv_desc->data_buff_size)
            {
                ddev_close(saradc_handle);
                break;
            }
        }
        

		pData = p_ADC_drv_desc->pData;
        sum1 = pData[1] + pData[2];
        sum2 = pData[3] + pData[4];
        sum = sum1  + sum2 ;        
        sum = sum / 4;
        p_ADC_drv_desc->pData[0] = sum;
     

        if(0 == strcmp(argv[2], "low"))
        {
            p_ADC_cal.mode = SARADC_CALIBRATE_LOW;
        }
        else if(0 == strcmp(argv[2], "high"))
        {
            p_ADC_cal.mode = SARADC_CALIBRATE_HIGH;
        }
        else
        {
            os_printf("invalid parameter\r\n");
            return;
        }
        p_ADC_cal.val = p_ADC_drv_desc->pData[0];
        if(SARADC_FAILURE == ddev_control(saradc_handle, SARADC_CMD_SET_CAL_VAL, (VOID *)&p_ADC_cal))
        {
            os_printf("set calibrate value failture\r\n");
            free(p_ADC_drv_desc->pData);
            free(p_ADC_drv_desc);
            return;
        }
        os_printf("set calibrate success\r\n");
        os_printf("type:[%s] value:[0x%x]\r\n", (p_ADC_cal.mode ? "high":"low"), p_ADC_cal.val);
        free(p_ADC_drv_desc->pData);
        free(p_ADC_drv_desc);
    }
    else if(0 == strcmp(argv[1], "write"))
    {
        manual_cal_save_chipinfo_tab_to_flash();
        os_printf("calibrate low value:[%x]\r\n", saradc_val.low);
        os_printf("calibrate high value:[%x]\r\n", saradc_val.high);
    }
    
    else if(0 == strcmp(argv[1], "get"))
    {
        p_ADC_drv_desc = (saradc_desc_t *)malloc(sizeof(saradc_desc_t));
        if (p_ADC_drv_desc == NULL)
        {
        	os_printf("malloc1 failed!\r\n");
            return;
        }

        rt_memset(p_ADC_drv_desc, 0x00, sizeof(saradc_desc_t));
        p_ADC_drv_desc->channel = 1;
        p_ADC_drv_desc->data_buff_size = ADC_TEMP_BUFFER_SIZE;
        p_ADC_drv_desc->mode = (ADC_CONFIG_MODE_CONTINUE << 0)
                              |(ADC_CONFIG_MODE_4CLK_DELAY << 2);
                               
        p_ADC_drv_desc->has_data                = 0;
        p_ADC_drv_desc->current_read_data_cnt   = 0;
        p_ADC_drv_desc->current_sample_data_cnt = 0;
        p_ADC_drv_desc->pre_div = 0x10;
        p_ADC_drv_desc->samp_rate = 0x20;
        p_ADC_drv_desc->pData = (UINT16 *)malloc(p_ADC_drv_desc->data_buff_size * sizeof(UINT16));
        rt_memset(p_ADC_drv_desc->pData, 0x00, p_ADC_drv_desc->data_buff_size * sizeof(UINT16));

        if(p_ADC_drv_desc->pData == NULL)
        {
            os_printf("malloc1 failed!\r\n");
            free(p_ADC_drv_desc);
            return;
        }

        saradc_handle = ddev_open(SARADC_DEV_NAME, &status, (UINT32)p_ADC_drv_desc);

        while (1)
        {
            if (p_ADC_drv_desc->current_sample_data_cnt == p_ADC_drv_desc->data_buff_size)
            {
                ddev_close(saradc_handle);
                break;
            }
        }

		pData = p_ADC_drv_desc->pData;
        sum1 = pData[1] + pData[2];
        sum2 = pData[3] + pData[4];
        sum = sum1 + sum2;        
        sum = sum / 4;
        p_ADC_drv_desc->pData[0] = sum;    
        
        voltage = saradc_calculate(p_ADC_drv_desc->pData[0]);
        
        os_printf("voltage is [%d] mv\r\n", (UINT32)(voltage * 1000));
        free(p_ADC_drv_desc->pData);
        free(p_ADC_drv_desc);
        return;
        
    }
    else
    {
        goto IDLE_CMD_ERR;
    }
    
    return;
IDLE_CMD_ERR:
    os_printf("Usage:ps [func] [param]\r\n");
}

MSH_CMD_EXPORT(adc_check, adc check);
#endif // CFG_SARADC_CALIBRATE

