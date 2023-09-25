#include "include.h"
#include "arm_arch.h"
#include "audio.h"
#include "audio_pub.h"
#include "intc_pub.h"
#include "icu_pub.h"
#include "sys_ctrl_pub.h"
#include "gpio_pub.h"
#include "mem_pub.h"

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#define AUD_DAC_DEF_DMA_CHANNEL     GDMA_CHANNEL_1
#endif

#if CFG_USE_AUD_DAC
typedef struct aud_volome_s
{
    UINT8 ana_gain;
    UINT8 dig_gain;
} AUD_VOL_ST, *AUD_VOL_PTR;

#define AUD_DAC_DEF_DIGTAL_GAIN     (0x2D)  // 0dm


#if 0//(AUD_USE_EXT_PA == 1)
static const AUD_VOL_ST aud_vol_table[AUD_DAC_VOL_TABLE_LEN] =
{
    {0x00, 0},           // 0
    {0x01, 26},           // 1
    {0x03, 26},           // 1+
    {0x05, 26},           // 2
    {0x07, 26},           // 3
    {0x09, 26},           // 4
    {0x0b, 26},           // 5
    {0x0d, 26},           // 6
    {0x0f, 26},           // 7
    {0x11, 26},           // 8
    {0x13, 26},           // 9
    {0x15, 26},           // A
    {0x17, 26},           // B
    {0x19, 26},           // C
    {0x1B, 26},           // D
    {0x1D, 26},           // E
    {0x1F, 26},           // F
};
#else
static const AUD_VOL_ST aud_vol_table[AUD_DAC_VOL_TABLE_LEN] =
{
    {0x1A, 0},           // 0
    {0x1A, 2},           // 1
    {0x1A, 4},           // 1+
    {0x1A, 6},           // 2
    {0x1A, 9},           // 3
    {0x1A, 12},           // 4
    {0x1A, 15},           // 5
    {0x1A, 18},           // 6
    {0x1A, 21},           // 7
    {0x1A, 24},           // 8
    {0x1A, 27},           // 9
    {0x1A, 30},           // A
    {0x1A, 33},           // B
    {0x1A, 36},           // C
    {0x1A, 39},           // D
    {0x1A, 42},           // E
    {0x1A, 45},           // F
};
#endif

void audio_dac_set_enable_bit(UINT32 enable)
{
    UINT32 reg_addr = AUDIO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (enable)
        reg_val |= DAC_ENABLE;
    else
        reg_val &= ~DAC_ENABLE;
    REG_WRITE(reg_addr, reg_val);
}

UINT32 audio_dac_is_enable_bit(void)
{
    UINT32 reg_addr = AUDIO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & DAC_ENABLE);

    return reg_val ? 1 : 0;
}

void audio_dac_set_int_enable_bit(UINT32 enable)
{
    UINT32 reg_addr = AUD_FIFO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (enable)
        reg_val |= (DAC_R_INT_EN | DAC_L_INT_EN);
    else
        reg_val &= ~(DAC_R_INT_EN | DAC_L_INT_EN);
    REG_WRITE(reg_addr, reg_val);
}

void audio_dac_set_read_thred_bit(UINT32 thred)
{
    UINT32 reg_addr = AUD_FIFO_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val &= ~(DAC_R_RD_THRED_MASK << DAC_R_RD_THRED_POSI);
    reg_val &= ~(DAC_L_RD_THRED_MASK << DAC_L_RD_THRED_POSI);

    reg_val |= ((thred & DAC_R_RD_THRED_MASK) << DAC_R_RD_THRED_POSI);
    reg_val |= ((thred & DAC_L_RD_THRED_MASK) << DAC_L_RD_THRED_POSI);

    REG_WRITE(reg_addr, reg_val);
}

void audio_dac_set_hpf1_bit(UINT32 enable)
{
    UINT32 reg_addr = AUD_DAC_CONFIG_0;
    UINT32 reg_val = REG_READ(reg_addr);

    if (enable)
        reg_val |= DAC_HPF1_BYPASS;
    else
        reg_val &= ~DAC_HPF1_BYPASS;
    REG_WRITE(reg_addr, reg_val);
}

void audio_dac_set_hpf2_bit(UINT32 enable)
{
    UINT32 reg_addr = AUD_DAC_CONFIG_0;
    UINT32 reg_val = REG_READ(reg_addr);

    if (enable)
        reg_val |= DAC_HPF2_BYPASS;
    else
        reg_val &= ~DAC_HPF2_BYPASS;
    REG_WRITE(reg_addr, reg_val);
}

void audio_dac_set_gain(UINT32 gain)
{
    UINT32 reg_addr = AUD_DAC_CONFIG_0;
    UINT32 reg_val = REG_READ(reg_addr);

    if (gain > DAC_SET_GAIN_MASK)
        gain = DAC_SET_GAIN_MASK;

    reg_val &= ~(DAC_SET_GAIN_MASK << DAC_SET_GAIN_POSI);
    reg_val |= ((gain & DAC_SET_GAIN_MASK)  << DAC_SET_GAIN_POSI);

    REG_WRITE(reg_addr, reg_val);
}

void audio_dac_set_sample(INT16 left, INT16 right)
{
    UINT32 reg_addr = AUD_DAC_FIFO_PORT;
    UINT32 reg_val;

    reg_val = ((left & AD_DAC_L_FIFO_MASK) << AD_DAC_L_FIFO_POSI)
              | ((right & AD_DAC_R_FIFO_MASK) << AD_DAC_R_FIFO_POSI);

    REG_WRITE(reg_addr, reg_val);
}

