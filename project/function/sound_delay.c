#include "rtthread.h"
#include "include.h"
#include "sound_delay.h"
#include <string.h>

#if CONFIG_SOUND_MIXER
static SDELAY_ST *sound_delay_ptr = NULL;
static uint8_t last_mic_low_freq = 0;
static uint8_t last_mic_high_freq = 0;
static uint32_t g_forepart_mute_count = 0;

void sdly_set_mixer_default_value(void)
{
    sound_delay_ptr->decay_value = 50;
    sound_delay_ptr->pcm_vol = 30;
    sound_delay_ptr->micphone_vol = 50;
}

void sdly_set_micphone_vol(int vol)
{
    if(sound_delay_ptr)
    {
        SDLY_PRINTF("sdly_set_micphone_vol :%d-%d\r\n",
            sound_delay_ptr->micphone_vol, vol);
        if(vol < 0)
            vol = 0;

        sound_delay_ptr->micphone_vol = vol;
    }
}

void sdly_set_value(int argc, char** argv)
{
    int32_t decay_value, pcm_vol, micphone_vol;
    
    if (argc != 4) 
    {
        SDLY_PRINTF("sdly_set_value err\r\n");
        return;
    }

    if(argc == 4) 
    {
        decay_value = atoi(argv[1]);
        pcm_vol = atoi(argv[2]);
        micphone_vol = atoi(argv[3]);
        
        SDLY_PRINTF("set decay pcm mic:%d-%d-%d\r\n", 
            decay_value, pcm_vol, micphone_vol);
        
        sound_delay_ptr->decay_value = decay_value;
        sound_delay_ptr->pcm_vol = pcm_vol;
        sound_delay_ptr->micphone_vol = micphone_vol;
    }
}
MSH_CMD_EXPORT(sdly_set_value, sdly_set_value test);

int32 sdly_init_mixer_entity(void)
{
    int32 ret = 0;

    if(sound_delay_ptr)
    {
        SDLY_PRINTF("[wzl]warning[sdly_init_mixer_entity], inited\r\n");
        SDLY_MEMSET(sound_delay_ptr, 0, sizeof(*sound_delay_ptr));
        sdly_set_mixer_default_value();

        goto init_exit;
    }

    sound_delay_ptr = (SDELAY_ST *)SDLY_MALLOC(sizeof(*sound_delay_ptr));
    if(NULL == sound_delay_ptr)
    {
        ret = -1;
        goto init_exit;
    }
    SDLY_PRINTF("[wzl]sdly_init_mixer_entity:0x%x\r\n", sound_delay_ptr);

    sdly_set_mixer_default_value();

init_exit:
    return ret;
}

int32 sdly_uninit_mixer_entity(void)
{
    if(sound_delay_ptr)
    {
        SDLY_MEMSET(sound_delay_ptr, 0, sizeof(*sound_delay_ptr));

        SDLY_FREE(sound_delay_ptr);
        sound_delay_ptr = 0;
    }

    return 0;
}

int32 sdly_is_valid_mixer_entity(void)
{
    return (sound_delay_ptr ? 1 : 0);
}

SDELAY_ST *sdly_get_mixer_entity(void)
{
    return sound_delay_ptr;
}

int32_t sdly_get_sample_time_fromadc(void)
{
    return SDLY_DEF_SAMPLE_TIME_MS;
}

int32_t sdly_get_decay_value_fromadc(void)
{
    return SDLY_DEF_DECAY_VAL;
}

uint8_t pre_emphasis_low_is_changed(uint8_t low_freq)
{
    if(low_freq != last_mic_low_freq)
    {
        last_mic_low_freq = low_freq;
        return low_freq;
    }

    return 0;
}

uint8_t pre_emphasis_high_is_changed(uint8_t high_freq)
{
    if(high_freq != last_mic_high_freq)
    {
        last_mic_high_freq = high_freq;
        return high_freq;
    }

    return 0;
}

