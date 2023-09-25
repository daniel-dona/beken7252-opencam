/*
 * File      : list_player.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-01     liu2guang    Perfect the description and format.
 */

#ifndef __LIST_PLAYER_INCLUDE__
#define __LIST_PLAYER_INCLUDE__

#ifdef __cplusplus
extern "C"
{
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include <player_app.h>

/* list play mode */
#define LISTER_NONE_MODE        (0x00) 
#define LISTER_LIST_ONCE_MODE   (0x01)
#define LISTER_SONG_ONCE_MODE   (0x02)
#define LISTER_LIST_REPEAT_MODE (0x03)
#define LISTER_SONG_REPEAT_MODE (0x04)

/* list player callback event */
#define LISTER_BEGIN_OF_ITEM    (0x01)
#define LISTER_END_OF_ITEM      (0x02) 
#define LISTER_NEXT_ITEM        (0x04) 
#define LISTER_PREV_ITEM        (0x08)
#define LISTER_BEGIN_OF_LIST    (0x10)
#define LISTER_END_OF_LIST      (0x20) 

struct music_item; 
struct music_info; 
struct music_list; 
struct player_list; 

typedef struct music_item *music_item_t; 
typedef struct music_list *music_list_t; 

/* list player callback function */
typedef void (*list_event_handler)(music_list_t table, int event, void *arg); 

/* music item */  
struct music_item
{
    char *name;
    char *URL;
};

struct music_info
{
    rt_list_t node;
    struct music_item item;
};

/* music list */ 
struct music_list
{
    rt_list_t list; 

    int item_num;                       /* 歌单中歌曲数目 */
    int item_position;                  /* 单曲的播放进度 */
    struct rt_mutex mutex;              /* 内部数据, 用户无需使用 */
    struct music_info *curr_info;       /* 当前正在播放的歌曲 */

    list_event_handler event_handler;   /* 播放列表回调函数 */
    void *user_data;                    /* 播放列表回调函数传入的参数 */
};

struct player_list
{
    struct player_app parent; 

    music_list_t table;                 /* 当前关联的歌单 */ 

    struct rt_work work; 
    int status;  
    int event;                          /* 列表播放器事件 */
    int player_mode;                    /* 列表播放器播放模式 */

    struct rt_mutex lock;  
    struct rt_mutex owner; 
};

/**
 * Initialization list player 
 *
 * @return  -RT_ENOMEM: no memory
 *           RT_EOK   : initialization successful, or repeat initialization
 * 
 * @note: Repeat calls are allowed, but only the first time will you 
 *        initialize the list player
 */
int list_player_init(void); 

/**
 * Returns the 'music_item_t' handle of the music being played
 * 
 * @return == RT_NULL: 1. list player not initialized 
 *                     2. list player does not associate music list
 *                     3. the current playlist never plays music
 *         != RT_NULL: the handle of the music being played
 */
music_item_t list_player_current_item(void); 

/**
 * Return to the index of the music being played
 * 
 * @return index: 
 * @return >= 0: the index of the music being played
 *           -1: No music found
 */
int list_player_current_index(void); 

/**
 * The status of the current list player(added with v1.2.0)
 * 
 * @return   0: PLAYER_STAT_STOPPED  @see "enum PLAYER_STAT" 
 *           1: PLAYER_STAT_PLAYING  @see "enum PLAYER_STAT" 
 *           2: PLAYER_STAT_PAUSED   @see "enum PLAYER_STAT" 
 * 
 * @note: The "list_player_current_state" API name changes during the v1.2.0 release!
 */
int list_player_current_state(void); 

/**
 * Returns the 'music_list_t' handle of the music list being played(added with v1.2.0)
 * 
 * @return == RT_NULL: 1. list player not initialized 
 *                     2. list player does not associate music list
 *         != RT_NULL: the handle of the music list being played
 * 
 * @note: The "list_player_current_items" API name changes during the v1.2.0 release!
 */
music_list_t list_player_current_items(void); 

/**
 * Gets the position of the music currently playing(added with v1.2.0)
 *
 * @return  -RT_EINVAL: table is a null
 *          >=0       : the position 
 * 
 * @note: The API name changes during the v1.2.0 release!
 */
int list_player_current_position(void); 

/**
 * Have an associated playlist in the current list player
 *
 * @return   0: there is no
 *           1: there are
 */
int list_player_is_exist(void); 

/**
 * Playing the music list
 *
 * @param table the handle of music list 
 *
 * @return  -RT_ENOSYS: list player not initialized
 *          -RT_EINVAL: table is a null pointer
 *          -RT_ERROR : playback failed
 *           RT_EOK   : playback successful
 * 
 * @note: 1. This function will automatically restore playback 
 *           based on the song information saved in the playlist 
 *        2. Calling this function will start playing automatically
 */
int list_player_play(music_list_t table);

/**
 * Swicth the music list (added with v1.2.0)
 * 
 * @param table the handle of music list 
 * @param index need to set the music list index
 * @param position need to set the music position
 * @param state need to set the player state @see "enum PLAYER_STAT" 
 *
 * @return  -RT_ENOSYS: list player not initialized
 *          -RT_EINVAL: table is a null pointer
 *           RT_EOK   : Swicth successful
 * 
 * @note: 1. This function will stop the previous play and switch the music list
 *        2. it will not resume playing according to the status of the switched music list
 */
int list_player_switch(music_list_t table, int index, int position, int state);

/**
 * Play music by index in current music list
 *
 * @param num the index of music item
 *
 * @return  RT_EOK: play successfully
 *
 * @note: v1.3.0 will remove the list_player_num(), please use list_player_play_index()!
 */
int list_player_num(int num); 
int list_player_play_index(int index);

/**
 * Play music by handle in current music list
 *
 * @param item: the handle of music item
 *
 * @return  RT_EOK: play successfully
 *
 * @note: v1.3.0 will remove the list_player_item(), please use list_player_play_item()!
 */
int list_player_item(music_item_t item); 
int list_player_play_item(music_item_t item); 

/**
 * Stop the music list
 */
void list_player_stop(void);

/**
 * Pause the music list
 */
void list_player_pause(void); 
void list_player_suspend(void); 

/**
 * Resume the music list
 */
void list_player_resume(void);

/**
 * Play the last music in the current music list
 * 
 * @note: when the first music is played, calling this function will start the last music
 */
void list_player_prev(void); 

/**
 * Play the next music in the current music list
 * 
 * @note: when the last music is played, calling this function will start the first music
 */
void list_player_next(void); 

/**
 * Detach music list from list player, but it doesn't free memory
 *
 * @return == RT_NULL: 1. list player not initialized
 *                     2. Empty playlist
 *         != RT_NULL: detach successful 
 * 
 * @note: The music list is playing will stop playing
 * @note: v1.3.0 will remove the list_player_detach_items(), please use list_player_detach()!
 */
music_list_t list_player_detach(void); 
music_list_t list_player_detach_items(void); 

/**
 * Empty the music list of current played. it will free memory
 *
 * @return  -RT_ERROR: list player not initialized
 *           RT_EOK  : detach successful 
 * 
 * @note: The music list is playing will stop playing
 */
int list_player_empty(void); 

/**
 * Set list playback mode
 *
 * @param mode @see "list play mode" related macro definition 
 *
 * @return  -RT_ENOSYS: list player not initialized
 *          -RT_EINVAL: mode parameter are out of range
 *           RT_EOK   : set successful 
 * 
 * @note: v1.3.0 will remove the list_player_mode_set(), please use list_player_set_mode()!
 */
int list_player_set_mode(int mode); 
int list_player_mode_set(int mode); 

/**
 * Create the music list
 *
 * @return == RT_NULL: no memory
 *         != RT_NULL: create successful 
 */
music_list_t list_player_items_create(void);

/**
 * Delete the music list
 *
 * @param table: the handle of music list
 */
void list_player_items_delete(music_list_t table);

/**
 * Empty the music list, but can't empty the music list in playing(added with v1.2.0)
 *
 * @param table the handle of music list
 *
 * @return  -RT_EINVAL: table is a null
 *          -RT_EBUSY : the music list is playing
 *           RT_EOK   : empty successful
 */
int list_player_items_empty(music_list_t table); 

/**
 * Set the music list callback function
 *
 * @param table the handle of music list
 * @param handler The callback function
 * @param arg The callback function parameter
 * 
 * @Todo: add event trigger situation!
 * @note: v1.3.0 will remove the list_player_set_table_handler(), please use list_player_items_set_handler()!
 */
void list_player_items_set_handler(music_list_t table, list_event_handler handler, void *arg);
void list_player_set_table_handler(music_list_t table, list_event_handler handler, void *arg);

/**
 * Get the number of musics in the music list(added with v1.2.0)
 *
 * @param table the handle of music list
 *
 * @return  -RT_EINVAL: table is a null
 *          >=0       : the number of musics
 */
int list_player_items_get_num(music_list_t table); 

/**
 * Get the index of last music in the music list(added with v1.2.0)
 *
 * @param table the handle of music list
 *
 * @return  -RT_EINVAL: table is a null
 *          -RT_ERROR : never played a music 
 *          >=0       : last music index
 */
int list_player_items_get_index(music_list_t table); 

/**
 * Get the item of last music in the music list(added with v1.2.0)
 *
 * @param table the handle of music list
 *
 * @return  == RT_NULL: table is a null, or table->curr_info is null
 *          != RT_NULL: last music item
 */
music_item_t list_player_items_get_item(music_list_t table); 

/**
 * Add music to the music list
 *
 * @param table the handle of music list
 * @param item the handle of music that need to be added
 * @param index the position the music added, -1 means add to the end of the music list
 *
 * @return  -RT_EINVAL: table is a null, or item is a null, or item->URL is a null
 *          -RT_ERROR : index is out of range(-1 ~ table->item_num)
 *          -RT_ENOMEM: no memory
 *           RT_EOK   : add successful 
 */
int list_player_item_add(music_list_t table, music_item_t item, int index); 

/**
 * Delete music in the music list
 * 
 * @param table the handle of music list
 * @param item the handle of music that need to be deleted
 *
 * @return  -RT_EINVAL: table is a null, or item is a null
 *          -RT_ERROR : the item not found
 *           RT_EOK   : delete successful 
 */
int list_player_item_del(music_list_t table, struct music_item *item);

/**
 * Delete music in the music list according to music index
 *
 * @param table: the handle of music list
 * @param index: the index of music that need to be deleted
 *
 * @return  -RT_EINVAL: table is a null, or table is empty, or index >= table->item_num
 *          -RT_ERROR : index is out of range(-1 ~ table->item_num)
 *           RT_EOK   : delete successful 
 */
int list_player_item_del_by_index(music_list_t table, int index);

/**
 * Gets the item handle in the music list based on the index of the music 
 *
 * @param index: the index of music
 *
 * @return == RT_NULL: No item have been found
 *         != RT_NULL: get item successful 
 */
music_item_t list_player_item_get(music_list_t table, int index);

/**
 * Gets the index of the music in the music list based on the item handle
 *
 * @param item: the handle of music item
 *
 * @return >=0        : item index
 *          -RT_EINVAL: item is a null
 *          -RT_ERROR : item->name and item->URL is a null    
 */
int list_player_index_get(music_list_t table, music_item_t item);

/**
 * Dump the specifies music list information
 *
 * @param table: the handle of music list
 *
 * @return  -RT_EINVAL: table is a null
 *           RT_EOK   : dump successful 
 */
int list_player_dump(music_list_t table); 

/** 
 * Bell internal use, developers please do not call 
 */ 
void list_player_lock(void);
void list_player_unlock(void);
void list_player_take(void); 
void list_player_release(void);

#ifdef __cplusplus
}
#endif

#endif
