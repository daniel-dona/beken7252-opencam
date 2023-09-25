#include <rtthread.h>


#if CONFIG_VOICE_CHANGER 
#include "app_voice_changer.h"
#include "voice_changer_core.h"
#include <string.h>
#include "stdlib.h"
#include "stdio.h"



static VCHANGER_PTR p_vchanger;

VC_ERR voice_changer_initial(uint32_t freq)
{
    uint32_t len;

    if(p_vchanger) {
        voice_changer_exit();
    }

    p_vchanger = VC_MALLOC(sizeof(VCHANGER_ST), M_ZERO);    
    if(!p_vchanger) {
        VC_PRINTF("no p_vchanger VC_NOMEMORY\r\n");
        goto init_fail;
    }
    VC_MEMSET(p_vchanger, 0, sizeof(VCHANGER_ST));

    if(freq == 44100)
        len = 256;
    else
        len = (VC_CALC_TIME_MS * freq / 1000);    
    p_vchanger->last = VC_MALLOC(len * sizeof(int16_t), M_ZERO);
    p_vchanger->last_size = len;
    VC_PRINTF("last_size:%d\r\n", p_vchanger->last_size);
    if(!p_vchanger->last) {
        VC_PRINTF("no last VC_NOMEMORY\r\n");
        goto init_fail;
    }

    len = len * VC_LINEAR_NUM;    
    p_vchanger->linear = VC_MALLOC(len * sizeof(int16_t), M_ZERO);
    p_vchanger->linear_size = len;
    VC_PRINTF("linear_size:%d\r\n", p_vchanger->linear_size);
    if(!p_vchanger->linear) {
        VC_PRINTF("no linear VC_NOMEMORY\r\n");
        goto init_fail;
    }

    // extal 1 sample is used for xin0
    len = VC_CALC_MAX_SIMPLE_SIZE(len) + 1;          
    p_vchanger->fifo = VC_MALLOC(len * sizeof(int32_t), M_ZERO);
    p_vchanger->fifo_size = len;
    VC_PRINTF("fifo_size:%d\r\n", p_vchanger->fifo_size);    
    if(!p_vchanger->fifo) {
        VC_PRINTF("no fifo VC_NOMEMORY\r\n");
        goto init_fail;
    }
    
    VC_MEMSET(&p_vchanger->fifo[0], 0, sizeof(p_vchanger->fifo_size));
    VC_MEMSET(&p_vchanger->linear[0], 0, sizeof(p_vchanger->linear_size));
    VC_MEMSET(&p_vchanger->last[0], 0, sizeof(p_vchanger->last_size));

    voice_changer_init_sample_len(p_vchanger);
    p_vchanger->sample_idx = VC_DEF_IDX;
    p_vchanger->sample_len = p_vchanger->sample_t[p_vchanger->sample_idx];
    VC_PRINTF("def sample idx:%d, %d\r\n", p_vchanger->sample_idx,
        p_vchanger->sample_len);
    p_vchanger->audio_freq = freq;
    p_vchanger->status = VC_STOP;

    VC_PRINTF("voice_changer_initial ok\r\n");
    
    return VC_OK;

init_fail:

    if(p_vchanger->fifo)
        VC_FREE(p_vchanger->fifo);

    if(p_vchanger->linear)
        VC_FREE(p_vchanger->linear);

    if(p_vchanger->last)
        VC_FREE(p_vchanger->last);

    if(p_vchanger)
        VC_FREE(p_vchanger);
    p_vchanger = NULL;

    return VC_NOMEMORY;
}

void voice_changer_exit(void)
{
    if(!p_vchanger)
        return; 
        
    if(p_vchanger->fifo)
        VC_FREE(p_vchanger->fifo);

    if(p_vchanger->linear)
        VC_FREE(p_vchanger->linear);

    if(p_vchanger->last)
        VC_FREE(p_vchanger->last);

    VC_FREE(p_vchanger);
    p_vchanger = NULL;

    VC_PRINTF("voice_changer_exit\r\n");    
}

void voice_changer_start(void)
{
    VC_INT_DECLARATION();
    
    if(!p_vchanger)
        return; 

    VC_MEMSET(&p_vchanger->fifo[0], 0, sizeof(p_vchanger->fifo_size));
    VC_MEMSET(&p_vchanger->linear[0], 0, sizeof(p_vchanger->linear_size));
    VC_MEMSET(&p_vchanger->last[0], 0, sizeof(p_vchanger->last_size));
    
    VC_INT_DISABLE();
    p_vchanger->in_len = 0;
    p_vchanger->status = VC_FIRST;
    VC_INT_RESTORE();

    VC_PRINTF("voice_changer_start\r\n"); 
}

