/*
 * File: player_app.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __PLAYER_APP_H__
#define __PLAYER_APP_H__

#include <rtdef.h>

enum PLAYER_KEY
{
    KEY_VOLINC,
    KEY_VOLDEC,
    KEY_NEXT,
    KEY_PREV,

    KEY_PAUSE,
    KEY_PLAY,
    KEY_PLAY_PAUSE,

    KEY_MUTE,

    KEY_MIC,
    KEY_EQ,
    KEY_MENU,
    KEY_CHANNEL,
    KEY_FAVORITE,

    //system shutdown, wifi config...
    KEY_PWROFF,
    KEY_CONFIG,
    KEY_NETWORK_MODE,

    KEY_SOURCE,
    KEY_UNKNOWN,
};

#define PLAYER_APP(app) (struct player_app*)(app)
struct player_app;

struct player_app_ops
{
    int (*connect)   (struct player_app *app);
    int (*disconnect)(struct player_app *app);

    void (*key_handle)(struct player_app* app, int key);
    void (*event_handle)(struct player_app* app, int event);
};

struct player_app
{
    const char* name;
    const struct player_app_ops *ops;

    rt_list_t list;
};

int player_app_init(void);

void player_app_handle_key(int key);
void player_app_handle_event(int event);

void player_app_set_volume(struct player_app *app, int volume);
int  player_app_get_volume(struct player_app *app);

void player_app_register  (struct player_app *app);
int  player_app_connect(struct player_app *app);
void player_app_disconnect(struct player_app *app);

void player_app_frozen    (void);
void player_app_unfrozen  (void);
rt_bool_t player_app_is_frozen(void);

struct player_app *player_app_get_activated(void);
rt_bool_t player_app_is_activated(struct player_app *app);

/* disconnect current player app and stop player */
struct player_app *player_app_stop(void);
/* suspend current player app (may disconnect current player) */
struct player_app *player_app_suspend(void);
/* resume player app (may connect the supended player app by app self.) */
void player_app_resume (struct player_app *app);

#endif