void sdly_change_sample_time_byadc(uint32_t time)
{
    uint32_t tmp_value = time >> 4;

    if(0 == sdly_is_valid_mixer_entity())
    {
        return;
    }

    if(sound_delay_ptr->sample_time_adc == tmp_value)
        return;

    sound_delay_ptr->sample_time_adc = tmp_value;
    if(tmp_value < 2)
        sound_delay_ptr->sample_time = 6;
    else
    {
        sound_delay_ptr->sample_time = 6 + (tmp_value << 1) + tmp_value;
    }

    sound_delay_ptr->sample_len = sound_delay_ptr->audio_freq * sound_delay_ptr->sample_time / SDLY_SAMPLE_TIME_ONE_S;

    if(sound_delay_ptr->audio_freq == 8000)
    {
        if(sound_delay_ptr->sample_time > SDLY_MAX_SAMPLE_TIME_MS_8K )
            sound_delay_ptr->sample_time = SDLY_MAX_SAMPLE_TIME_MS_8K;

        if(sound_delay_ptr->sample_len >= SDLY_MAX_FIFO_SIZE_8K)
            sound_delay_ptr->sample_len = SDLY_MAX_FIFO_SIZE_8K - 1;
    }
    else
    {
        if(sound_delay_ptr->sample_time > SDLY_MAX_SAMPLE_TIME_MS )
            sound_delay_ptr->sample_time = SDLY_MAX_SAMPLE_TIME_MS;

        if(sound_delay_ptr->sample_len >= SDLY_MAX_FIFO_SIZE)
            sound_delay_ptr->sample_len = SDLY_MAX_FIFO_SIZE - 1;
    }

    SDLY_MEMSET(sound_delay_ptr->fifo, 0, sizeof(int16_t) * (sound_delay_ptr->sample_len + 1));
    sound_delay_ptr->op_ptr = &sound_delay_ptr->fifo[sound_delay_ptr->sample_len];

    SDLY_PRINTF("tmp_value:%d,adc_val:%d\r\n", sound_delay_ptr->sample_time, time);
}

uint16 sdly_get_pcm_vol(void)
{
    if(0 == sdly_is_valid_mixer_entity())
    {
        return -1;
    }

    return sound_delay_ptr->pcm_vol;
}

void sdly_set_pcm_vol(uint8_t vol)
{
    sound_delay_ptr->pcm_vol = 16;
}

void sdly_set_sample_vol(uint8_t vol)
{
    if(0 == sdly_is_valid_mixer_entity())
    {
        return;
    }

    if((sound_delay_ptr->sample_time_adc == vol) || (sound_delay_ptr->decay_value_adc == vol))
        return;

    sound_delay_ptr->sample_time_adc = vol;
    sound_delay_ptr->decay_value_adc = vol;

    if(sound_delay_ptr->sample_time_adc < 2)
        sound_delay_ptr->sample_time = 6;
    else
    {
        sound_delay_ptr->sample_time = 6 + (sound_delay_ptr->sample_time_adc << 1) + sound_delay_ptr->sample_time_adc;
    }

    sound_delay_ptr->sample_len = sound_delay_ptr->audio_freq * sound_delay_ptr->sample_time / SDLY_SAMPLE_TIME_ONE_S;
    if(sound_delay_ptr->sample_len >= SDLY_MAX_FIFO_SIZE)
        sound_delay_ptr->sample_len = SDLY_MAX_FIFO_SIZE - 1;

    SDLY_MEMSET(sound_delay_ptr->fifo, 0, sizeof(int16_t) * (sound_delay_ptr->sample_len + 1));
    sound_delay_ptr->op_ptr = &sound_delay_ptr->fifo[sound_delay_ptr->sample_len];

    if(sound_delay_ptr->decay_value_adc == 0)
        sound_delay_ptr->decay_value = 1;
    else
        sound_delay_ptr->decay_value = sound_delay_ptr->decay_value_adc + (sound_delay_ptr->decay_value_adc >> 1);// 1.5 * tmp_value
}