void voice_changer_stop(void)
{
    VC_INT_DECLARATION();
    
    if(!p_vchanger)
        return; 

    VC_INT_DISABLE();
    p_vchanger->status = VC_STOP;
    VC_INT_RESTORE();

    VC_PRINTF("voice_changer_stop\r\n"); 
}

void voice_changer_set_change_flag(void)
{
    VC_INT_DECLARATION();

    if(!p_vchanger) 
        return;
    
    VC_INT_DISABLE();
    p_vchanger->change_flag = 1;
    VC_INT_RESTORE();
}

int voice_changer_get_need_mic_data(void)
{
    int left_len;

    if((!p_vchanger) || (p_vchanger->status == VC_STOP)) {
        // voice_changer not initial
        return -1;
    }

    if(p_vchanger->status == VC_FIRST) {
        left_len = p_vchanger->sample_len - p_vchanger->in_len;
        
    }else {
        left_len = p_vchanger->last_size - p_vchanger->in_len;
    }
    left_len *= sizeof(int32_t);

    return left_len;
}

int voice_changer_set_cost_data(int cost_len)
{
    int left_len;
    VC_INT_DECLARATION();
    
    if((!p_vchanger) || (p_vchanger->status == VC_STOP)) {
        // voice_changer not initial
        return -1;
    }
 
    left_len = p_vchanger->out_len;
    left_len -= cost_len;
    VC_INT_DISABLE();
    p_vchanger->out_len = left_len;
    VC_INT_RESTORE();

    if(left_len <= 0) {
        int len, copy_size;
        
        // copy left data to front of fifo
        len = p_vchanger->last_size;
        copy_size = p_vchanger->sample_len - len;
        VC_INT_DISABLE();
        p_vchanger->in_len = 0;
        VC_MEMCPY(&p_vchanger->fifo[0], &p_vchanger->fifo[len], copy_size * sizeof(int32_t));
        VC_INT_RESTORE();

        voice_changer_set_sample_len(p_vchanger,0);
    }

    return left_len;
}

int voice_changer_data_handle(uint8_t *mic_in, int mic_len, uint8_t **vc_out)
{
    int len, left_len;
    uint8_t *in_put;
    VC_INT_DECLARATION();

    if((!p_vchanger) || (p_vchanger->status == VC_STOP)) {
        // voice_changer not initial
        *vc_out = NULL;
        return -1;
    }

    if(p_vchanger->status == VC_FIRST) 
    {
        left_len = p_vchanger->sample_len - p_vchanger->in_len;
        in_put = (uint8_t*)&p_vchanger->fifo[p_vchanger->in_len + 1];
        left_len *= sizeof(int32_t);

        if(mic_len > left_len) {
            VC_PRINTF("mic too long %d-%d\r\n", mic_len, left_len);
            len = left_len;
        }else {
            len = mic_len;
        }
        VC_MEMCPY(in_put, mic_in, len);
        
        left_len -= len;
        
        VC_INT_DISABLE();
        p_vchanger->in_len += len/sizeof(int32_t);
        VC_INT_RESTORE();
        
        if(left_len > 0) {
            // no enough data for vc, return 0
            *vc_out = NULL;
            return 0;
        } else {
            voice_changer_init_first_sample(p_vchanger);
        }
    }
    else 
    {
        if(p_vchanger->out_len) {
            len = p_vchanger->out_len;
            goto fill_buf;
        } 

        left_len = p_vchanger->last_size - p_vchanger->in_len;
        len = p_vchanger->sample_len - p_vchanger->last_size + p_vchanger->in_len;
        in_put = (uint8_t*)&p_vchanger->fifo[len];
        left_len *= sizeof(int32_t);
        
        if(mic_len > left_len) {
            VC_PRINTF("mic too long %d-%d\r\n", mic_len, left_len);
            len = left_len;
        }else {
            len = mic_len;
        }
        VC_MEMCPY(in_put, mic_in, len);
        
        left_len -= len;
        
        VC_INT_DISABLE();
        p_vchanger->in_len += len/sizeof(int32_t);
        VC_INT_RESTORE();

        // if no enough data for vc, return 0
        if(left_len > 0) {
            *vc_out = NULL;
            return 0;
        }
    }

    voice_changer_linear(p_vchanger);
    len = voice_changer_transit(p_vchanger,voice_changer_min_index(p_vchanger)); 

    VC_INT_DISABLE();
    p_vchanger->out_len = len;
    VC_INT_RESTORE();

fill_buf:
    
    left_len = p_vchanger->last_size - p_vchanger->out_len/sizeof(int32_t);
    *vc_out = (uint8_t*)&p_vchanger->fifo[left_len];

    return p_vchanger->out_len;
}

#endif  // CONFIG_VOICE_CHANGER

