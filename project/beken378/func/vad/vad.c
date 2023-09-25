#include <string.h>
#include "stdlib.h"

#include "wb_vad.h"

#include "vad_core.h"
#include "vad.h"
#include "rtdebug.h"
#include "rtthread.h"

#if 1
static int vad_speech_count;
static int vad_noise_count;
static int vad_start_flag;

void vad_pre_start(void)
{
    vad_speech_count = 0;
    vad_noise_count = 0;
    vad_start_flag = 0;
	
	rt_kprintf("vad_pre_start\r\n");
}

int vad_estimate(void)
{
    int ret = VAD_OCCUR_NULL;
	
//	VAD_PRTF("vad_estimate0:%d:%d:%d\r\n", vad_start_flag, vad_noise_count, vad_speech_count);

    if(vad_start_flag)
    {
        if(vad_speech_count)
        {
            vad_noise_count = 0;
			vad_speech_count = 0;
        }

        if(vad_noise_count >= TAIL_NOISE_COUNT)
        {
            ret = VAD_OCCUR_DONE;
        }
    }
    else
    {
        if(vad_noise_count)
        {
            vad_speech_count = 0;
            vad_noise_count = 0;
        }

        if(vad_speech_count >= HEAD_SPEECH_COUNT)
        {
            vad_start_flag ++;
			VAD_PRTF("vad_start_flag\r\n");
			vad_noise_count = 0;
        }
    }
//	VAD_PRTF("vad_estimate1:%d:%d:%d\r\n", vad_start_flag, vad_noise_count, vad_speech_count);

    return ret;
}

int vad_entry(short samples[], int len)
{
    int type;

    type = vad(samples, len);
    if(FRAME_TYPE_SPEECH == type)
    {
        vad_speech_count ++;
    }
    else
    {
        vad_noise_count ++;
    }

    return vad_estimate();
}
#endif

#if 2
static int wb_vad_speech_count;
static int wb_vad_noise_count;
static int wb_vad_start_flag;

static VAD_VARS_T *vad_state_ptr = NULL;
static float *wb_in_data = NULL;
static char *wb_buffer = NULL;
static int wb_buffer_len = 0;
static int wb_handle_frame_count = 0;

int wb_vad_enter(void) 
{
	int ret, val;
	int in_len;
	int buf_len;
	char *temp;

	RT_ASSERT(0 == vad_state_ptr);
	RT_ASSERT(0 == wb_in_data);

	val = VAD_RET_SUCCESS;
	
	ret = wb_vad_init(&vad_state_ptr);
	if(0 != ret)
	{
		val = VAD_RET_FAILURE;
		goto wb_exit;
	}

	in_len = WB_FRAME_LEN * sizeof(wb_in_data[0]);
	buf_len = WB_FRAME_LEN * sizeof(wb_buffer[0]);

	temp = (char *)calloc(in_len + buf_len, 1);
	RT_ASSERT(temp);
	
	if(0 == temp)
	{
		val = VAD_RET_FAILURE;
		
		wb_vad_exit(&vad_state_ptr);
		goto wb_exit;
	}

	wb_in_data = (float *)temp;
	wb_buffer = (char *)((int)temp + in_len);
	wb_buffer_len = 0;
	
	wb_handle_frame_count = 0;

	wb_estimate_init();
	
wb_exit:
	return val;
}

int wb_vad_handler(char * buffer)
{
	int type, temp;
	
	RT_ASSERT(wb_in_data);
	
	for(int i = 0; i < WB_FRAME_LEN/2; i++)
	{
		wb_in_data[i] = 0;
		
		temp = 0;
		memcpy((void *)&temp, buffer + 2 * i, 2);
		wb_in_data[i] = (short)temp;
		
		if (wb_in_data[i] > 65535 / 2)
		{
			wb_in_data[i] = wb_in_data[i] - 65536;
		}
	}
	
    type = wb_vad(vad_state_ptr, wb_in_data);
	
	return type;
}

void wb_frame_count_record(int frame_type)
{
	if(WB_FRAME_TYPE_SPEECH == frame_type)
	{
		wb_vad_speech_count ++;
	}
	else if(WB_FRAME_TYPE_NOISE == frame_type)
	{
		wb_vad_noise_count ++;
	}
}

void wb_estimate_init(void)
{
	wb_vad_speech_count = 0;
	wb_vad_noise_count = 0;
	wb_vad_start_flag = 0;
}

int wb_vad_estimate(void)
{
	int ret = VAD_OCCUR_NULL;
	
//	VAD_PRTF("vad_estimate0:%d:%d:%d\r\n", wb_vad_start_flag, wb_vad_noise_count, wb_vad_speech_count);

	if(wb_vad_start_flag)
	{
		if(wb_vad_speech_count)
		{
			wb_vad_noise_count = 0;
			wb_vad_speech_count = 0;
		}

		if(wb_vad_noise_count >= TAIL_NOISE_COUNT)
		{
			ret = VAD_OCCUR_DONE;
		}
	}
	else
	{
		if(wb_vad_noise_count)
		{
			wb_vad_speech_count = 0;
			wb_vad_noise_count = 0;
		}

		if(wb_vad_speech_count >= HEAD_SPEECH_COUNT)
		{
			wb_vad_start_flag ++;
			VAD_PRTF("vad_start_flag\r\n");
			wb_vad_noise_count = 0;
		}
	}
//	VAD_PRTF("vad_estimate1:%d:%d:%d\r\n", wb_vad_start_flag, wb_vad_noise_count, wb_vad_speech_count);

	return ret;
}

int wb_vad_get_frame_len(void)
{
	return WB_FRAME_LEN;
}

int wb_vad_entry(char *buffer, int len)
{
	int ret;
	int frm_type;
	int backup_count;
	int vad_flag = 0;
	int head_used_count;

	frm_type = WB_FRAME_TYPE_NULL;
	if(wb_buffer_len + len >= WB_FRAME_LEN)
	{
		head_used_count = (WB_FRAME_LEN - wb_buffer_len);
		memcpy(&wb_buffer[wb_buffer_len], buffer, head_used_count);

		wb_handle_frame_count ++;
		if(wb_handle_frame_count & 0x01)
		{			
			ret = wb_vad_handler(wb_buffer);
			if(ret)
			{
				frm_type = WB_FRAME_TYPE_SPEECH;
			}
			else
			{
				frm_type = WB_FRAME_TYPE_NOISE;
			}
		}

		backup_count = len - head_used_count;
		if(backup_count)
		{
			memcpy(wb_buffer, &buffer[head_used_count], backup_count);
		}
		wb_buffer_len = len - head_used_count;
	}
	else
	{
		memcpy(&wb_buffer[wb_buffer_len], buffer, len);
		wb_buffer_len += len;
	}

	if(WB_FRAME_TYPE_NULL != frm_type)
	{
		wb_frame_count_record(frm_type);
		vad_flag = wb_vad_estimate();
	}
	
	return vad_flag;
}

void wb_vad_deinit(void)
{
	wb_vad_exit(&vad_state_ptr);
	vad_state_ptr = NULL;

	free(wb_in_data);
	wb_in_data = NULL;
	wb_buffer = NULL;
	
	wb_buffer_len = 0;

	wb_estimate_init();
}
#endif

// eof

