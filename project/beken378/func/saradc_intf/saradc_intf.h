#ifndef __SARADC_INTF_H__
#define __SARADC_INTF_H__

#define TADC_DEBUG_PRTF                     0

#define TADC_FATAL_PRINTF                   rt_kprintf
#define TADC_WARNING_PRINTF                 rt_kprintf

#if TADC_DEBUG_PRTF
#define TADC_PRINTF                         rt_kprintf
#else
#define TADC_PRINTF(...)
#endif //TADC_DEBUG_PRTF

#define ADC_SCAN_INTERVALV           (20)  // ms

typedef void (*adc_obj_callback)(int new_mv, void *user_data);

typedef struct _adc_obj_ {
	void *user_data;
    UINT32 channel;
	adc_obj_callback cb;
	struct _adc_obj_ *next;
}ADC_OBJ;

void adc_obj_init(ADC_OBJ* handle, adc_obj_callback cb, UINT32 channel, void *user_data);
int adc_obj_start(ADC_OBJ* handle);
void adc_obj_stop(ADC_OBJ* handle);

void saradc_config_vddram_voltage(UINT32 vol);
void saradc_work_create(UINT32 scan_interval_ms);
void saradc_disable_vddram_voltage();

#endif