void audio_dac_set_sample_rate(UINT32 sample_rate)
{
    UINT32 reg;

    /* disable dac handset bit again, to make sure this bit unset */
    reg = REG_READ(AUD_EXTEND_CFG);
    reg &= ~(DAC_FRACMOD_MANUAL);
    REG_WRITE(AUD_EXTEND_CFG, reg);

    switch (sample_rate)
    {
    case 11025:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= DAC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_441K << 2);
        REG_WRITE(AUD_DAC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_44_1_K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 22050:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= DAC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_441K << 1);
        REG_WRITE(AUD_DAC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_44_1_K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 44100:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_44_1_K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 12000:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= DAC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_48K << 2);
        REG_WRITE(AUD_DAC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 24000:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= DAC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_48K << 1);
        REG_WRITE(AUD_DAC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 48000:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_48K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 8000:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_8K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 16000:
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_16K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    case 32000:
        reg = REG_READ(AUD_EXTEND_CFG);
        reg |= DAC_FRACMOD_MANUAL;
        REG_WRITE(AUD_EXTEND_CFG, reg);
        reg = (CONST_DIV_16K >> 1);
        REG_WRITE(AUD_DAC_FRACMOD, reg);
        reg = REG_READ(AUDIO_CONFIG);
        reg &= ~(SAMPLE_RATE_DAC_MASK << SAMPLE_RATE_DAC_POSI);
        reg |= ((SAMPLE_RATE_16K & SAMPLE_RATE_DAC_MASK) << SAMPLE_RATE_DAC_POSI);
        REG_WRITE(AUDIO_CONFIG, reg);
        break;

    default:
        AUD_PRT("unsupported sample rate:%d\r\n", sample_rate);
        break;
    }
}

#if CFG_GENERAL_DMA
void audio_dac_dma_handler(UINT32 param)
{

}

static void audio_dac_set_dma(UINT32 enable)
{
    GDMA_CFG_ST en_cfg;

    en_cfg.channel = AUD_DAC_DEF_DMA_CHANNEL;
    if (enable)
        en_cfg.param = 1;
    else
        en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
}

static void audio_dac_eixt_dma(void)
{
    GDMA_CFG_ST en_cfg;

    en_cfg.channel = AUD_DAC_DEF_DMA_CHANNEL;
    en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
}
#endif // CFG_GENERAL_DMA

void audio_dac_volume_use_single_port(void)
{
    UINT32 param;

    param = AUDIO_DAC_VOL_SINGLE_MODE;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_VOLUME_PORT, 
        &param);
}

void audio_dac_volume_diff_port(void)
{
    // default mode is ready diff port
    // if use single port, call audio_dac_volume_use_single_port when initial dac
    UINT32 param;

    param = AUDIO_DAC_VOL_DIFF_MODE;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_VOLUME_PORT, 
        &param);
}

void audio_dac_open_analog_regs(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_OPEN_DAC_ANALOG, NULL);
}

void audio_dac_close_analog_regs(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_CLOSE_DAC_ANALOG, NULL);
}

void audio_dac_set_analog_mute(UINT32 enable)
{
    UINT32 val = 0;
    
    if (enable)
        val = AUDIO_DAC_ANALOG_MUTE;
    else
        val = AUDIO_DAC_ANALOG_UNMUTE;

    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_AUD_DAC_MUTE, &val);
}

#if AUD_USE_EXT_PA
void audio_dac_init_mute_pin(void)
{
    UINT32 param;

    ASSERT(AUD_DAC_MUTE_PIN < GPIONUM);

    param = GPIO_CFG_PARAM(AUD_DAC_MUTE_PIN, GMODE_OUTPUT);
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
}

void audio_dac_eable_mute(UINT32 enable)
{
    UINT32 param;

    if (enable)
    {
        param = GPIO_CFG_PARAM(AUD_DAC_MUTE_PIN, AUD_DAC_MUTE_ENA_LEVEL);
    }
    else
    {
        param = GPIO_CFG_PARAM(AUD_DAC_MUTE_PIN, ~AUD_DAC_MUTE_ENA_LEVEL);
    }
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_OUTPUT, &param);
}
#endif

void audio_dac_set_volume(UINT32 percent)
{
    UINT32 param = percent, idx;
    AUD_VOL_PTR vol;

    if (percent > 99)
        percent = 99;

    idx = percent * AUD_DAC_VOL_TABLE_LEN / 100;

    vol = (AUD_VOL_PTR)&aud_vol_table[idx];

    audio_dac_set_gain(vol->dig_gain);

    param = vol->ana_gain;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_DAC_VOLUME_ANALOG, &param);

    if (percent == 0)
    {
        // no volume, so do mute(disable extral PA)
#if AUD_USE_EXT_PA
        audio_dac_eable_mute(1);
#endif
    }
    else
    {
#if AUD_USE_EXT_PA
        if(audio_dac_is_enable_bit())
            audio_dac_eable_mute(0);
#endif
    }

    AUD_PRT("set dac vol:%d - indx:%d,dig:%d,ana:%02x\r\n", percent, idx, vol->dig_gain, vol->ana_gain);
}
#endif // CFG_USE_AUD_DAC