SDELAY_ERR sdly_config(uint32_t freq)
{
    SDLY_PRINTF("\r\n\r\n[wzl]sdly_config:%d\r\n", freq);
    if(0 == sdly_is_valid_mixer_entity())
    {
        return SDLY_NOMEMORY;
    }

    if(8000 == freq)
    {
        SDLY_MEMSET(sound_delay_ptr->fifo, 0, SDLY_MAX_FIFO_SIZE_8K * sizeof(int16_t));

        sound_delay_ptr->max_sample_time = ((SDLY_MAX_FIFO_SIZE_8K - 1) * SDLY_SAMPLE_TIME_ONE_S / 8000);
        sound_delay_ptr->sample_time = SDLY_DEF_SAMPLE_TIME_MS_8K;
        sound_delay_ptr->audio_freq = 8000;
        sound_delay_ptr->sample_len = 8000 * sound_delay_ptr->sample_time / SDLY_SAMPLE_TIME_ONE_S;
    }
    else
    {
        SDLY_MEMSET(sound_delay_ptr->fifo, 0, SDLY_MAX_FIFO_SIZE * sizeof(int16_t));

        sound_delay_ptr->max_sample_time = ((SDLY_MAX_FIFO_SIZE - 1) * SDLY_SAMPLE_TIME_ONE_S / 44100);
        sound_delay_ptr->sample_time = SDLY_DEF_SAMPLE_TIME_MS;
        sound_delay_ptr->audio_freq = 44100;
        sound_delay_ptr->sample_len = 44100 * sound_delay_ptr->sample_time / SDLY_SAMPLE_TIME_ONE_S;
    }

    sound_delay_ptr->op_ptr = &sound_delay_ptr->fifo[sound_delay_ptr->sample_len];
    sound_delay_ptr->status = SDLY_STOP;

    SDLY_PRINTF("sdly_config ed\r\n");

    return SDLY_OK;
}

void sdly_exit(void)
{
}

void sdly_start_play(void)
{
    SDLY_MEMSET(sound_delay_ptr->fifo, 0, sizeof(int16_t) * (sound_delay_ptr->sample_len + 1));

    sound_delay_ptr->op_ptr = &sound_delay_ptr->fifo[sound_delay_ptr->sample_len];
    sound_delay_ptr->status = SDLY_START;

    SDLY_PRINTF("sdly_start_play ed\r\n");
}

static int32_t sdly_low_pass_filter(int32_t sample)
{
    int32_t ret_val = sample;

    if(ret_val > 32767)
        ret_val = 32767;
    else if(ret_val < -32768)
        ret_val = -32768;

    return ret_val;
}

void sdly_straignt_through_playing(uint8_t *simple_buffer, uint8_t *pile_buf, uint16_t simple_len_byte, uint8_t mode)
{
    memcpy(pile_buf, simple_buffer, simple_len_byte);
}

void sdly_mixer_single_playing(uint8_t *sample_ptr, uint16_t simple_len_byte)
{
    int32_t temp;
    int16_t i, len_hword;
    int16_t *sample = (int16_t *)sample_ptr;
    int32_t left_data, right_data, mix_sample_tmp;
    len_hword = simple_len_byte >> 1;

    SDLY_LOG("mixer:0x%x:%d:%d\r\n", sound_delay_ptr->op_ptr,
             sound_delay_ptr->decay_value,
             sound_delay_ptr->status);

    for(i = 0; i < len_hword; i += 2)
    {
        mix_sample_tmp = 0;
        if(sound_delay_ptr->status == SDLY_START)
        {
            mix_sample_tmp = *sound_delay_ptr->op_ptr;
            mix_sample_tmp = mix_sample_tmp >> 1;
        }

        mix_sample_tmp += sample[i + 1];
        mix_sample_tmp = sdly_low_pass_filter(mix_sample_tmp);

        //temp = ((int32_t)(*sound_delay_ptr->op_ptr) * sound_delay_ptr->decay_value ) / SDLY_MIN_DECAY_VAL;
        temp = (int32_t)(*sound_delay_ptr->op_ptr);
        temp += (int32_t)sample[i];
        
        temp = (temp * sound_delay_ptr->micphone_vol) >> 4;
        temp = sdly_low_pass_filter(temp);

        sample[i] =  (int16_t) temp;
        sample[i + 1] = (int16_t) temp;

        if(sound_delay_ptr->status == SDLY_START)
        {
            *(sound_delay_ptr->op_ptr) = (int16_t) mix_sample_tmp;
            if(sound_delay_ptr->op_ptr == sound_delay_ptr->fifo)
                sound_delay_ptr->op_ptr = &sound_delay_ptr->fifo[sound_delay_ptr->sample_len];
            else
                sound_delay_ptr->op_ptr--;
        }
    }
}

