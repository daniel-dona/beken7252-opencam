#include "rtthread.h"
#include "optparse.h"

#include <stdlib.h>
#include "player_system.h"
#include "player.h"
#include "audio_codec.h"
#include "audio_stream.h"

#if defined(PLAYER_ENABLE_NET_STREAM) 
#include "netstream.h"
#include "stream_pipe.h"
#include "netstream_buffer.h"
#endif 

#if defined(PLAYER_ENABLE_NET_STREAM)
extern struct stream_pipe * netstream_get_pipe(void);
extern rt_uint32_t rb_buffer_data_len(struct rb_buffer *rb);
#endif 

static struct optparse_long opts[] = 
{
    {"version" , 'V', OPTPARSE_NONE    },     /* 版本 */ 
    {"help"    , 'h', OPTPARSE_NONE    },     /* 帮助 */ 
    {"start"   , 's', OPTPARSE_REQUIRED},     /* 播放 */ 
    {"stop"    , 't', OPTPARSE_NONE    },     /* 停止 */ 
    {"pause"   , 'p', OPTPARSE_NONE    },     /* 暂停 */ 
    {"resume"  , 'r', OPTPARSE_NONE    },     /* 恢复 */ 
    {"seek"    , 'k', OPTPARSE_REQUIRED},     /* 移动 */ 
    {"volume"  , 'v', OPTPARSE_REQUIRED},     /* 音量 */ 
    {"dump"    , 'd', OPTPARSE_NONE    },     /* 信息 */ 
    { NULL     ,  0,  OPTPARSE_NONE    }
}; 

static void usage(void)
{
    rt_kprintf("usage: player [option] [target] ...\n\n"); 
    rt_kprintf("usage options:\n"); 
    rt_kprintf("  -V,     --version                  Print player version message.\n"); 
    rt_kprintf("  -h,     --help                     Print defined help message.\n"); 
    rt_kprintf("  -s URI, --start=URI                Play music with URI(network links or local files).\n");
    rt_kprintf("  -t,     --stop                     Stop playing music.\n");
    rt_kprintf("  -p,     --pause                    Pause the music.\n");
    rt_kprintf("  -r,     --resume                   Resume the music.\n");
    rt_kprintf("  -k sec, --seek=sec                 Seek the specified seconds to play.\n");
    rt_kprintf("  -v lvl, --volume=lvl               Change the volume(0~99).\n");
    rt_kprintf("  -d,     --dump                     Dump play relevant information.\n");
}

#if defined(PLAYER_ENABLE_NET_STREAM)
int stream_buffer(int argc, char **argv)
{
	struct stream_pipe *pipe = netstream_get_pipe();

    rt_kprintf("read_mirror : %d read_index : %d\n", pipe->ringbuffer.read_mirror, pipe->ringbuffer.read_index);
    rt_kprintf("write_mirror : %d write_index : %d\n", pipe->ringbuffer.write_mirror, pipe->ringbuffer.write_index);
    rt_kprintf("buffer_size : %d\n", pipe->ringbuffer.buffer_size);

    return 0;
}

int stream_pipe_dump(void)
{
    rt_uint32_t total_size, used_size, remain_size;
	struct stream_pipe *pipe = netstream_get_pipe();

    total_size = pipe->ringbuffer.buffer_size;
    used_size = rb_buffer_data_len(&pipe->ringbuffer);
    remain_size = total_size - used_size;

    rt_kprintf("\nPlayer NetCache:\n"); 
    rt_kprintf("total size   - %d \n", total_size);
    rt_kprintf("used size    - %d \n", used_size);
    // rt_kprintf("remain size  - %d \n", remain_size);
    // rt_kprintf("read_mirror  - %d \n", pipe->ringbuffer.read_mirror);
    // rt_kprintf("read_index   - %d \n", pipe->ringbuffer.read_index);
    // rt_kprintf("write_mirror - %d \n", pipe->ringbuffer.write_mirror); 
    // rt_kprintf("write_index  - %d \n", pipe->ringbuffer.write_index);
    rt_kprintf("ready_wm     - %d \n", pipe->reader_ready_wm); 
    rt_kprintf("resume_wm    - %d \n", pipe->writer_resume_wm);

    return 0;
}
#endif 

static void dump_status(void)
{
    const char *state[] =
    {
        "STOPPED",
        "PLAYING",
        "PAUSED" 
    };

    rt_kprintf("\nPlayer Dump Status:\n");
    rt_kprintf("status   - %s\n", state[player_get_state()]);
    rt_kprintf("URI      - %s\n", (player_get_uri() != NULL) ? player_get_uri() : "NULL");
    rt_kprintf("volume   - %d\n", player_get_volume()); 

    if (player_get_state() != PLAYER_STAT_STOPPED)
    {
        int value;

        value = player_get_duration();
        rt_kprintf("duration - %02d:%02d\n", value/60, value%60);

        value = player_get_position() / 1000;
        rt_kprintf("position - %02d:%02d\n", value/60, value%60);
    }

#if defined(PLAYER_ENABLE_NET_STREAM) 
    stream_pipe_dump(); 
#endif 
}

