#ifndef __APP_VOICE_CHANGER_H__
#define __APP_VOICE_CHANGER_H__

#define VC_OUT_SIZE                   (40)
#define VC_DEF_SAMPLE_SIZE            (40)

#define VC_DEF_IDX                     3

typedef enum {
    VC_OK,
    VC_NOMEMORY,        
} VC_ERR;

VC_ERR voice_changer_initial(uint32_t freq);
void voice_changer_exit(void);
void voice_changer_start(void);
void voice_changer_stop(void);
void voice_changer_set_change_flag(void);
int voice_changer_get_need_mic_data(void);
int voice_changer_set_cost_data(int cost_len);
int voice_changer_data_handle(uint8_t *mic_in, int mic_len, uint8_t **vc_out);

#endif  // __APP_VOICE_CHANGER_H__
