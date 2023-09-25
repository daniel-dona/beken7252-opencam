# TJpgDec

## 1、介绍

[TJpgDec（Tiny JPEG Decompressor）](http://elm-chan.org/fsw/tjpgd/00index.html)是针对嵌入式系统高度优化的轻量级通用 JPEG 图像解码器，具有易于使用、资源占用小、方便移植等优点，广泛应用在各种资源受限的微处理器上，本软件包是 TJpgDec 的 RTT 移植版本。

### 1.1 目录结构

| 文件夹 | 说明 |
| ---- | ---- |
| examples | Jpeg 解码示例 |

### 1.2 许可证

TJpgDec 遵循 BSD 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 3.0+

## 2、如何打开 TJpgDec

使用 TJpgDec 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    multimedia packages --->
        [*] TJPGDecode: Tiny JPEG Decompressor.  --->
```

### 2.1 配置选项说明

| 配置选项            | 类型       | 说明                                       |
|--------------------|------------|-------------------------------------------|
| Size of stream input buffer | int | 图像输入缓冲区大小                        |
| Output format      | bool       | 图像输出格式，可选 RGB888 或 RGB565         |
| Use output descaling | bool     | 使能输出图像尺寸缩放                        |
| Use table for satureation | bool | 使能参数表加速解码过程，但额外占用 1K 代码空间  |
| Enable tjpgd sample | bool | 使能 tjpgd 示例程序 |

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 TJpgDec

在打开 TJpgDec 后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。

TJpgDec 解码库的工作流程包括 Prepare 和 Decompress 两个阶段，核心接口有 `jd_prepare()`、`jd_decomp()`、`in_func()`、`out_func()`，其中 `in_func()` 和 `out_func()` 是数据读写回调函数，需要用户实现。

### 3.1 示例代码

勾选 `Enable tipgd sample` 菜单选项启动示例程序，编译工程，系统运行后在 Shell 输入 `Jpeg_Dec xxx.jpg` 命令测试解码库。

```
 \ | /
- RT -     Thread Operating System
 / | \     4.0.2 build Aug 20 2019
 2006 - 2019 Copyright by rt-thread team
lwIP-2.0.2 initialized!
[I/SDIO] SD card capacity 65536 KB.
found part[0], begin: 32256, size: 63.992MB
file system initialization done!
hello rt-thread
msh />
msh />
msh />Jpeg_Dec rtt.jpg
Image dimensions: 128 by 128. 3096 bytes used.
OK
msh />
```

* 完整的 API 手册可以访问 [TJpgDec - Tiny JPEG Decompressor](http://elm-chan.org/fsw/tjpgd/00index.html)

## 4、注意事项

* 解码过大的图片将消耗大量内存，TJpgDec 库工作需要 3K 的算法内存，以及 `3 × image_width × image_height` Byte 的图像输出缓存（RGB888 格式输出时），请确保系统 RAM 足够。
* 若解码出现 `Failed to prepare: rc=3` 错误，请尝试将输出 buffer 改小（一般情况下保持默认选项即可）。

## 5、联系方式 & 感谢

* 维护：RT-Thread 开发团队
* 主页：https://github.com/RT-Thread-packages/TJpgDec


