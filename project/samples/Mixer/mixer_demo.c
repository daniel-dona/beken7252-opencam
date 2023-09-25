
#include "rtconfig.h"

#if CONFIG_SOUND_MIXER
#include "mixer.h"

void mixer_set_value(int argc, char** argv)
{
    int val;
    
    val = atoi(argv[1]);
    
    if(val == 1) {
		rt_kprintf("mixer_set_value:%d pause\r\n", val);
        mixer_pause();
    } else if(val == 0) {
		rt_kprintf("mixer_set_value:%d replay\r\n", val);
        mixer_replay();
	}	
}

MSH_CMD_EXPORT(mixer_set_value, mixer_set_value test);
#endif