void sdly_mixer_multi_playing(uint8_t *sample_adc, uint8_t *sample_audio, uint16_t sample_adc_len)
{
    int32_t temp;
    int16_t i, len_hword;
    int16_t *simple_ptr = (int16_t *)sample_adc;
    int16_t *audio_ptr = (int16_t *)sample_audio;
    int32_t left_data, right_data, mix_sample_tmp;
    len_hword = sample_adc_len >> 1;

    for(i = 0; i < len_hword; i += 2)
    {
        mix_sample_tmp = 0;
        if(sound_delay_ptr->status == SDLY_START)
        {
            mix_sample_tmp = *sound_delay_ptr->op_ptr;
            mix_sample_tmp = mix_sample_tmp >> 1;
        }

        mix_sample_tmp += simple_ptr[i + 1];
        mix_sample_tmp = sdly_low_pass_filter(mix_sample_tmp);

        left_data = (int32) (audio_ptr[i]);
        right_data = (int32)(audio_ptr[i + 1]);

        //temp = ((int32_t)(*sound_delay_ptr->op_ptr) * sound_delay_ptr->decay_value) / SDLY_MIN_DECAY_VAL;
        temp = (int32_t)(*sound_delay_ptr->op_ptr);
        temp += (int32_t)simple_ptr[i];
        temp = (temp * sound_delay_ptr->micphone_vol) >> 4;
		temp = sdly_low_pass_filter(temp);

        left_data = (left_data * sound_delay_ptr->pcm_vol) >> 4;
        left_data += temp;
        left_data = sdly_low_pass_filter(left_data);

        right_data = (right_data * sound_delay_ptr->pcm_vol) >> 4;
        right_data += temp;
        right_data = sdly_low_pass_filter(right_data);

        audio_ptr[i] = (int16_t)left_data;
        audio_ptr[i + 1] = (int16_t)right_data;

        if(sound_delay_ptr->status == SDLY_START)
        {
            *(sound_delay_ptr->op_ptr) = (int16_t) mix_sample_tmp;
            if(sound_delay_ptr->op_ptr == sound_delay_ptr->fifo)
                sound_delay_ptr->op_ptr = &sound_delay_ptr->fifo[sound_delay_ptr->sample_len];
            else
                sound_delay_ptr->op_ptr--;
        }
    }
}

void sdly_init_forepart_mute(void)
{
    g_forepart_mute_count = 0;
}

void sdly_forepart_mute(void *sample, uint32_t len)
{
    if(0 == len)
    {
        return;
    }

    if(g_forepart_mute_count < SDLY_FOREPART_MUTE_SECTION_COUNT)
    {
        g_forepart_mute_count ++;
        SDLY_MEMSET((uint8_t *)sample, 0, len);
    }
    else if(g_forepart_mute_count == SDLY_FOREPART_MUTE_SECTION_COUNT)
    {
        g_forepart_mute_count++;
        SDLY_PRINTF("sdly_forepart_mute\r\n");
    }
}

void sdly_deinit(void)
{
    SDLY_PRINTF("[wzl]close_idle_mixer\r\n\r\n");

    sdly_uninit_mixer_entity();
}

void sdly_init(void)
{
    sdly_init_mixer_entity();
    sdly_init_forepart_mute();

    sdly_config(8000);

    SDLY_PRINTF("[wzl]sdly_init\r\n");
}
#endif

// eof

