#ifndef __APP_SOUND_MIXER_H__
#define __APP_SOUND_MIXER_H__

#define CFG_MIEXER_STRAIGHT_THROUGH      0

#define SDLY_MALLOC                  sdram_malloc
#define SDLY_FREE                    sdram_free
#define SDLY_MEMSET                  rt_memset
#define SDLY_PRINTF                  rt_kprintf
#define SDLY_LOG(...)

#define SDLY_FOREPART_MUTE_SECTION_COUNT   (40)

#define SDLY_ADC_DECAY_GPIO_PIN           GPIO19
#define SDLY_ADC_MODE                     SARADC_MODE_SINGLESTEP  // SINGLE
#define SDLY_ADC_DIV                      SARADC_CLK_DIV8
#define SDLY_MIC_TAB_LEN                  2304

#define SCAN_CNT_THR_DECAY      	    (20U)
#define SCAN_CNT_THR_MIC     		    (40U)
#define SCAN_CNT_THR_SPTIME     	    (60U)
#define SCAN_CNT_THR_HIGH_FREQ     	    (80U)
#define SCAN_CNT_THR_LOW_FREQ     	    (100U)
#define SCAN_CNT_THR_BAT			    (120U)
#define DEFAULT_DAC_VOL                  16

#define SDLY_NEED_FIFO_SIZE_400MS         (19200u + 1u)   // 48000 * 0.3
#define SDLY_NEED_FIFO_SIZE_350MS         (16800u + 1u)   // 48000 * 0.35
#define SDLY_NEED_FIFO_SIZE_300MS         (14400u + 1u)   // 48000 * 0.3
#define SDLY_NEED_FIFO_SIZE_250MS         (12000u + 1u)   // 48000 * 0.25
#define SDLY_NEED_FIFO_SIZE_200MS         (9600u + 1u)    // 48000 * 0.2
#define SDLY_NEED_FIFO_SIZE_180MS         (8640u + 1u)    // 48000 * 0.18

#define SDLY_NEED_FIFO_SIZE_10MS          (480u + 1u)    // 48000 * 0.01


#define SDLY_NEED_FIFO_SIZE_200MS_8K      (1600u + 1u)    // 48000 * 0.2
#define SDLY_NEED_FIFO_SIZE_180MS_8K      (1440u + 1u)    // 8000 * 0.18
#define SDLY_NEED_FIFO_SIZE_10MS_8K       (80u + 1u)

#define MUSIC_MAX_SAMPLE_RATE_8K          (8000u)

#define SDLY_MAX_FIFO_SIZE_8K             SDLY_NEED_FIFO_SIZE_180MS_8K //SDLY_NEED_FIFO_SIZE_10MS_8K
#define SDLY_MAX_SAMPLE_TIME_MS_8K      ((SDLY_MAX_FIFO_SIZE_8K-1)*SDLY_SAMPLE_TIME_ONE_S/MUSIC_MAX_SAMPLE_RATE_8K)
#define SDLY_DEF_SAMPLE_TIME_MS_8K        SDLY_MAX_SAMPLE_TIME_MS_8K

#define SDLY_MAX_FIFO_SIZE                SDLY_NEED_FIFO_SIZE_180MS//SDLY_NEED_FIFO_SIZE_10MS //SDLY_NEED_FIFO_SIZE_300MS

#define MUSIC_MAX_SAMPLE_RATE             (48000u)
#define SDLY_SAMPLE_TIME_ONE_S            (1000u)

#define SDLY_MIN_SAMPLE_TIME_MS           (6u)   // 48000 * 0.006  = 288
#define SDLY_MAX_SAMPLE_TIME_MS           ((SDLY_MAX_FIFO_SIZE-1)*SDLY_SAMPLE_TIME_ONE_S/MUSIC_MAX_SAMPLE_RATE)
#define SDLY_DEF_SAMPLE_TIME_MS           SDLY_MAX_SAMPLE_TIME_MS
#define SDLY_ADC_MAX_SAMPLE               (1023u)


#define SDLY_DEF_DECAY_VAL                70
#define SDLY_MIN_DECAY_VAL                100
#define SDLY_ADC_MAX_DECAY                1023

#define SDLY_MAX_SAMPLE_VAL               (32767l)
#define SDLY_MIN_SAMPLE_VAL               (-32767l)

#define SDLY_ADC_CHANGE_PARAM_THRE        10

#define SDLY_ADC_MICVOL_GPIO_PIN          5
#define SDLY_ADC_SAMPLE_TIME_CHANNEL      1
#define SDLY_ADC_DECAY_CHANNEL            4
#define SDLY_ADC_PCM_VOL_CHANNEL          4
#define SDLY_ADC_MIC_VOL_CHANNEL          2
#define SDLY_VBAT_CHANNEL			      0

#define BAT_DETECT_NUM		              15

typedef enum
{
    SDLY_STOP,
    SDLY_START,
} SDELAY_STA;

typedef enum
{
    SDLY_OK,
    SDLY_NOMEMORY,
} SDELAY_ERR;

#define ALIGN_32BIT               __attribute__((aligned(4)))
#define SDLY_TEMP_BUFFER_COUNT     (512 * 2)

typedef struct sound_mixer_st
{
    ALIGN_32BIT int16_t fifo[SDLY_MAX_FIFO_SIZE];

    int16_t *op_ptr;

    uint32_t audio_freq;
    uint32_t sample_len;
    uint32_t max_sample_time;
    uint32_t sample_time;

    int32_t decay_value;
    uint32_t sample_time_adc;
    int32_t decay_value_adc;

    SDELAY_STA status;
    uint16_t micphone_vol;
    uint16_t pcm_vol;
    uint8_t mic_high_freq;
    uint8_t mic_low_freq;
    uint16_t padding;
} SDELAY_ST;

extern uint8 sdly_get_idle_mixer_flag(void);
extern void sdly_exit(void);
extern void sdly_start_play(void);
extern void sdly_scanning_dac(void);
extern SDELAY_ERR sdly_config(uint32_t freq);
extern void sdly_process_dac_result(uint16_t channel, uint16_t value);
extern void sdly_mixer_single_playing(uint8_t *, uint16_t );
extern void sdly_mixer_multi_playing(uint8_t *, uint8_t *, uint16_t );
extern uint32 sdly_microphone_data_handler(uint8 *sbc_buf, uint32 buf_len, uint8 mode);
extern void sdly_deinit(void);
extern void sdly_init(void);
extern void sdly_set_sample_vol(uint8 vol);
extern void sdly_set_pcm_vol(uint8 vol);
extern SDELAY_ST *sdly_get_mixer_entity(void);
extern void sdly_forepart_mute(void *sample, uint32_t len);
extern void sdly_init_forepart_mute(void);
extern void sdly_set_micphone_vol(int vol);
#endif
// eof

