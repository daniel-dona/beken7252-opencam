#include <rtthread.h>
#include "vad_core.h"
#include "vad.h"

#define VAD_BIG_ENDIAN          0
#define VAD_UNDERSAMPLED        0

//sum是检测到声音的'大小'，sum2是阈值
int vad(short samples[], int len)
{
#if VAD_BIG_ENDIAN
    unsigned char *p;
    unsigned char temp;
#endif
    long  sum = 0, sum2 = 0;

    for(int i = 0; i < len; i++)
    {
#if VAD_BIG_ENDIAN
        p = &samples[i];
        temp = p[0];
        p[0] = p[1];
        p[1] = temp;
#endif

#if VAD_UNDERSAMPLED
		if(1 == (i & 0x01))
#endif
		{			
	        sum += (samples[i] * samples[i]);
	        sum2 += (POWER_THRESHOLD_VAL);
		}
    }

	rt_kprintf("vad:%ld:%ld s:%d\r\n", sum, sum2, samples[1]);

    if (sum > sum2)
        return FRAME_TYPE_SPEECH;
    else
        return FRAME_TYPE_NOISE;
}
// eof

