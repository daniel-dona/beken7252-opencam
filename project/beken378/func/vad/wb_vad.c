/******************************************************************************
*                         INCLUDE FILES
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <rtthread.h>
#include "wb_vad.h"
#include "wb_vad_c.h"


/******************************************************************************
*                         PRIVATE PROGRAM CODE
******************************************************************************/
/******************************************************************************
*
*     Function     : filter5
*     Purpose      : Fifth-order half-band lowpass/highpass filter pair with
*                    decimation.
*
*******************************************************************************
*/
static void filter5(
    float *in0,    /* i/o : input values; output low-pass part  */
    float *in1,    /* i/o : input values; output high-pass part */
    float data[]   /* i/o : updated filter memory               */
)
{
    float temp0, temp1, temp2;
    temp0 = *in0 - COEFF5_1 * data[0];
    temp1 = data[0] + COEFF5_1 * temp0;
    data[0] = temp0;
    temp0 = *in1 - COEFF5_2 * data[1];
    temp2 = data[1] + COEFF5_2 * temp0;
    data[1] = temp0;
    *in0 = (temp1 + temp2) / 2.0f;
    *in1 = (temp1 - temp2) / 2.0f;
}
/******************************************************************************
*
*     Function     : filter3
*     Purpose      : Third-order half-band lowpass/highpass filter pair with
*                    decimation.
*
*******************************************************************************
*/
static void filter3(
    float *in0,   /* i/o : input values; output low-pass part  */
    float *in1,   /* i/o : input values; output high-pass part */
    float *data   /* i/o : updated filter memory               */
)
{
    float temp1, temp2;
    temp1 = *in1 - COEFF3 * *data;
    temp2 = *data + COEFF3 * temp1;
    *data = temp1;
    *in1 = (*in0 - temp2) / 2.0f;
    *in0 = (*in0 + temp2) / 2.0f;
}
/******************************************************************************
*
*     Function   : level_calculation
*     Purpose    : Calculate signal level in a sub-band. Level is calculated
*                  by summing absolute values of the input data.
*
*                  Because speech coder has a lookahead, signal level calculated
*                  over the lookahead (data[count1 - count2]) is stored (*sub_level)
*                  and added to the level of the next frame. Additionally, group
*                  delay and decimation of the filter bank is taken into the count
*                  for the values of the counters (count1, count2).
*
*******************************************************************************
*/
static float level_calculation( /* return: signal level */
    float data[],     /* i   : signal buffer                                    */
    float *sub_level, /* i   : level calculated at the end of the previous frame*/
    /* o   : level of signal calculated from the last         */
    /*       (count2 - count1) samples                        */
    VAD_I16 count1,     /* i   : number of samples to be counted                  */
    VAD_I16 count2,     /* i   : number of samples to be counted                  */
    VAD_I16 ind_m,      /* i   : step size for the index of the data buffer       */
    VAD_I16 ind_a,      /* i   : starting index of the data buffer                */
    float scale       /* i   : scaling for the level calculation                */
)
{
    double l_temp1, l_temp2;
    float level;
    VAD_I16 i;
    l_temp1 = 0.0;
    for (i = count1; i < count2; i++)
    {
        l_temp1 += fabs(data[ind_m * i + ind_a]);
    }
    l_temp1 *= 2.0f;
    l_temp2 = l_temp1 + *sub_level / scale;
    *sub_level = (float)(l_temp1 * scale);
    for (i = 0; i < count1; i++)
    {
        l_temp2 += 2.0f * fabs(data[ind_m * i + ind_a]);
    }
    level = (float)(l_temp2 * scale);
    return level;
}
/******************************************************************************
*
*     Function     : filter_bank
*     Purpose      : Divide input signal into bands and calculate level of
*                    the signal in each band
*
*******************************************************************************
*/
static void filter_bank(
    VAD_VARS_T *st,   /* i/o : State struct               */
    float in[],   /* i   : input frame                */
    float level[] /* 0   : signal levels at each band */
)
{
    VAD_I16 i;
    float tmp_buf[FRAME_LEN];
	
    /* shift input 1 bit down for safe scaling */
    for (i = 0; i < FRAME_LEN; i++)
    {
        tmp_buf[i] = in[i] / 2.0f;
    }
    /* run the filter bank */
    for (i = 0; i < FRAME_LEN / 2; i++)
    {
        filter5(&tmp_buf[2 * i], &tmp_buf[2 * i + 1], st->a_data5[0]);
    }
    for (i = 0; i < FRAME_LEN / 4; i++)
    {
        filter5(&tmp_buf[4 * i], &tmp_buf[4 * i + 2], st->a_data5[1]);
        filter5(&tmp_buf[4 * i + 1], &tmp_buf[4 * i + 3], st->a_data5[2]);
    }
    for (i = 0; i < FRAME_LEN / 8; i++)
    {
        filter5(&tmp_buf[8 * i], &tmp_buf[8 * i + 4], st->a_data5[3]);
        filter5(&tmp_buf[8 * i + 2], &tmp_buf[8 * i + 6], st->a_data5[4]);
        filter3(&tmp_buf[8 * i + 3], &tmp_buf[8 * i + 7], &st->a_data3[0]);
    }
    for (i = 0; i < FRAME_LEN / 16; i++)
    {
        filter3(&tmp_buf[16 * i + 0], &tmp_buf[16 * i + 8], &st->a_data3[1]);
        filter3(&tmp_buf[16 * i + 4], &tmp_buf[16 * i + 12], &st->a_data3[2]);
        filter3(&tmp_buf[16 * i + 6], &tmp_buf[16 * i + 14], &st->a_data3[3]);
    }
    for (i = 0; i < FRAME_LEN / 32; i++)
    {
        filter3(&tmp_buf[32 * i + 0], &tmp_buf[32 * i + 16], &st->a_data3[4]);
        filter3(&tmp_buf[32 * i + 8], &tmp_buf[32 * i + 24], &st->a_data3[5]);
    }
    /* calculate levels in each frequency band */
    /* 4800 - 6400 Hz*/
    level[11] = level_calculation(tmp_buf, &st->sub_level[11],
                                  FRAME_LEN / 4 - 48, FRAME_LEN / 4, 4, 1, 0.25);
    /* 4000 - 4800 Hz*/
    level[10] = level_calculation(tmp_buf, &st->sub_level[10],
                                  FRAME_LEN / 8 - 24, FRAME_LEN / 8, 8, 7, 0.5);
    /* 3200 - 4000 Hz*/
    level[9] = level_calculation(tmp_buf, &st->sub_level[9],
                                 FRAME_LEN / 8 - 24, FRAME_LEN / 8, 8, 3, 0.5);
    /* 2400 - 3200 Hz*/
    level[8] = level_calculation(tmp_buf, &st->sub_level[8],
                                 FRAME_LEN / 8 - 24, FRAME_LEN / 8, 8, 2, 0.5);
    /* 2000 - 2400 Hz*/
    level[7] = level_calculation(tmp_buf, &st->sub_level[7],
                                 FRAME_LEN / 16 - 12, FRAME_LEN / 16, 16, 14, 1.0);
    /* 1600 - 2000 Hz*/
    level[6] = level_calculation(tmp_buf, &st->sub_level[6],
                                 FRAME_LEN / 16 - 12, FRAME_LEN / 16, 16, 6, 1.0);
    /* 1200 - 1600 Hz*/
    level[5] = level_calculation(tmp_buf, &st->sub_level[5],
                                 FRAME_LEN / 16 - 12, FRAME_LEN / 16, 16, 4, 1.0);
    /* 800 - 1200 Hz*/
    level[4] = level_calculation(tmp_buf, &st->sub_level[4],
                                 FRAME_LEN / 16 - 12, FRAME_LEN / 16, 16, 12, 1.0);
    /* 600 - 800 Hz*/
    level[3] = level_calculation(tmp_buf, &st->sub_level[3],
                                 FRAME_LEN / 32 - 6, FRAME_LEN / 32, 32, 8, 2.0);
    /* 400 - 600 Hz*/
    level[2] = level_calculation(tmp_buf, &st->sub_level[2],
                                 FRAME_LEN / 32 - 6, FRAME_LEN / 32, 32, 24, 2.0);
    /* 200 - 400 Hz*/
    level[1] = level_calculation(tmp_buf, &st->sub_level[1],
                                 FRAME_LEN / 32 - 6, FRAME_LEN / 32, 32, 16, 2.0);
    /* 0 - 200 Hz*/
    level[0] = level_calculation(tmp_buf, &st->sub_level[0],
                   FRAME_LEN/32-6, FRAME_LEN/32, 32, 0, 2.0)/2;  
}
/******************************************************************************
*
*     Function   : update_cntrl
*     Purpose    : Control update of the background noise estimate.
*
*******************************************************************************
*/
static void update_cntrl(
    VAD_VARS_T *st,   /* i/o : State structure                    */
    float level[] /* i   : sub-band levels of the input frame */
)
{
    VAD_I16 i;
    float stat_rat;
    float num, denom;
    float alpha;
    /* if fullband pitch or tone have been detected for a while, initialize stat_count */
    if ((st->pitch_tone & 0x7c00) == 0x7c00)
    {
        st->stat_count = STAT_COUNT;
    }
    else
    {
        /* if 8 last vad-decisions have been "0", reinitialize stat_count */
        if ((st->vad_reg & 0x7f80) == 0)
        {
            st->stat_count = STAT_COUNT;
        }
        else
        {
            stat_rat = 0;
            for (i = 0; i < SUB_BAND_CNT; i++)
            {
                if (level[i] > st->average_level[i])
                {
                    num = level[i];
                    denom = st->average_level[i];
                }
                else
                {
                    num = st->average_level[i];
                    denom = level[i];
                }
                /* Limit nimimum value of num and denom to STAT_THR_LEVEL */
                if (num  < STAT_THR_LEVEL)
                {
                    num = STAT_THR_LEVEL;
                }
                if (denom < STAT_THR_LEVEL)
                {
                    denom = STAT_THR_LEVEL;
                }
                stat_rat += num / denom * 64;
            }
            /* compare stat_rat with a threshold and update stat_count */
            if (stat_rat  > STAT_THR)
            {
                st->stat_count = STAT_COUNT;
            }
            else
            {
                if ((st->vad_reg & 0x4000) != 0)
                {
                    if (st->stat_count != 0)
                    {
                        st->stat_count--;
                    }
                }
            }
        }
    }
    /* Update average amplitude estimate for stationarity estimation */
    alpha = ALPHA4;
    if (st->stat_count == STAT_COUNT)
    {
        alpha = 1.0;
    }
    else if ((st->vad_reg & 0x4000) == 0)
    {
        alpha = ALPHA5;
    }
    for (i = 0; i < SUB_BAND_CNT; i++)
    {
        st->average_level[i] += alpha * (level[i] - st->average_level[i]);
    }
}
/******************************************************************************
*
*     Function     : hangover_addition
*     Purpose      : Add hangover after speech bursts
*
*******************************************************************************
*/
static VAD_I16 hangover_addition( /* return: VAD_flag indicating final VAD decision */
    VAD_VARS_T *st,       /* i/o : State structure                     */
    VAD_I16 low_power,  /* i   : flag power of the input frame    */
    VAD_I16 hang_len,   /* i   : hangover length */
    VAD_I16 burst_len   /* i   : minimum burst length for hangover addition */
)
{
    /* if the input power (pow_sum) is lower than a threshold, clear
       counters and set VAD_flag to "0"  "fast exit"                 */
    if (low_power != 0)
    {
        st->burst_count = 0;
        st->hang_count = 0;
        return 0;
    }
    /* update the counters (hang_count, burst_count) */
    if ((st->vad_reg & 0x4000) != 0)
    {
        st->burst_count++;
        if (st->burst_count >= burst_len)
        {
            st->hang_count = hang_len;
        }
        return 1;
    }
    else
    {
        st->burst_count = 0;
        if (st->hang_count > 0)
        {
            st->hang_count--;
            return 1;
        }
    }
    return 0;
}
/******************************************************************************
*
*     Function   : noise_estimate_update
*     Purpose    : Update of background noise estimate
*
*******************************************************************************
*/
static void noise_estimate_update(
    VAD_VARS_T *st,    /* i/o : State structure                       */
    float level[]   /* i   : sub-band levels of the input frame */
)
{
    VAD_I16 i;
    float alpha_up, alpha_down, bckr_add, temp;
    /* Control update of backgroud_estimate[] */
    update_cntrl(st, level);
    /* Choose update speed */
    bckr_add = 2.0;
    if ((0x7800 & st->vad_reg) == 0)
    {
        alpha_up = ALPHA_UP1;
        alpha_down = ALPHA_DOWN1;
    }
    else
    {
        if (st->stat_count == 0)
        {
            alpha_up = ALPHA_UP2;
            alpha_down = ALPHA_DOWN2;
        }
        else
        {
            alpha_up = 0.0;
            alpha_down = ALPHA3;
            bckr_add = 0.0;
        }
    }
    /* Update noise estimate (backgroud_estimate) */
    for (i = 0; i < SUB_BAND_CNT; i++)
    {
        temp = st->old_level[i] - st->backgroud_estimate[i];
        if (temp < 0.0)
        {
            /* update downwards*/
            st->backgroud_estimate[i] += -2 + (alpha_down * temp);
            /* limit minimum value of the noise estimate to NOISE_MIN */
            if (st->backgroud_estimate[i] < NOISE_MIN)
            {
                st->backgroud_estimate[i] = NOISE_MIN;
            }
        }
        else
        {
            /* update upwards */
            st->backgroud_estimate[i] += bckr_add + (alpha_up * temp);
            /* limit maximum value of the noise estimate to NOISE_MAX */
            if (st->backgroud_estimate[i] > NOISE_MAX)
            {
                st->backgroud_estimate[i] = NOISE_MAX;
            }
        }
    }
    /* Update signal levels of the previous frame (old_level) */
    for(i = 0; i < SUB_BAND_CNT; i++)
    {
        st->old_level[i] = level[i];
    }
}
/******************************************************************************
*
*     Function     : vad_decision
*     Purpose      : Calculates VAD_flag
*
*******************************************************************************
*/
static VAD_I16 vad_decision( /*return value : VAD_flag */
    VAD_VARS_T *st,          /* i/o : State structure                       */
    float level[SUB_BAND_CNT], /* i   : sub-band levels of the input frame */
    double pow_sum         /* i   : power of the input frame           */
)
{
    VAD_I16 i;
    double L_snr_sum;
    double L_temp;
    float vad_thr, temp, noise_level;
    VAD_I16 low_power_flag;
    VAD_I16 hang_len, burst_len;
    float ilog2_speech_level, ilog2_noise_level;
    float temp2;
    /*
       Calculate squared sum of the input levels (level)
       divided by the background noise components (backgroud_estimate).
       */
    L_snr_sum = 0.0;
    for (i = 0; i < SUB_BAND_CNT; i++)
    {
        temp = level[i] / st->backgroud_estimate[i];
        L_snr_sum += temp * temp;
    }
    /* Calculate average level of estimated background noise */
    L_temp = 0.0;
    for (i = 1; i < SUB_BAND_CNT; i++) /* ignore lowest band */
    {
        L_temp += st->backgroud_estimate[i];
    }
    noise_level = (float)(L_temp / 16.0f);
    /*
       if SNR is lower than a threshold (MIN_SPEECH_SNR),
       and increase speech_level
    */
    temp = noise_level * MIN_SPEECH_SNR * 8;
    if (st->speech_level < temp)
    {
        st->speech_level = temp;
    }
    ilog2_noise_level = (float)(-1024.0f * log10(noise_level / 2147483648.0f) / log10(2.0f));
    /*
    If SNR is very poor, speech_level is probably corrupted by noise level. This
    is correctred by subtracting -MIN_SPEECH_SNR*noise_level from speech level
    */
    ilog2_speech_level = (float)(-1024.0f * log10((st->speech_level - temp) / 2147483648.0f) / log10(2.0f));
    /*ilog2_speech_level = ilog2(st->speech_level);*/
    temp = NO_SLOPE * (ilog2_noise_level - NO_P1) + THR_HIGH;
    temp2 = SP_CH_MIN + SP_SLOPE * (ilog2_speech_level - SP_P1);
    if (temp2 < SP_CH_MIN)
    {
        temp2 = SP_CH_MIN;
    }
    if (temp2 > SP_CH_MAX)
    {
        temp2 = SP_CH_MAX;
    }
    vad_thr = temp + temp2;
	//printf("%f %f ", (vad_thr*(float)SUB_BAND_CNT/128.0f), L_snr_sum);
    if (vad_thr < THR_MIN)
    {
        vad_thr = THR_MIN;
    }
    /* Shift VAD decision register */
    st->vad_reg = (short)((st->vad_reg) >> 1);
    /* Make intermediate VAD decision */
    if (L_snr_sum > (vad_thr * (float)SUB_BAND_CNT / 128.0f))
    {
    	//printf("speech ");
        st->vad_reg = (VAD_I16)(st->vad_reg | 0x4000);
    }
	else
	{
		//printf("noise ");
	}
    /* primary vad decsion made */
    /* check if the input power (pow_sum) is lower than a threshold" */
//    printf("power sum=%ld\n", (long int)pow_sum);

    if (pow_sum < VAD_POW_LOW)
    {
        low_power_flag = 1;
    }
    else
    {
        low_power_flag = 0;
    }
    /* Update speech subband background noise estimates */
    noise_estimate_update(st, level);
    hang_len = (VAD_I16)((VAD_I16)(HANG_SLOPE * (vad_thr - HANG_P1) - 0.5) + HANG_HIGH);
    if (hang_len < HANG_LOW)
    {
        hang_len = HANG_LOW;
    };
    burst_len = (VAD_I16)((VAD_I16)(BURST_SLOPE * (vad_thr - BURST_P1) - 0.5) + BURST_HIGH);
    return(hangover_addition(st, low_power_flag, hang_len, burst_len));
}
/******************************************************************************
*
*     estimate_speech()
*     Purpose      : Estimate speech level
*
* Maximum signal level is searched and stored to the variable sp_max.
* The speech frames must locate within SP_EST_COUNT number of frames to be counted.
* Thus, noisy frames having occasional VAD = "1" decisions will not
* affect to the estimated speech_level.
*
*******************************************************************************
*/
static void estimate_speech(
    VAD_VARS_T *st,    /* i/o : State structure    */
    float in_level /* level of the input frame */
)
{
    float alpha, tmp;
	
    /* if the required activity count cannot be achieved, reset counters */
    if (SP_ACTIVITY_COUNT  > (SP_EST_COUNT - st->sp_est_cnt + st->sp_max_cnt))
    {
        st->sp_est_cnt = 0;
        st->sp_max = 0.0;
        st->sp_max_cnt = 0;
    }
    st->sp_est_cnt++;
	
    if (((st->vad_reg & 0x4000) || (in_level > st->speech_level))
            && (in_level > MIN_SPEECH_LEVEL1))
    {
        if (in_level > st->sp_max)
        {
            st->sp_max = in_level;
        }
        st->sp_max_cnt++;
        if (st->sp_max_cnt >= SP_ACTIVITY_COUNT)
        {
            tmp = st->sp_max / 2.0f; /* scale to get "average" speech level*/
            if (tmp > st->speech_level)
            {
                alpha = ALPHA_SP_UP;
            }
            else
            {
                alpha = ALPHA_SP_DOWN;
            }
			
            if (tmp > MIN_SPEECH_LEVEL2)
            {
                st->speech_level += alpha * (tmp - st->speech_level);
            }
			
            st->sp_max = 0.0;
            st->sp_max_cnt = 0;
            st->sp_est_cnt = 0;
        }
    }
}
/******************************************************************************
*                         PUBLIC PROGRAM CODE
******************************************************************************/
/******************************************************************************
*
*  Function:   wb_vad_init
*  Purpose:    Allocates state memory and initializes state memory
*
*******************************************************************************
*/
int wb_vad_init (         /* return: non-zero with error, zero for ok. */
    VAD_VARS_T **state    /* i/o : State structure    */
)
{
    VAD_VARS_T *s;
	
    if (state == (VAD_VARS_T **) NULL)
    {
        rt_kprintf("vad_init: invalid parameter\n");
        return -1;
    }
	
    *state = NULL;
	
    /* allocate memory */
    if ((s = (VAD_VARS_T *) malloc(sizeof(VAD_VARS_T))) == NULL)
    {
        rt_kprintf("vad_init: can not malloc state structure\n");
        return -1;
    }
	
    wb_vad_reset(s);
    *state = s;
	
    return 0;
}
/******************************************************************************
*
*  Function:   wb_vad_reset
*  Purpose:    Initializes state memory to zero
*
*******************************************************************************
*******************************************************************************
*/
int wb_vad_reset ( /* return: non-zero with error, zero for ok. */
    VAD_VARS_T *state  /* i/o : State structure    */
)
{
    VAD_I16 i, j;
	
    if (state == (VAD_VARS_T *) NULL)
    {
        WB_VAD_PRF("vad_reset: invalid parameter\n");
        return -1;
    }
	
    /* Initialize pitch detection variables */
    state->pitch_tone = 0;
    state->vad_reg = 0;
    state->hang_count = 0;
    state->burst_count = 0;
    state->hang_count = 0;
	
    /* initialize memory used by the filter bank */
    for (i = 0; i < F_5TH_CNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            state->a_data5[i][j] = 0.0;
        }
    }
	
    for (i = 0; i < F_3TH_CNT; i++)
    {
        state->a_data3[i] = 0.0;
    }
	
    /* initialize the rest of the memory */
    for (i = 0; i < SUB_BAND_CNT; i++)
    {
        state->backgroud_estimate[i] = NOISE_INIT;
        state->old_level[i] = NOISE_INIT;
        state->average_level[i] = NOISE_INIT;
        state->sub_level[i] = 0;
        state->level[i] = 0.0;
        state->prevLevel[i] = 0.0;
    }
	
    state->sp_est_cnt = 0;
    state->sp_max = 0;
    state->sp_max_cnt = 0;
    state->speech_level = SPEECH_LEVEL_INIT;
    state->prev_pow_sum = 0;
	
    return 0;
}
/******************************************************************************
*
*  Function:   wb_vad_exit
*  Purpose:    The memory used for state memory is freed
*
*******************************************************************************
*******************************************************************************
*/
void wb_vad_exit (
    VAD_VARS_T **state /* i/o : State structure    */
)
{
    if (state == NULL || *state == NULL)
        return;
	
    /* deallocate memory */
    free(*state);	
    *state = NULL;
	
    return;
}
/******************************************************************************
*
*     Function     : wb_vad_tone_detection
*     Purpose      : Set tone flag if pitch gain is high. This is used to detect
*                    signaling tones and other signals with high pitch gain.
*
*******************************************************************************
*/
void wb_vad_pitch_tone_detection (
    VAD_VARS_T *st,  /* i/o : State struct            */
    float p_gain /* pitch gain      */
)
{
    /* update tone flag and pitch flag */
    st->pitch_tone = (VAD_I16)((st->pitch_tone) >> 1);
    /* if (pitch_gain > TONE_THR)
           set tone flag
    */
    if (p_gain > TONE_THR)
    {
        st->pitch_tone = (VAD_I16)(st->pitch_tone | 0x4000);
    }
}
/******************************************************************************
*
*     Function     : wb_vad
*     Purpose      : Main program for Voice Activity Detection (VAD)
*
*******************************************************************************
*/
VAD_I16 wb_vad(          /* Return value : VAD Decision, 1 = speech, 0 = noise */
    VAD_VARS_T *st,      /* i/o : State structure                 */
    float in_buf[]       /* i   : samples of the input frame   */
)
{
    VAD_I16 i;
    float temp;
    VAD_I16 vad_flag;
    double tmp_power, pow_sum;
	
    for(i = 0; i < SUB_BAND_CNT; i++)
    {
        st->prevLevel[i] = st->level[i];
    }
	
    /* Calculate power of the input frame. */
    tmp_power = 0.0;
    for (i = 0; i < FRAME_LEN; i++)
    {
        tmp_power += in_buf[i] * in_buf[i];
    }
	
    tmp_power *= 2.0;
    /* pow_sum = power of current frame and previous frame */
    pow_sum = tmp_power + st->prev_pow_sum;
    /* save power of current frame for next call */
    st->prev_pow_sum = tmp_power;
	
    /* If input power is very low, clear tone flag */
    if (pow_sum < POW_PITCH_TONE_THR)
    {
        st->pitch_tone = (VAD_I16)(st->pitch_tone & 0x1fff);
    }
	
    /*  Run the filter bank and calculate signal levels at each band */
    filter_bank(st, in_buf, st->level);
	
    /* compute VAD decision */
    vad_flag = vad_decision(st, st->level, pow_sum);
	
    /* Calculate input level */
    tmp_power = 0.0;
    for (i = 1; i < SUB_BAND_CNT; i++) /* ignore lowest band */
    {
        tmp_power += st->level[i];
    }
	
    temp = (float)(tmp_power / 16.0f);
    estimate_speech(st, temp); /* Estimate speech level */
	
    return(vad_flag);
}
// eof

