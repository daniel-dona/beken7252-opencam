# RT-Thread OTA 包移植说明

本文主要介绍如何将 rt_ota 包移植到其他板卡平台上，以及相关的配置说明。

## 1、概述

OTA 的主要功能就是固件的下载和升级，与之紧密相关的就是 Flash 操作， rt_ota 包移植的主要工作就是对接 Flash 的驱动以及划分分区表。

移植过程中主要涉及的文件：

|文件                                    | 说明 |
|:-----                                  |:----|
|/ports/temp/rt_ota_cfg.h                   | 包相关的配置文件，比如：Flash 设备表、分区表等其他选项 |
|/ports/temp/rt_ota_key_port.c              | 获取OTA升级时用以AES解密的密钥信息 |
|/ports/temp/rt_ota_flash_temp.c            | Flash 移植接口的模板文件 |
|/ports/stm32f2/                            | stm32f2 型号设备移植示例 |
|/ports/beken/                              | beken7231 型号设备移植示例 |

## 2、Flash 设备驱动

Flash 设备一般分为 `片内 Flash` 和 `片外 Flash`， 如果 `片内 Flash` 不足以存储 OTA 固件，则还需要用户提供片外 Flash 驱动接口，一般接口为 SPI 。

在对接 Flash 接口， 可以参考 `/ports/rt_ota_flash_temp.c` 来实现板卡相关的接口。首先将自己的移植文件添加到自己项目对应`ports`目录下，在按照下面的顺序去实现每个接口。

### 2.1 Flash 读数据接口

`read` 函数是 `rt_ota` 包中抽象出来，用于对接 flash 驱动的接口，用户需要在这里实现自己板卡相应的 Flash read 驱动接口。

```C
static int read(uint32_t offset, uint8_t *buf, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|offset                                  |读取数据的 Flash 偏移地址|
|buf                                     |存放待读取数据的缓冲区|
|size                                    |待读取数据的大小|
|return                                  |返回实际读取的数据大小|

### 2.2 Flash 写数据接口

```C
static int write(uint32_t offset, const uint8_t *buf, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|offset                                  |写入数据的 Flash 偏移地址|
|buf                                     |存放待写入数据的缓冲区|
|size                                    |待写入数据的大小|
|return                                  |返回实际写入的数据大小|

### 2.3 Flash 擦除数据接口

`erase` 擦除函数，是根据 Flash 扇区大小进行擦除的，实现该接口的时候注意以扇区进行擦除。

```C
static int erase(uint32_t offset, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|offset                                  |擦除区域的 Flash 偏移地址|
|size                                    |擦除区域的大小|
|return                                  |返回实际擦除的区域大小|

### 2.4 定义 Flash 设备

Flash 设备需要定义在移植文件的末尾，可以参考 `rt_ota_flash_temp.c` 文件，下面简单举例： 

- 片内 Flash

```C
const struct rt_ota_flash_dev onchip_flash_dev = {"onchip_flash", RT_OTA_FLASH_START_ADDR, RT_OTA_FLASH_SIZE, {read, write, erase}};
```

- 片外 Flash

```C
const struct rt_ota_flash_dev spi_flash_dev = {"spi_flash", RT_OTA_SPI_FLASH_START_ADDR, RT_OTA_SPI_FLASH_SIZE, {read, write, erase}};
```

### 2.5 定义 Flash 设备表

定义好的 Flash 设备以后，需要在 `rt_ota_cfg.h` 文件中定义 Flash 设备表。 可以参考下面示例，将定义好的 Flash 设备添加到 `RT_OTA_FLASH_DEV_TABLE` 宏里去。

```C
/* ===================== Flash device Configuration ========================= */
extern const struct rt_ota_flash_dev onchip_flash_dev;
extern const struct rt_ota_flash_dev spi_flash_dev;

/* flash device table */
#define RT_OTA_FLASH_DEV_TABLE                           \
{                                                        \
    &onchip_flash_dev,                                   \
    &spi_flash_dev,                                      \
}
```

### 2.6 万能 SPI Flash 驱动 SFUD 的对接说明

如果使用的是 SFUD 来驱动的 SPI Flash，这里的移植工作将会减少很多。因为包里已经添加好了 SFUD Flash 设备驱动的参考示例，只需做如下简单修改，即可使用。

- 在 `menuconfig` 使能 `RT_OTA_FLASH_PORT_DRIVER_SFUD` 配置
- 打开 `/port/rt_ota_flash_sfud_port.c` 文件
- 修改 `extern` 的 SFUD Flash 的设备即可（默认对象为：`extern sfud_flash sfud_norflash0;`）

## 3、分区表

分区表记录了全部分区的信息，例如下面这个分区表，针对两个 Flash 设备定义了 3 个分区

| 分区名      | Flash 设备名   | 偏移地址    | 大小  | 说明       |
|:-----       | :----          |:----        |:----  |:----       |
|"bootloader" | "onchip_flash" | 0           | 64KB  | 引导程序   |
|"app"        | "onchip_flash" | 64*1024     | 448KB | 应用程序   |
|"download"   | "spi_flash"    | 1*1024*1024 | 512KB | OTA 下载区 |

### 3.1 分区的参数

用户需要修改的分区参数包括：分区名称、关联的 Flash 设备名、偏移地址（相对 Flash 设备内部）、大小，需要注意以下几点：

- 分区名保证 **不能重复**
- 关联的 Flash 设备 **务必已经在 Flash 设备表中定义好** ，并且 **名称一致** ，否则会出现无法找到 Flash 设备的错误
- 分区的起始地址和大小 **不能超过 Flash 设备的地址范围** ，否则会导致包初始化错误

### 3.2 定义分区表

明确了分区参数后，需要在 `rt_ota_cfg.h` 文件中定义分区表。对应的宏定义为 `RT_OTA_PART_TABLE` 。可以参考下面的示例，该示例在分区表中添加了 3 个分区，与第 3 章开始部分列出的示例分区表信息一致：

```C
/* partition table */
#define RT_OTA_PART_TABLE                                            \
{                                                                    \
    {RT_OTA_PART_MAGIC_WROD, "bootloader", "onchip_flash", 0,           RT_OTA_BL_PART_LEN,  0}, \
    {RT_OTA_PART_MAGIC_WROD, "app",        "onchip_flash", 64*1024,     RT_OTA_APP_PART_LEN, 0}, \
    {RT_OTA_PART_MAGIC_WROD, "download",   "spi_flash",    1*1024*1024, RT_OTA_DL_PART_LEN,  0}, \
}
```

> 注意：每个分区定义时，除了填写 3.1 中介绍的参数属性外，需在前面增加 `RT_OTA_PART_MAGIC_WROD` 属性，末尾增加 `0` （目前用于保留功能）
