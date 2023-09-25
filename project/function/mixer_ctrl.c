#include "rtthread.h"
#include "include.h"
#include "sound_delay.h"
#include "mixer_ctrl.h"

#if (CONFIG_SOUND_MIXER && defined(CFG_CONTROL_MIXER_PARAMETER_BY_SARADC))
static uint8 g_saradc_step_status = 0;

static void sdly_change_decay_value_byadc(int32_t val)
{
    SDELAY_ST *mixer_ptr;
    uint32 tmp_value = val >> 4;

    if(0 == sdly_is_valid_mixer_entity())
    {
        return;
    }

    mixer_ptr = sdly_get_mixer_entity();

    if(mixer_ptr->decay_value_adc == tmp_value)
        return;

    mixer_ptr->decay_value_adc = tmp_value;
    mixer_ptr->decay_value = SDLY_DEF_DECAY_VAL * tmp_value / 63;

    if(mixer_ptr->decay_value > SDLY_DEF_DECAY_VAL)
        mixer_ptr->decay_value = SDLY_DEF_DECAY_VAL;

    SDLY_PRINTF("g_sound_mixer decay:%d,adc_val:%d\r\n", mixer_ptr->decay_value, val);
}

static void sdly_change_vol_byadc(uint16_t val, uint8 mode)
{
    uint16 tmp_vol;
    SDELAY_ST *mixer_ptr;

    if(0 == sdly_is_valid_mixer_entity())
    {
        return;
    }
    mixer_ptr = sdly_get_mixer_entity();

    tmp_vol = val >> 6;

    if(mode == 0)
    {
        if(mixer_ptr->pcm_vol == tmp_vol)
            return;

        SDLY_PRINTF("--vol:%d,val:%d,tmp_vol:%x--\r\n", mixer_ptr->pcm_vol, val, tmp_vol);
        mixer_ptr->pcm_vol = tmp_vol;

        if(tmp_vol != 0)
            mixer_ptr->pcm_vol = tmp_vol;
    }
    else
    {
        if(mixer_ptr->micphone_vol == tmp_vol)
            return;

        SDLY_PRINTF("---mic vol:%d,val:%d,tmp:%d--\r\n", mixer_ptr->micphone_vol, val, tmp_vol);

        mixer_ptr->micphone_vol = tmp_vol;
    }
}

static void sdly_change_mic_param_byadc(int32_t val, uint8 mode)
{
    SDELAY_ST *mixer_ptr;
    uint8 tmp_value = val >> 5;

    if(0 == sdly_is_valid_mixer_entity())
    {
        return;
    }
    mixer_ptr = sdly_get_mixer_entity();

    if(tmp_value >= 15)
        tmp_value = 15;

    if(mode == 0)
    {
        if(mixer_ptr->pcm_vol == 15 - tmp_value)
            return;

        mixer_ptr->pcm_vol = 15 - tmp_value;
    }
}

void sdly_process_dac_result(uint16_t channel, uint16_t value)
{
    if(channel == SDLY_ADC_SAMPLE_TIME_CHANNEL)
    {
        if(g_saradc_step_status == 1)
        {
            sdly_change_mic_param_byadc((uint32_t)value, 0);
        }
    }
    else if(channel == SDLY_ADC_PCM_VOL_CHANNEL)
    {
        sdly_change_vol_byadc(value, 1);
    }
    else if(channel == SDLY_ADC_MIC_VOL_CHANNEL)
    {
        sdly_change_decay_value_byadc((int32_t)value);
    }
}

void sdly_scanning_dac(void)
{
    static uint16_t scan_cnt = 0;
    static uint16_t bat_detect_num = BAT_DETECT_NUM + 1;

    scan_cnt++;

    if(scan_cnt == SCAN_CNT_THR_DECAY)
    {
        gpio_config(SDLY_ADC_DECAY_GPIO_PIN, 5 );
        saradc_init(SDLY_ADC_MODE, SDLY_ADC_DECAY_CHANNEL, 3);
        g_saradc_step_status = 0;
    }
    else if(scan_cnt == SCAN_CNT_THR_MIC)
    {
        gpio_config(SDLY_ADC_MICVOL_GPIO_PIN, 5 );
        saradc_init(SDLY_ADC_MODE, SDLY_ADC_MIC_VOL_CHANNEL, 3);
        g_saradc_step_status = 0;
    }
    else if(scan_cnt == SCAN_CNT_THR_SPTIME)
    {
        gpio_config( 10, 5 );
        gpio_config( 9, 5 );

        gpio_config( 7, 1 );
        gpio_output( 7, 0 );
        saradc_init(SDLY_ADC_MODE, SDLY_ADC_SAMPLE_TIME_CHANNEL, 3 );
        g_saradc_step_status = 1;
    }
    else if(scan_cnt == SCAN_CNT_THR_HIGH_FREQ)
    {
        gpio_config( 10, 5 );
        gpio_config( 7, 5 );

        gpio_config( 9, 1 );
        gpio_output( 9, 0 );
        saradc_init(SDLY_ADC_MODE, SDLY_ADC_SAMPLE_TIME_CHANNEL, 3 );
        g_saradc_step_status = 2;
    }
    else if(scan_cnt == SCAN_CNT_THR_LOW_FREQ)
    {
        gpio_config( 9, 5 );
        gpio_config( 7, 5 );

        gpio_config( 10, 1 );
        gpio_output( 10, 0 );
        saradc_init(SDLY_ADC_MODE, SDLY_ADC_SAMPLE_TIME_CHANNEL, 3 );
        g_saradc_step_status = 3;
        if(bat_detect_num++  < BAT_DETECT_NUM)
            scan_cnt = 0;
    }
    else if(scan_cnt >= SCAN_CNT_THR_BAT)
    {
        saradc_init( SARADC_MODE_SINGLESTEP, 0, 3 );
        bat_detect_num = 0;
        scan_cnt = 0;
    }
}

#endif
// eof

