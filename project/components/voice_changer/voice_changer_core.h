#ifndef _VOICE_CHANGER_CORE_H_
#define _VOICE_CHANGER_CORE_H_

#include <rtthread.h>

#if CONFIG_VOICE_CHANGER 


#ifndef M_ZERO
#define M_ZERO 0
#endif

#if 0
#define VC_INT_DECLARATION()          //uint32 interrupts_info;
#define VC_INT_DISABLE()              //VICMR_disable_interrupts(&interrupts_info);
#define VC_INT_RESTORE()              //VICMR_restore_interrupts(interrupts_info);
#else
#define VC_INT_DECLARATION()          rt_base_t level
#define VC_INT_DISABLE()              level = rt_hw_interrupt_disable()
#define VC_INT_RESTORE()              rt_hw_interrupt_enable(level)
#endif

#define VC_MALLOC(size, flag)         rt_malloc((size))
#define VC_FREE                       rt_free
#define VC_MEMSET                     memset
#define VC_MEMCPY                     memcpy
#define VC_PRINTF                     rt_kprintf

#define VC_CALC_TIME_MS               (5)
#define VC_MAX_SIMPLE_RATE            (44100)
#define VC_LINEAR_NUM                 (4)

#define VC_MAX_OUT_SIZE               (VC_CALC_TIME_MS * VC_MAX_SIMPLE_RATE / 1000)
#define VC_MAX_LINEAR_SIZE            (VC_MAX_OUT_SIZE * VC_LINEAR_NUM)

#define VC_PARAM_UP_STEP(len)         (len/2)
#define VC_PARAM_UP_NUM               (2)
#define VC_PARAM_DOWN_STEP(len)       (len/4)
#define VC_PARAM_DOWN                 (2)

#define VC_CALC_MAX_SIMPLE_SIZE(len)  (len + VC_PARAM_UP_STEP(len)*VC_PARAM_UP_NUM)
#define VC_MAX_SIMPLE_SIZE            VC_CALC_MAX_SIMPLE_SIZE(VC_MAX_LINEAR_SIZE)

#define VC_HIGH_SAMPLE_STEP           (8)
#define VC_LOW_SAMPLE_STEP            (1)

#define VC_MAX_INT16                  (32767)   // -32768  --- 32767
#define VC_MIN_INT16                  (-32768)

typedef enum {
    VC_STOP,
    VC_FIRST,
    VC_START,         
} VC_STA;

typedef struct voice_changer_st {
    int32_t *fifo;  // extal 1 sample is used for xin0
    int16_t *linear;
    int16_t *last;
    uint32_t fifo_size;    
    uint32_t linear_size;
    uint32_t last_size;
    int32_t in_len;
    int32_t out_len;       
    uint32_t audio_freq;
    uint32_t sample_t[VC_PARAM_DOWN +1+VC_PARAM_UP_NUM];
    uint32_t sample_len;
    uint32_t sample_idx;
    uint32_t change_flag;
    VC_STA status;    
} VCHANGER_ST, *VCHANGER_PTR;


void voice_changer_linear(VCHANGER_PTR p_vc) ;
int voice_changer_min_index(VCHANGER_PTR p_vc) ;
int voice_changer_transit(VCHANGER_PTR p_vc,int indx);
void voice_changer_init_sample_len(VCHANGER_PTR p_vc);
void voice_changer_init_first_sample(VCHANGER_PTR p_vc);
void voice_changer_set_sample_len(VCHANGER_PTR p_vc,uint32_t value);


#endif  //#if CONFIG_VOICE_CHANGER 
#endif
