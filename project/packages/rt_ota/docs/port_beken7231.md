# RT-Thread OTA 包移植说明
本文主要介绍如何将 rt_ota 包移植到其他板卡平台上，以 beken7231 SOC 为例。

## 1、 beken7231 概述

beken7231 是博通集成电路（上海）有限公司旗下的一款 WiFi SOC，ARM968E-S 核， 120MHz 主频， 拥有 2Mbytes 的 NorFlash (ADDR: 0x0 - 0x200000)， 256Kbytes RAM (ADDR: 0x400000 - 0x440000)。

需要注意的是，beken7231 NorFlash 存储的代码每 32Bytes 后有 2Bytes 的 CRC16 校验码。CPU 在执行过程中，读取代码的时候会校验 CRC ，如果校验失败，CPU 发生异常，执行中断。CPU 执行的代码的 `逻辑地址` 与 Flash 存储的 `物理地址` 是不同的，需要进行转换，在移植 rt_ota 包的时候需要尤其注意。 


## 2、 Flash 设备驱动

Flash 设备驱动的移植包括芯片内置的 `片内 Flash` 和用户外接的 `片外 Flash`， 如果 `片内 Flash` 不足以存储 OTA 固件， 则需要用户提供片外 Flash 驱动接口，一般为 SPI Flash 驱动。

beken7231 板卡片内有 2Mbytes 大小的 Flash 空间， 无外接 SPI Flash， 因此， 只需要将片内 Flash 驱动接口对接到 rt_ota 包上即可， 参考 `rt_ota` 包目录下的 `./ports/rt_ota_flash_temp.c` 文件。

因为 beken7231 固件的 CRC 特性， 我们需要进行一下考虑： 

> 1 上传到 OTA 服务器的固件是没有经过 CRC 编码的  
  2 下载到 beken 板卡 `download` OTA 区域的固件同样没有经过 CRC 编码  
  3 将代码从 `download` OTA 区域搬运到 `app` 程序运行区域的时候，需要进行 CRC 编码  
  4 对 `app` 程序运行区域进行读写需要使用带 CRC 的 Flash 驱动接口  
  5 对非 `app` 区域进行读写操作的时候， 需要使用无 CRC 的 Flash 驱动接口

通过以上的考虑， 我们需要实现两套 Flash 驱动， 然后对接到 `rt_ota` 包， 参考 `./ports/rt_ota_flash_temp.c` 文件， 分别实现 `rt_ota_flash_beken7231_port.c` 和 `rt_ota_flash_beken7231_port_crc.c`。

### 2.1 Flash 读数据接口

`read` 函数是 `rt_ota` 包中抽象出来， 用于对接 flash 驱动的接口， 用户需要在这里实现自己板卡相应的 Flash read 驱动接口。

> Note:  
在 beken7321 的 `rt_ota_flash_beken7231_port_crc.c` read 接口中， 读取的起始地址是 `逻辑地址` ， 是 CPU 可见的地址， 需要以 32 字节对齐， 并且要转换为 Flash 的 `物理地址` ，然后进行数据读取， 这是由 beken7231 的 CRC 特性所决定的。