int player(int argc, char **argv)
{
    int ch; 
    int option_index; 
    struct optparse options;

    rt_bool_t  help    = RT_FALSE; 
    rt_bool_t  start   = RT_FALSE; 
    rt_bool_t  stop    = RT_FALSE; 
    rt_bool_t  pause   = RT_FALSE; 
    rt_bool_t  resume  = RT_FALSE; 
    rt_bool_t  seek    = RT_FALSE; 
    rt_int32_t second  = (-1); 
    rt_int8_t  volume  = (-1); 
    rt_bool_t  dump    = RT_FALSE; 
    rt_bool_t  version = RT_FALSE; 

    rt_uint8_t action_cnt = 0; 

    char *uri = RT_NULL; 

    if(argc == 1)
    {
        usage(); 
        return RT_EOK; 
    }
    
    /* Parse cmd */ 
    optparse_init(&options, argv); 
    while((ch = optparse_long(&options, opts, &option_index)) != -1)
    {
        switch(ch)
        {
        case 'h':   /* 帮助 */ 
            help = RT_TRUE; 
            break; 

        case 's':   /* 播放 */ 
            start = RT_TRUE; 
            uri = (options.optarg == RT_NULL) ? (RT_NULL) : rt_strdup(options.optarg); 
            action_cnt++; 
            break; 

        case 't':   /* 停止 */ 
            stop = RT_TRUE; 
            action_cnt++; 
            break; 

        case 'p':   /* 暂停 */ 
            pause = RT_TRUE; 
            action_cnt++; 
            break; 

        case 'r':   /* 恢复 */ 
            resume = RT_TRUE; 
            action_cnt++; 
            break; 

        case 'k':   /* 移动 */ 
            seek = RT_TRUE; 
            second = (options.optarg == RT_NULL) ? (-1) : atoi(options.optarg); 
            action_cnt++; 
            break; 

        case 'v':   /* 音量 */ 
            volume = (options.optarg == RT_NULL) ? (-1) : atoi(options.optarg);
            break; 

        case 'd':   /* 信息 */ 
            dump = RT_TRUE; 
            break; 

        case 'V':   /* 版本 */ 
            version = RT_TRUE; 
            break; 
        }
    }

    // 判断 播放 暂停 停止 恢复 移动 命令是否多次使用 不能共存使用
    if(action_cnt > 1)
    {
        rt_kprintf("START STOP PAUSE RESUME SEEK parameter can't be used at the same time.\n"); 
        return RT_EINVAL; 
    }

    if(help == RT_TRUE)
    {
        usage(); 
        return RT_EOK;
    }

    // 播放器动作
    if((start == RT_TRUE) && (uri != RT_NULL))
    {
        rt_kprintf("//////////////////////////// player_play \n"); 
        player_stop(); 
        player_set_uri(uri); 
        player_play(); 
        rt_kprintf("//////////////////////////// player_play end \n"); 
        
        rt_free(uri); 
    }
    else if(stop == RT_TRUE)
    {
        rt_kprintf("//////////////////////////// player_stop \n"); 
        player_stop(); 
        rt_kprintf("//////////////////////////// player_stop end \n"); 
        rt_kprintf("stop play.\n"); 
    }
    else if(pause == RT_TRUE)
    {
        rt_kprintf("//////////////////////////// player_pause \n"); 
        player_pause(); 
        rt_kprintf("//////////////////////////// player_pause end \n"); 
        rt_kprintf("pause play.\n"); 
    }
    else if(resume == RT_TRUE)
    {
        rt_kprintf("//////////////////////////// player_play(resume) \n"); 
        player_play(); 
        rt_kprintf("//////////////////////////// player_play(resume) end \n"); 
        rt_kprintf("resume play.\n"); 
    }
    else if((seek == RT_TRUE) && (second != (-1)))
    {
        rt_kprintf("//////////////////////////// player_do_seek \n"); 
        player_do_seek(second); 
        rt_kprintf("//////////////////////////// player_do_seek end \n"); 
        rt_kprintf("seek %dsec.\n", second); 
    }

    if(volume != (-1))
    {
        if((volume < 0) || (volume > 99))
        {
            rt_kprintf("set volume failed. volume needs to be set to 0~99.\n", volume); 
        }
        else
        {
            player_set_volume(volume); 
            rt_kprintf("set play volume %d%%.\n", volume); 
        }
    }

    if(dump == RT_TRUE)
    {
        dump_status(); 
    }

    if(version == RT_TRUE)
    {
        player_get_version(); 
    }
    
    return RT_EOK; 
}
MSH_CMD_EXPORT(player, player func test cmd.); 
