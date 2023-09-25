#include "rtthread.h"
#include "optparse.h"
#include "finsh.h"
#include <stdlib.h>

#include "player.h"
#include "player_app.h"
#include "list_player.h"
#include "player_system.h"
#include "player.h"
#include "audio_codec.h"
#include "audio_stream.h"
#include "netstream.h"
#include "stream_pipe.h"
#include "netstream_buffer.h"

#define OPTS_INVALID(n) (200+(n))

static music_list_t music_list; 
static rt_bool_t enable = RT_FALSE; 

static struct optparse_long opts[] = 
{
    {"help"  , 'h', OPTPARSE_NONE    },     // 命令行帮助信息
    {"dump"  , 'u', OPTPARSE_OPTIONAL},     // dump列表播放器/指定歌曲信息
    {"add"   , 'a', OPTPARSE_NONE    },     // 添加歌曲, 支持歌曲名称/歌曲资源链接/歌曲编号 
    {"del"   , 'd', OPTPARSE_OPTIONAL},     // 删除歌曲
    {"play"  , 'p', OPTPARSE_NONE    },     // 按照设置模式播放歌单/播放指定名称编号和资源链接的歌曲 
    {"stop"  , 't', OPTPARSE_NONE    },     // 停止播放歌单 
    {"pause" , 'e', OPTPARSE_NONE    },     // 暂停播放歌单
    {"resume", 'r', OPTPARSE_NONE    },     // 恢复播放歌单
    {"prev"  , 'v', OPTPARSE_NONE    },     // 播放上一首
    {"next"  , 'n', OPTPARSE_NONE    },     // 播放下一首
    {"mode"  , 'm', OPTPARSE_OPTIONAL},     // 设置/打印列表播放器模式

    {"name"  , OPTS_INVALID(0), OPTPARSE_REQUIRED},     // 指定动作的歌曲名称, 部分不需要该选项
    {"uri"   , OPTS_INVALID(1), OPTPARSE_REQUIRED},     // 指定动作的歌资源链接, 部分不需要该选项
    {"index" , OPTS_INVALID(2), OPTPARSE_REQUIRED},     // 指定动作的歌单中编号, 部分不需要该选项

    { NULL   , 0,  OPTPARSE_NONE    }
}; 

static void listplayer_help(int argc, char **argv)
{
    rt_kprintf("usage: %s [option] [target] ...\n\n", argv[0]); 
    rt_kprintf("%s options:\n", argv[0]); 
    rt_kprintf("  -h,      --help                     Print defined help message.\n"); 
    rt_kprintf("  -u,      --dump                     Dump playlist or music info.\n"); 
    rt_kprintf("  -a,      --add                      Add music to list player, need to specify music name uri or index.\n"); 
    rt_kprintf("  -d,      --del                      Delete playlist music.\n"); 
    rt_kprintf("  -t,      --stop                     Stop the list player.\n"); 
    rt_kprintf("  -e,      --pause                    Pause the list player.\n"); 
    rt_kprintf("  -r,      --resume                   Resume the list player.\n"); 
    rt_kprintf("  -v,      --prev                     Play the prev song.\n"); 
    rt_kprintf("  -n,      --next                     Play the next song.\n"); 
    rt_kprintf("  -m MODE, --mode=MODE                Set list player play mode.\n\n"); 

    rt_kprintf("  --name=MODE                         Describe the name of the music.\n"); 
    rt_kprintf("  --uri=URI                           Describe the URI of the music.\n"); 
    rt_kprintf("  --index=INDEX                       Describe the number of music in the play list.\n"); 
}

