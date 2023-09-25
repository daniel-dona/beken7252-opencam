## RT-Thread Play Component 例程 

本文件夹下主要包括 Play Component 的相关例程，目的是为了快速上手熟悉和快速构建应用。

目前所支持的例程包括：

| 编号 | 类别    | 文件夹                   | 说明                                         | 状态       |
| :--- | :----- | :----------------------- | :------------------------------------------ | :--------- |
| 00 | 命令   | 00_cmd                     | Play组件msh命令, 用于熟悉功能, 测试            | 完成, 待讨论 |
| 01 | 基础   | 01_basic_play_list         | 简单播放歌单, 4种播放模式                      | 完成, 待讨论 |
| 02 | 基础   | 02_basic_random_play_list  | 随机播放音乐demo                              | 完成, 待讨论 |
| 03 | 应用   | 03_app_insert_bell         | 播放音乐过程中插入提示音demo, 依赖于play_bell   | 完成, 待讨论 |
| 04 | 应用   | 04_app_cloud_push_music    | 云端下发音乐demo                              | 完成 |
| 04 | 应用   | 05_app_limit_play_list_num | 限制歌单中歌曲数量                            | 完成 |

## 支持命令lp/listplayer

```c
msh />listplayer
usage: listplayer [option] [target] ...

listplayer options:
  -h,      --help                     Print defined help message.
  -u,      --dump                     Dump playlist or music info.
  -a,      --add                      Add music to list player, need to specify music name uri or index.
  -d,      --del                      Delete playlist music.
  -t,      --stop                     Stop the list player.
  -e,      --pause                    Pause the list player.
  -r,      --resume                   Resume the list player.
  -r,      --prev                     Play the prev song.
  -r,      --next                     Play the next song.
  -m MODE, --mode=MODE                Set list player play mode.

  --name=MODE                         Describe the name of the music.
  --uri=URI                           Describe the URI of the music.
  --index=INDEX                       Describe the number of music in the play list.

listplayer examples:
  listplayer --add --name=xxx --uri=http://www.xxx.com/1.mp3
  listplayer --add --index=n --name=xxx --uri=http://www.xxx.com/1.mp3

  listplayer --del=all

  listplayer --del --name=xxx
  listplayer --del --index=n
  listplayer --del --uri=http://www.xxx.com/1.mp3

  listplayer --play
  listplayer --play --name=xxxx
  listplayer --play --index=1
  listplayer --play --uri=http://www.xxx.com/1.mp3

  listplayer --stop
  listplayer --pause
  listplayer --resume
  listplayer --prev
  listplayer --next
  listplayer --help

  listplayer --dump
  listplayer --dump --name=xxx
  listplayer --dump --index=n
  listplayer --dump --uri=http://www.xxx.com/1.mp3

  listplayer --mode=once-list
  listplayer --mode=once-song
  listplayer --mode=repeat-list
  listplayer --mode=repeat-song
```
