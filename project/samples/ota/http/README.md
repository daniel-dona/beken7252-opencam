# HTTP OTA 升级

## 1、简述

本例程展示了如何使用 HTTP 方式完成 RT-Thread OTA 升级功能。

## 2、代码说明

- 依赖 **webclient** 包
- 依赖 **fal** 包  
`samples/ota/port/` 目录下是对 **fal** 包的必要移植文件。
    - **fal_cfg.h** 存放着分区表相关的配置
- `http_client_ota.c` 配置默认的 OTA URI  

```C
#define HTTP_OTA_URL              "http://192.168.10.135:80/rtthread.rbl"
```

## 3、如何使用

### 2.1 使用默认配置
为了让用户能够快速上手使用 OTA 的功能，在 `ota` 目录下提供了默认的配置文件 `.config`、`rtconfig.h` 和 `link.lds`，用户只需要将这三个文件拷贝到 `bk7231` 项目的`根目录`下既可。

完成配置文件拷贝后使用 `scons` 命令，编译得到 `rtthread.bin` 固件。


### 2.2 使用 menuconfig 配置

在用户对整个工程比较熟悉的情况下，用户可以使用 `menuconfig` 进行自定义配置。

在 menuconfig 中配置使能 HTTP OTA 例程，如下所示：

```
Application Samples Config --->
    RT-Thread OTA samples --->
        [*] HTTP OTA mode
```

### 2.3 使用流程

- 烧录 all.bin
- 准备 OTA 升级固件
    - 使能 HTTP OTA 功能
    - 配置链接脚本
    - 打包 OTA 固件
- 板卡与电脑通过串口连接  
- `MSH` 命令升级  

```
msh />
msh />http_ota http://192.168.10.135:8000/rtthread.rbl
[I/HTTP_OTA] Start erase flash download partition!
[I/HTTP_OTA] Erase flash (download) partition success!
[I/HTTP_OTA] Download: [====================================================================================================] 100%
[I/OTA] Verify 'download' partition(fw ver: 24.4, timestamp: 1521791141) success.
reboot system 
```

详细的使用说明参考 `doc` 目录下的 **`UM9001_RT-Thread_Beken7231_OTA_User_Manual`** 手册。