int listplayer(int argc, char **argv)
{
    int ch; 
    int option_index; 
    struct optparse options;

    struct music_item item; 
    rt_int32_t idx = -2; 
    rt_uint8_t cnt =  0; 

    rt_bool_t help = RT_FALSE; 
    rt_bool_t dump = RT_FALSE; 
    char     *mode = RT_NULL; 

    // 播放器动作
    rt_uint8_t action = 0; 

    if(argc == 1)
    {
        listplayer_help(argc, argv); 
        return RT_EOK; 
    }

    rt_memset(&item, 0x00, sizeof(struct music_item)); 

    /* Parse cmd */ 
    optparse_init(&options, argv); 
    while((ch = optparse_long(&options, opts, &option_index)) != -1)
    {
        switch(ch)
        {
        case 'h':             // 命令行帮助信息
            help = RT_TRUE; 
            break; 

        case 'u':             // dump列表播放器/指定歌曲信息
            dump = RT_TRUE; 
            break; 

        case 'a':             // 添加歌曲, 支持歌曲名称/歌曲资源链接/歌曲编号
        case 'd':             // 删除歌曲
        case 'p':             // 按照设置模式播放歌单/播放指定名称编号和资源链接的歌曲 
        case 't':             // 停止播放歌单
        case 'e':             // 暂停播放歌单
        case 'r':             // 恢复播放歌单
        case 'v':             // 播放上一首
        case 'n':             // 播放下一首 
            action = options.optopt; 
            cnt++; 
            break; 

        case 'm':             // 设置/打印列表播放器模式
            action = options.optopt; 
            mode = (options.optarg == RT_NULL) ? (RT_NULL) : rt_strdup(options.optarg); 
            cnt++; 
            break; 

        case OPTS_INVALID(0): // 指定动作的歌曲名称, 部分不需要该选项
            item.name = (options.optarg == RT_NULL) ? (RT_NULL) : rt_strdup(options.optarg); 
            break; 
            
        case OPTS_INVALID(1): // 指定动作的歌资源链接, 部分不需要该选项
            item.URL = (options.optarg == RT_NULL) ? (RT_NULL) : rt_strdup(options.optarg); 
            break; 

        case OPTS_INVALID(2): // 指定动作的歌单中编号, 部分不需要该选项
            idx = (options.optarg == RT_NULL) ? (-2) : atoi(options.optarg); 
            break; 
        }
    }

    // 判断 播放 暂停 停止 恢复 命令是否多次使用 不能共存使用
    if(cnt > 1)
    {
        rt_kprintf("[Add/Del/Play/Stop/Pause/Resume/Prev/Next/Mode] action can't be used at the same time.\n"); 
        goto __err; 
    }

    // 命令行帮助信息
    if(help == RT_TRUE)
    {
        listplayer_help(argc, argv); 
    }

    // dump当前播放器信息
    if(dump == RT_TRUE)
    {
        list_player_dump(list_player_current_items()); 
    }

    // 执行对应动作 
    switch(action)
    {
    case 'a':
        if((item.name != RT_NULL) || (item.URL != RT_NULL))
        {
            list_player_item_add(music_list, &item, (idx == -2) ? 0:idx); 
        }
        break; 

    case 'd':
        if((idx == -2) && (item.name == RT_NULL) && (item.URL == RT_NULL)) // listplayer --del
        {
            if(list_player_is_exist() == 1)
            {
                list_player_empty(); 
                rt_kprintf("Empty current play list.\n"); 
            }
            else
            {
                rt_kprintf("No play list is playing, cannot be empty.\n"); 
            }
        }
        else if((item.name != RT_NULL) || (item.URL != RT_NULL)) 
        {
            if(list_player_is_exist() == 1)
            {
                list_player_item_del(music_list, &item); 
                rt_kprintf("Empty current play list.\n"); 
            }
            else
            {
                rt_kprintf("No play list is playing, cannot be del item music.\n"); 
            }
        }
        else if((idx == -1) || (idx > 0))
        {
            if(list_player_is_exist() == 1)
            {
                list_player_item_del_by_index(music_list, idx); 
                rt_kprintf("Delete music from play list succeed.\n"); 
            }
            else
            {
                rt_kprintf("No play list is playing, cannot be del index music.\n"); 
            }
        }
        break; 

    case 'p':
        if((idx == -2) && (item.name == RT_NULL) && (item.URL == RT_NULL))      // listplayer --play
        {
            
            rt_kprintf("Start play list.\n"); 
            list_player_play(music_list); 
        }

        // listplayer --del
        // listplayer --del --name=xxx
        // listplayer --del --index=n
        // listplayer --del --uri=http://www.xxx.com/1.mp3
        else if((item.name != RT_NULL) || (item.URL != RT_NULL))
        {
            if(list_player_is_exist() == 1)
            {
                list_player_item(&item);
                rt_kprintf("Start Play specific name or URL music.\n"); 
            }
            else
            {
                rt_kprintf("You need to play the play list first.\n"); 
            }
        }
        else if((idx == -1) || (idx > 0))
        {
            if(list_player_is_exist() == 1)
            {
                list_player_num(idx); 
                rt_kprintf("Start Play specific index music.\n"); 
            }
            else
            {
                rt_kprintf("You need to play the play list first.\n"); 
            }
        }
        break; 

    case 't':   // 停止
        if(list_player_is_exist() == 1)
        {
            list_player_stop(); 
            rt_kprintf("Stop list player.\n"); 
        }
        else
        {
            rt_kprintf("No play list is playing, cannot be stop music.\n"); 
        }
        break; 

    case 'e':   // 暂停
        if(list_player_is_exist() == 1)
        {
            list_player_pause(); 
            rt_kprintf("Stop list player.\n"); 
        }
        else
        {
            rt_kprintf("No play list is playing, cannot be pause music.\n"); 
        }
        break; 

    case 'r':   // 恢复
        list_player_resume(); 
        rt_kprintf("You need to play the play list first.\n"); 
        break; 

    case 'v':   // 上一首
        if(list_player_is_exist() == 1)
        {
            list_player_prev(); 
            rt_kprintf("Play prev music.\n"); 
        }
        else
        {
            rt_kprintf("No play list is playing, cannot be play prev music.\n"); 
        }
        break; 

    case 'n':   // 下一首
        if(list_player_is_exist() == 1)
        {
            list_player_next(); 
            rt_kprintf("Play next music.\n"); 
        }
        else
        {
            rt_kprintf("No play list is playing, cannot be play next music.\n"); 
        }
        break; 

    case 'm':   // 设置模式
        if(rt_strcmp(mode, "once-list") == 0)
        {
            list_player_mode_set(LISTER_LIST_ONCE_MODE); 
        }
        else if(rt_strcmp(mode, "once-song") == 0)
        {
            list_player_mode_set(LISTER_SONG_ONCE_MODE); 
        }
        else if(rt_strcmp(mode, "repeat-list") == 0)
        {
            list_player_mode_set(LISTER_LIST_REPEAT_MODE); 
        }
        else if(rt_strcmp(mode, "repeat-song") == 0)
        {
            list_player_mode_set(LISTER_SONG_REPEAT_MODE); 
        }
        else
        {
            rt_kprintf("Set play %s mode failed.\n", mode); 
            break; 
        }

        rt_kprintf("Set play %s mode.\n", mode); 
        break; 
    }

__err:
    if(item.URL != RT_NULL) 
    {
        rt_free(item.URL); 
        item.URL = RT_NULL; 
    }

    if(item.name != RT_NULL) 
    {
        rt_free(item.name); 
        item.name = RT_NULL; 
    }

    if(mode != RT_NULL) 
    {
        rt_free(mode); 
        mode = RT_NULL; 
    }

    return RT_EOK; 
}
MSH_CMD_EXPORT(listplayer, list_player_xxx api func test cmd.); 
MSH_CMD_EXPORT_ALIAS(listplayer, lp, list_player_xxx api func test cmd.); // 导出简短命令

