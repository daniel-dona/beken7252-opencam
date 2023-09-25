#include "include.h"
#include "arm_arch.h"
#include "audio.h"
#include "audio_pub.h"
#include "intc_pub.h"
#include "icu_pub.h"
#include "sys_ctrl_pub.h"
#include "mem_pub.h"
#include "gpio_pub.h"

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#define AUD_ADC_DEF_DMA_CHANNEL     GDMA_CHANNEL_2
#endif

#if CFG_USE_AUD_ADC
enum
{
    AUD_ADC_STA_CLOSED   = 0,
    AUD_ADC_STA_OPENED,
    AUD_ADC_STA_PAUSE,
    AUD_ADC_STA_PLAYING
};

typedef struct aud_adc_desc
{
    UINT8 *buf;
    UINT16 buf_len;
    UINT16 freq;
    UINT16 channels;
    UINT16 mode;
    UINT32 linein_detect_pin;

    UINT32 status;
} AUD_ADC_DESC_ST, *AUD_ADC_DESC_PTR;

void audio_adc_set_enable_bit(UINT32 enable)
{
    UINT32 reg_addr = AUDIO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    // linein and adc enable bits are the two channels for adc
    // open both of them, no mater channel is.
    if (enable)
        reg_val |= (ADC_ENABLE | LINEIN_ENABLE);
    else
        reg_val &= ~(ADC_ENABLE | LINEIN_ENABLE);
    REG_WRITE(reg_addr, reg_val);
}

void audio_adc_set_int_enable_bit(UINT32 enable)
{
    UINT32 reg_addr = AUD_FIFO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (enable)
        reg_val |= ADC_INT_EN;
    else
        reg_val &= ~ADC_INT_EN;
    REG_WRITE(reg_addr, reg_val);
}

void audio_adc_get_l_sample(INT16 *left)
{
    UINT32 reg_addr = AUD_ADC_FIFO_PORT;
    UINT32 reg_val = REG_READ(reg_addr);

    *left = (INT16)(reg_val & AD_ADC_L_FIFO_MASK);
}

void audio_adc_get_l_and_r_samples(INT16 *left, INT16 *right)
{
    UINT32 reg_addr = AUD_ADC_FIFO_PORT;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val &= AD_ADC_LR_FIFO_MASK;

    *left = (INT16)(reg_val & AD_ADC_L_FIFO_MASK);
    *right = (INT16)((reg_val >> AD_ADC_R_FIFO_POSI) & AD_ADC_R_FIFO_MASK);
}

void audio_adc_set_hpf2_bypass_bit(UINT32 enable)
{
    UINT32 reg_addr = AUD_ADC_CONFIG_0;
    UINT32 reg_val = REG_READ(reg_addr);

    if (enable)
        reg_val |= ADC_HPF2_BYPASS;
    else
        reg_val &= ~ADC_HPF2_BYPASS;
    REG_WRITE(reg_addr, reg_val);
}

void audio_adc_set_gain(UINT32 gain)
{
    UINT32 reg_addr = AUD_ADC_CONFIG_0;
    UINT32 reg_val = REG_READ(reg_addr);

    if (gain > ADC_SET_GAIN_MASK)
        gain = ADC_SET_GAIN_MASK;

    reg_val &= ~(ADC_SET_GAIN_MASK << ADC_SET_GAIN_POSI);
    reg_val |= ((gain & ADC_SET_GAIN_MASK)  << ADC_SET_GAIN_POSI);

    REG_WRITE(reg_addr, reg_val);
}

void audio_adc_set_write_thred_bit(UINT32 thred)
{
    UINT32 reg_addr = AUD_FIFO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (thred > ADC_WR_THRED_MASK)
        thred = ADC_WR_THRED_MASK;

    reg_val &= ~(ADC_WR_THRED_MASK << ADC_WR_THRED_POSI);
    reg_val &= ~(DTMF_WR_THRED_MASK << DTMF_WR_THRED_POSI);
    reg_val |= ((thred & ADC_WR_THRED_MASK) << ADC_WR_THRED_POSI);
    reg_val |= ((thred & DTMF_WR_THRED_MASK) << DTMF_WR_THRED_POSI);

    REG_WRITE(reg_addr, reg_val);
}

void audio_adc_set_sample_rate(UINT32 sample_rate)
{
    UINT32 reg;

    /* disable adc handset bit again, to make sure this bit unset */
    reg = REG_READ(AUD_EXTEND_CFG);
    reg &= ~(ADC_FRACMOD_MANUAL);
    REG_WRITE(AUD_EXTEND_CFG, reg);

    switch (sample_rate)
    {
    case 11025:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= ADC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_441K);
        REG_WRITE(AUD_ADC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_44_1_K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 22050: //
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= ADC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_441K >> 1);
        REG_WRITE(AUD_ADC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_44_1_K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 44100:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_44_1_K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 12000:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= ADC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_48K);
        REG_WRITE(AUD_ADC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 24000:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= ADC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_48K >> 1);
        REG_WRITE(AUD_ADC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 48000:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 8000:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_8K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 16000:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_16K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 32000:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= ADC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_32K);
        REG_WRITE(AUD_ADC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_ADC_MASK << SAMPLE_RATE_ADC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_ADC_MASK) << SAMPLE_RATE_ADC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    default:
        AUD_PRT("unsupported sample rate:%d\r\n", sample_rate);
        break;
    }
}

#if CFG_GENERAL_DMA
void audio_adc_set_dma(UINT32 enable)
{
    GDMA_CFG_ST en_cfg;

    en_cfg.channel = AUD_ADC_DEF_DMA_CHANNEL;
    if (enable)
        en_cfg.param = 1;
    else
        en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
}

static void audio_adc_eixt_dma(void)
{
    GDMA_CFG_ST en_cfg;

    en_cfg.channel = AUD_ADC_DEF_DMA_CHANNEL;
    en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
}
#endif // CFG_GENERAL_DMA

void audio_adc_open_analog_regs(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_OPEN_ADC_MIC_ANALOG, NULL);
}

void audio_adc_close_analog_regs(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_CLOSE_ADC_MIC_ANALOG, NULL);
}

void audio_adc_enable_linein(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_ENALBLE_ADC_LINE_IN, NULL);
}

void audio_adc_disable_linein(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_DISALBLE_ADC_LINE_IN, NULL);
}

static void audio_adc_init_linein_detect_pin(UINT32 pin)
{
    UINT32 param;

    ASSERT(pin < GPIONUM);

    param = GPIO_CFG_PARAM(pin, GMODE_INPUT_PULLUP);
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
}

static void audio_adc_linein_detect(void)
{
    // TODO:
}

void audio_adc_set_volume(UINT32 volume)
{
    UINT32 act_vol;
    UINT8 high, low;
    UINT32 reg_addr = AUD_AGC_CONFIG_2;
    UINT32 reg_val = REG_READ(reg_addr);

    if (volume > AUD_ADC_MAX_VOLUME)
        volume = AUD_ADC_MAX_VOLUME;

    high = volume & 0x70;
    low = volume & 0x0f;

    if (low > 0xc)
        low = 0xc;

    act_vol = high | low;

    reg_val &= ~(MANUAL_PGA_VAL_MASK << MANUAL_PGA_VAL_POSI);
    reg_val |= ((act_vol & MANUAL_PGA_VAL_MASK) << MANUAL_PGA_VAL_POSI);

    REG_WRITE(reg_addr, reg_val);

    AUD_PRT("set adc vol: %d - %d\r\n", volume, act_vol);
}

#endif // CFG_USE_AUD_ADC