```C
static int read(uint32_t addr, uint8_t *buf, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|addr                                    |读取数据的 Flash 起始地址|
|buf                                     |存放待读取数据的缓冲区|
|size                                    |待读取数据的大小|
|return                                  |返回实际读取的数据大小|

### 2.2 Flash 写数据接口

`write` 函数是 `rt_ota` 包中抽象出来， 用于对接 flash 驱动的接口， 用户需要在这里实现自己板卡相应的 Flash write 驱动接口。

> Note:  
在 beken7321 的 `rt_ota_flash_beken7231_port_crc.c` write 接口中， 写入的起始地址是 `逻辑地址` ， 是 CPU 可见的地址， 需要以 32 字节对齐， 并且要转换为 Flash 的 `物理地址` ，然后进行数据读取， 这是由 beken7231 的 CRC 特性所决定的。

```C
static int write(uint32_t addr, const uint8_t *buf, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|addr                                    |写入数据的 Flash 起始地址|
|buf                                     |存放待写入数据的缓冲区|
|size                                    |待写入数据的大小|
|return                                  |返回实际写入的数据大小|

### 2.3 Flash 擦除数据接口

`erase` 函数是 `rt_ota` 包中抽象出来， 用于对接 flash 驱动的接口， 用户需要在这里实现自己板卡相应的 Flash erase 驱动接口。

`erase` 擦除函数， 是根据 Flash 扇区大小进行擦除的， 一般扇区大小为 `4Kbytes`， 实现该接口的时候注意以扇区进行擦除。

```C
static int erase(uint32_t addr, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|addr                                    |擦除区域的 Flash 起始地址|
|size                                    |擦除区域的大小|
|return                                  |返回擦除区域的大小|

### 2.4 定义 Flash 设备

Flash 设备由 `rt_ota_flash_dev` 结构体进行描述， 如下所示：

```C
struct rt_ota_flash_dev
{
    /* flash device name */
    char name[RT_OTA_FLASH_DEV_NAME_MAX];

    /* flash device start address and len  */
    uint32_t addr;
    /* flash device capacity, unit bytes */
    size_t len;

    struct
    {
        int (*read)(uint32_t addr, uint8_t *buf, size_t size);
        int (*write)(uint32_t addr, const uint8_t *buf, size_t size);
        int (*erase)(uint32_t addr, size_t size);
    } ops;
};
```

|成员                                    |描述|
|:-----                                  |:----|
|name                                    |Flash 设备的名称|
|addr                                    |Flash 设备的起始地址|
|len                                     |Flash 设备的容量，单位：字节|
|read                                    |Flash 读接口|
|write                                   |Flash 写接口|
|erase                                   |Flash 擦除接口|

由于 beken 的 CRC 特性， 这里将一块 Flash 抽象出两个设备， rt_ota 包会根据 `分区名` 查找对应使用 Flash 设备， 进而使用相对应的 Flash 操作接口。

> 1 无 CRC 操作设备

```C
const struct rt_ota_flash_dev beken_onchip_flash_crc = {"beken_onchip", RT_OTA_FLASH_START_ADDR, RT_OTA_FLASH_SIZE, {read, write, erase}};
```

> 2 有 CRC 操作设备
```C
const struct rt_ota_flash_dev beken_onchip_flash_crc = {"beken_onchip_crc", RT_OTA_FLASH_START_ADDR, RT_OTA_FLASH_SIZE, {read, write, erase}};
```

### 2.5 定义 Flash 设备表

Flash 设备表定义在 `rt_ota_cfg.h` 文件中， 如下所示：

```C
/* ===================== Flash device Configuration ========================= */
extern const struct rt_ota_flash_dev beken_onchip_flash;
extern const struct rt_ota_flash_dev beken_onchip_flash_crc;

/* flash device table */
#define RT_OTA_FLASH_DEV_TABLE                                       \
{                                                                    \
    &beken_onchip_flash,                                             \
    &beken_onchip_flash_crc,                                         \
}
```

## 3、 分区表

分区表展示了每一个分区的对应功能，以及每一个分区的起始地址和分区名称，如下所示：

| 分区名 | 起始地址(逻辑地址) | 结束地址(逻辑地址) |大小(byte)| 起始地址(物理地址) | 结束地址(物理地址) | 大小(byte) | 说明 |
|:-----       |:---- |:----|:----|:---- |:----|:----|:----|
|bootloader   |0x00000000 | 0x0010000 | 64k  | 0x00000000 | 0x00011000 | 68k  | 引导程序 |
|app          |0x00010000 | 0x00F8000 | 928k | 0x00011000 | 0x00107800 | 986k | 可执行程序 |
|download     | -         | -         | -    | 0x00107800 | 0x001FE000 | 986k | OTA 下载区 |
|UserParam    | -         | -         | -    | 0X001FE000 | 0x001FF000 | 4k   | 用户参数区 |
|SysParam     | -         | -         | -    | 0x001FF000 | 0x00200000 | 4k   | 系统参数区(WiFi预留) |

```
  [physical_addr = logical_addr/32*34]
  Flash Table: <Logical address>   <Physical address>
  ################### bootloader ####################
  +------------+  0x0000000          0x00000000
  |vectors(32b)|
  |            |
  |------------+  bootloader: 64k    68K
  |Bootloader  |
  |            |
  +------------+  0x0010000          0x11000
  ################### app ###########################
  +------------+  0x0010000          0x11000
  |vectors(32b)|
  |            |
  |------------+
  |text        |
  |data        |  app: 928k          986k
  |            |
  |------------+  
  | app header |  0xF7FA0            0x10779A
  |    (96b)   | 
  +------------+  0xF8000            0x107800
  ################### download ######################  
  +------------+  0x107800           0x107800
  | DL  header |
  |    (96b)   |
  +------------+  download: 986k
  |     OTA    |
  |   PACKAGE  |  
  |            |
  +------------+  0x1FE000           0x1FE000
  ################### UserParam #####################
  +------------+  0X1FE000           0x1FE000
  |User        |
  |Param       |  UserParam: 4k
  |            |
  +------------+  0x1FF000           0x1FF000
  ################### SysParam ######################
  +------------+  0x1FF000           0x1FF000
  |  Sys Param |
  |   (WiFi)   |  SysParam: 4k
  |            |
  +------------+  0x200000           0x200000
```


### 3.1 分区的参数

定义一个分区的时候， 在 `rt_ota_cfg.h` 文件中 `RT_OTA_PART_TABLE` 宏定义下追加自己的分区。 分区参数释义如下：

```C
/**
 * OTA partition
 */
struct rt_ota_partition
{
    uint32_t magic_word;

    /* partition name */
    char name[RT_OTA_PART_NAME_MAX];
    /* flash device name for partition */
    char flash_name[RT_OTA_FLASH_DEV_NAME_MAX];

    /* partition start address on flash device */
    uint32_t addr;
    size_t len;

    uint8_t reserved;
};
```

|成员                                    |描述|
|:-----                                  |:----|
|magic_word                              |魔法数|
|name                                    |分区名|
|flash_name                              |flash设备名|
|addr                                    |分区起始地址|
|len                                     |分区大小|
|reserved                                |保留字|

### 3.2 定义分区表

根据以上分区参数， 在 `rt_ota_cfg.h` 文件中 `RT_OTA_PART_TABLE` 宏定义下追加自己的分区。

```C
#if RT_OTA_PART_HAS_TABLE_CFG
/* partition table of rt_ota_partition struct, app table need crc_write mode */
#define RT_OTA_PART_TABLE                                            \
{                                                                    \
    {RT_OTA_PART_MAGIC_WROD, "bootloader", "beken_onchip",     RT_OTA_BL_PART_ADDR,  RT_OTA_BL_PART_LEN,  0}, \
    {RT_OTA_PART_MAGIC_WROD, "app",        "beken_onchip_crc", RT_OTA_APP_PART_ADDR, RT_OTA_APP_PART_LEN, 0}, \
    {RT_OTA_PART_MAGIC_WROD, "download",   "beken_onchip",     RT_OTA_DL_PART_ADDR,  RT_OTA_DL_PART_LEN,  0}, \
}
#endif /* RT_OTA_PART_HAS_TABLE_CFG */
```