// List play callback function
static void list_event_handle(music_list_t table, int event, void *arg)
{
    switch(event)
    {
    case LISTER_BEGIN_OF_ITEM: 
        rt_kprintf("[OS Tick: %.8d] Trigger \"%s\" event.\n", rt_tick_get(), "LISTER_BEGIN_OF_ITEM"); 
        break; 

    case LISTER_END_OF_ITEM: 
        rt_kprintf("[OS Tick: %.8d] Trigger \"%s\" event.\n", rt_tick_get(), "LISTER_END_OF_ITEM"); 
        break; 

    case LISTER_BEGIN_OF_LIST: 
        rt_kprintf("[OS Tick: %.8d] Trigger \"%s\" event.\n", rt_tick_get(), "LISTER_BEGIN_OF_LIST"); 
        break; 

    case LISTER_END_OF_LIST: 
        rt_kprintf("[OS Tick: %.8d] Trigger \"%s\" event.\n", rt_tick_get(), "LISTER_END_OF_LIST"); 
        break; 
    }
}

int listplayer_init(void)
{
    struct music_item item; 

    // 创建默认歌单: 
    if(enable == RT_FALSE)
    {
        rt_memset(&item, 0x00, sizeof(struct music_item)); 

        music_list = list_player_items_create(); 

#if 1   // 网络歌曲, 需要打开网络功能 
        item.name = ("chengdu");
        item.URL  = ("http://music.163.com/song/media/outer/url?id=436514312.mp3");
        list_player_item_add(music_list, &item, 0); 

        item.name = ("SouthernGirl");
        item.URL  = ("http://music.163.com/song/media/outer/url?id=202373.mp3");
        list_player_item_add(music_list, &item, 0); 

        item.name = ("willing");
        item.URL  = ("http://music.163.com/song/media/outer/url?id=554191378.mp3");
        list_player_item_add(music_list, &item, 0); 

        item.name = ("Diao");
        item.URL  = ("http://music.163.com/song/media/outer/url?id=447925059.mp3");
        list_player_item_add(music_list, &item, 0); 

#else   // 本地歌曲, 需要本地文件系统中有对于歌曲
        item.name = ("7.mp3");
        item.URL  = ("/mnt/sd/music/7.mp3");
        list_player_item_add(music_list, &item, -1); 

        item.name = ("2.mp3");
        item.URL  = ("/mnt/sd/music/2.mp3");
        list_player_item_add(music_list, &item, -1); 

        item.name = ("3.mp3");
        item.URL  = ("/mnt/sd/music/3.mp3");
        list_player_item_add(music_list, &item, -1); 

        item.name = ("4.mp3");
        item.URL  = ("/mnt/sd/music/4.mp3");
        list_player_item_add(music_list, &item, -1); 

        item.name = ("5.mp3");
        item.URL  = ("/mnt/sd/music/5.mp3");
        list_player_item_add(music_list, &item, -1); 
#endif

        rt_kprintf("create list player succeed.\n"); 

        // set list play callback function
        list_player_set_table_handler(music_list, list_event_handle, RT_NULL); 
        list_player_mode_set(LISTER_LIST_ONCE_MODE); 
        
        enable = RT_FALSE; 
    }

    return RT_EOK; 
}
INIT_APP_EXPORT(listplayer_init); 

int _switch(int argc, char *argv[])
{
    int index = atoi(argv[1]);
    int sec = atoi(argv[2]);
    
    rt_kprintf("Index: %d.\n", index); 
    rt_kprintf("Sec: %d.\n", sec); 
    
    list_player_switch(music_list, index, sec * 1000, PLAYER_STAT_PLAYING); 
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(_switch, switch, switch); 
