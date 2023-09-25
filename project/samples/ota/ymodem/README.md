# Ymodem

## 1、简述

这是 Ymodem 在 beken7231 平台上的应用示例，展示了如何使用 Ymodem 方式完成 RT-Thread OTA 升级功能。

## 2、代码说明

- 依赖 **fal** 包  
`samples/ota/port/` 目录下是对 **rt_ota** 包的必要移植文件。
    - **fal_cfg.h** 存放着分区表相关的配置

## 3、如何使用

- 烧录 all.bin
- 准备 OTA 升级固件
    - 使能 Ymodem OTA 功能
    - 配置链接脚本
    - 打包 OTA 固件
- 板卡与电脑通过串口连接
- Ymodem 升级

上面是 Ymodem 方式 OTA 升级的基本步骤，详细的使用说明参考 `doc` 目录下的 **`UM9001_RT-Thread_Beken7231_OTA_User_Manual`** 手册。