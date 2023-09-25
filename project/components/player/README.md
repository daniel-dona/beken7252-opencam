# Player

## 1、介绍

**Player** 是 RT-Thread 开发的适合 MCU 的轻型流媒体音频框架，资源占用小，响应快。

RT-Thread 提供的 Player 组件具有以下优势：

- 本地播放：支持播放本地文件系统中的音频文件
- 网络播放: 支持播放网络数据流歌曲，支持 `http`、`https`、`HLS`、`TTS` 流式音频播放
- 支持多种音频格式：默认支持 `pcm`、`wav` 音频格式
- 支持外置注册解码器，支持 `mp3`、`aac`，`m4a`、`flac`、`amr`、`ogg`、`wma`、`flac` 格式音频外置注册
- 支持歌单播放功能：支持 `添加`、`删除`、`播放`、`暂停`、`上一曲`、`下一曲`、`切换歌单` 等多种歌单播放功能

更多详细介绍，参考 [Player 介绍](docs/introduction.md) 文档。

### 1.1 目录结构

> player packages 源码 的目录结构

| 名称 | 说明 |
| ---- | ---- |
| example | Player 组件使用例程 |
| inc  | Player 头文件路径 |
| libs | Player 静态库, 包含 debug 和 release |
| LICENSE | 许可声明 |
| README.md | 简单介绍 |
| SConscript | Player 组件 scons 脚本 |

### 1.2 许可证

**Player** 软件包遵循 GPL2+ 商业双许可。该软件包可以根据 GUN 标准使用通用公共许可证，详见 LICENSE 文件。如果用于商业应用，可以通过电子邮箱 <business@rt-thread.com > 与我们联系获取商业许可。

### 1.3 依赖

- RT-Thread 3.0+
- [LwIP](https://github.com/RT-Thread/rt-thread/tree/master/components/net/lwip-2.0.2)
- [cJson](https://github.com/RT-Thread-packages/cJSON)
- [WebClient v2.x.x](https://github.com/RT-Thread-packages/webclient)
- [DFS](https://github.com/RT-Thread/rt-thread/tree/master/components/dfs)
- [Finsh](https://github.com/RT-Thread/rt-thread/tree/master/components/finsh)
- [Codec](None)

## 2、联系方式

* 维护：RT-Thread 团队
