#ifndef __AUDIO_PUB_H__
#define __AUDIO_PUB_H__

#define AUD_FAILURE                  (1)
#define AUD_SUCCESS                  (0)

enum CODEC_STATE
{
    DAC_DMA_IRQ_ENABLE = 0x01,
    ADC_DMA_IRQ_ENABLE = 0x02,
    DAC_IRQ_ENABLE     = 0x04,
    ADC_IRQ_ENABLE     = 0x08,
    DAC_IS_OPENED      = 0x10,    
    ADC_IS_OPENED      = 0x40,    
};

#define AUD_DAC_DEV_NAME             "aud_dac"
#define AUD_DAC_CMD_MAGIC            (0x1EBC0000)

typedef struct aud_dac_cfg_st
{
    UINT8 *buf;
    UINT16 buf_len;
    UINT16 freq;
    UINT16 channels;
    UINT16 dma_mode;
} AUD_DAC_CFG_ST, *AUD_DAC_CFG_PTR;

enum
{
    AUD_DAC_CMD_GET_FREE_BUF_SIZE = AUD_DAC_CMD_MAGIC + 1,
    AUD_DAC_CMD_PLAY,
    AUD_DAC_CMD_PAUSE,

};

#define AUD_DAC_VOL_TABLE_LEN       (17)
#define AUD_USE_EXT_PA              1
#include "gpio_pub.h"
#if AUD_USE_EXT_PA
#define AUD_DAC_MUTE_PIN            GPIO9
#define AUD_DAC_MUTE_ENA_LEVEL      GPIO_INT_LEVEL_LOW
#endif


///////////////////////////////////////////////////////////////////////////////


#define AUD_ADC_DEV_NAME             "aud_adc"
#define AUD_ADC_CMD_MAGIC            (0x2EBC0000)
typedef struct aud_adc_cfg_st
{
    UINT8 *buf;
    UINT16 buf_len;
    UINT16 freq;
    UINT16 channels;
    UINT16 mode;
    UINT32 linein_detect_pin;
} AUD_ADC_CFG_ST, *AUD_ADC_CFG_PTR;

enum
{
    AUD_ADC_CMD_GET_FILL_BUF_SIZE = AUD_ADC_CMD_MAGIC + 1,
    AUD_ADC_CMD_PLAY,
    AUD_ADC_CMD_PAUSE,
    AUD_ADC_CMD_DO_LINEIN_DETECT,
};

#define AUD_ADC_LINEIN_DETECT_PIN           GPIO8
#define AUD_ADC_LINEIN_ENABLE_LEVEL         0

#define AUD_ADC_MODE_DMA_BIT                (1 << 0)  // 1: DMA MODE, 0: ISR MODE
#define AUD_ADC_MODE_LINEIN                 (1 << 1)  // 1: LINEIN, 0: MIC 
#define AUD_ADC_DEF_WR_THRED                (8)
#define AUD_ADC_DEF_GAIN                    (0x2D)   // 0dm
#define AUD_ADC_MAX_THRED                   (0x10)
#define AUD_ADC_MAX_VOLUME                  (124)

void audio_adc_set_enable_bit(UINT32 enable);
void audio_adc_set_int_enable_bit(UINT32 enable);
void audio_adc_get_l_sample(INT16 *left);
void audio_adc_get_l_and_r_samples(INT16 *left, INT16 *right);
void audio_adc_set_hpf2_bypass_bit(UINT32 enable);
void audio_adc_set_gain(UINT32 gain);
void audio_adc_set_write_thred_bit(UINT32 thred);
void audio_adc_set_sample_rate(UINT32 sample_rate);
void audio_adc_set_dma(UINT32 enable);
void audio_adc_set_volume(UINT32 volume);

/* DAC Interface */
void audio_dac_set_enable_bit(UINT32 enable);
void audio_dac_set_int_enable_bit(UINT32 enable);
void audio_dac_set_read_thred_bit(UINT32 thred);
void audio_dac_set_gain(UINT32 gain);
void audio_dac_set_hpf1_bit(UINT32 enable);
void audio_dac_set_hpf2_bit(UINT32 enable);
void audio_dac_set_sample_rate(UINT32 sample_rate);
void audio_dac_set_sample(INT16 left, INT16 right);
void audio_dac_open_analog_regs(void);
void audio_dac_close_analog_regs(void);
void audio_dac_set_analog_mute(UINT32 enable);
void audio_dac_init_mute_pin(void);
void audio_dac_eable_mute(UINT32 enable);
UINT32 audio_dac_is_mute(void);
void audio_dac_set_volume(UINT32 percent);
void audio_dac_volume_use_single_port(void);
void audio_dac_volume_diff_port(void);

/* ADC Interface */
void audio_adc_open_analog_regs(void);
void audio_adc_close_analog_regs(void);

void audio_init(void);
void audio_exit(void);

#endif // __AUDIO_PUB_H__