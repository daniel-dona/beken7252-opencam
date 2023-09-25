/*
 * simple Player demo for file and tip playing
 */

#include <rtthread.h>
#include "player.h"
#include "list_player.h"
#include "player_app.h"
#include <finsh.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct play_list_struct{
	music_list_t  which_playlist;
	int play_list_status;
	int play_list_position;
	int play_list_num;
	music_item_t play_list_content;
	char backup_url[128];
}play_list_struct;

play_list_struct saved_list;
static void save_current_playlist_status(void)
{
	int state = list_player_current_state();
	int num = list_player_current_index();
	int postion  =list_player_current_position();
	music_list_t tmp_list =list_player_current_items();
	int list_num = list_player_items_get_num(tmp_list);
	music_item_t tmp = list_player_current_item();
	
	saved_list.which_playlist = tmp_list;	
	saved_list.play_list_status = state;
	saved_list.play_list_num = num;
	saved_list.play_list_position = postion; 
	saved_list.play_list_content = tmp;

	rt_kprintf("%p %d %d %d\r\n", tmp_list, num, postion, state);
}

static void bell_list_handle(void)
{
   list_player_switch(saved_list.which_playlist,
		saved_list.play_list_num,
		saved_list.play_list_position,
		saved_list.play_list_status);
}

music_list_t song_list = NULL; 
int list_player(int argc, char** argv)
{
	struct music_item items = {0};
	items.name = ("Stream");
		
	items.URL = argv[1];

	if (!song_list)
	{
	    song_list =list_player_items_create();
	}
	
	list_player_mode_set(LISTER_LIST_ONCE_MODE);
	list_player_item_add(song_list, &items,-1);
	list_player_play(song_list);

    rt_kprintf("list player test\r\n");
}

music_list_t bell_list = NULL; 
int bell_player(int argc, char** argv)
{
    struct music_item items = {0};
	items.name = ("Bell");
		
	items.URL = ("/flash0/rhymes.mp3"); //argv[1];

    save_current_playlist_status();
	
	if (!bell_list)
	{
		bell_list =list_player_items_create();
	}
	
    list_player_mode_set(LISTER_LIST_ONCE_MODE);
	list_player_item_add(bell_list, &items,-1);
	list_player_switch(bell_list,0,0,PLAYER_STAT_PLAYING);
	//list_player_play(bell_list);
	list_player_set_table_handler(bell_list,bell_list_handle,NULL);

    rt_kprintf("bell player test\r\n");
	// Add table handler to resume song_list playing
}
MSH_CMD_EXPORT(list_player, list_player test);
MSH_CMD_EXPORT(bell_player, bell_player test);

