# RT-Thread 云接入演示例程

## 1、 简介

`samples/rtcloud` 是 `cloudsdk` 包在 `beken` 平台上的一个例程，这里简单介绍了如何使用 rt-thread 云。

## 2、代码说明

- 依赖 **cloudsdk** 包
- 依赖 **rt_ota** 包  
`samples/ota/port/rt_ota` 目录下是对 **rt_ota** 包的必要移植文件。
    - **rt_ota_cfg.h** 存放着分区表相关的配置
- 依赖 **easyflash** 包
- `samples/rtcloud/port` 目录  
该目录下是 `cloudsdk` 必要的移植文件，包括 `easyflash` 和 `rt_cld`。
- CLD_SN   
设备序列号，在 RT-Thread Cloud 上注册获取，修改位置在 `samples/rtcloud/port/rt_cld/rt_cld_port.c`。

## 3、 如何使用

### 3.1 使用默认配置
为了让用户能够快速上手使用 `rtcloud` 的功能，在 `rtcloud` 目录下提供了默认的配置文件 `.config`、`rtconfig.h` 和 `link.lds`，用户只需要将这三个文件拷贝到 `bk7231` 项目的`根目录`下既可。

完成配置文件拷贝后使用 `scons` 命令，编译得到 `rtthread.bin` 固件。

这里就 cloudsdk 的 OTA 升级功能给出以下基本使用流程：  

- 拷贝默认配置文件
- 编译固件
- 打包工具打包升级固件
- 上传升级固件到 rtthread cloud
- 下载运行固件到 beken 板卡
- 固件运行检查 OTA 升级
- 完成 OTA 升级

### 3.2 使用 menuconfig 配置

在用户对整个工程比较熟悉的情况下，用户可以使用 `menuconfig` 进行自定义配置。

- 使能 `rtcloud` 例程
```
Application Samples config --->
        [*] Enable rt-thread cloudsdk sample
```

- 配置 `cloudsdk`   
    - Device Product ID    
产品 ID，在 RT-Thread Cloud 上注册获取。  
    - Device Product KEY   
产品密钥，在 RT-Thread Cloud 上注册获取。  
```
  Privated Packages of RealThread --->
          [*] CloudSDK: RT_thread IOT CloudSDK
          (XXX)   Device Product ID
          (XXX)   Device Product KEY
          [ ]     Device using default information and ignore activate
          [*]     Enable Web Shell
          [*]     Enable Web Log
          [*]     Enable OTA
                  CloudSDK version (latest_version) --->
```


## 4、 注意事项

- **Device activate failed** 激活失败  
查看 **CLD_SN** 是否正确。
- **create socket error** 创建 socket 失败  
系统配置的 socket 数量过少，使用 menuconfig 修改：
```
RT-Thread Components  --->
    Device virtual file system  --->
        [8] The maximal number of opened files
    Network stack  --->
        light weight TCP/IP stack  --->
            (8) the number of TCP socket
```
