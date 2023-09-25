# Ayla SDK 上手指南

## 1、 简介

**Ayla SDK** 是艾拉物联推出的用于快速接入 **Ayla 云** 的软件包。

**艾拉物联（Ayla Networks）** 在业界率先推出敏捷物联网云平台，能够加速物联网产品的开发、支持和迭代，是顶级制造厂商的首选平台。Ayla端到端解决方案能够连通设备、云以及移动App，为我们的客户和最终消费者提供安全连接、大数据分析、以及功能丰富的用户体验。

作为PaaS云平台，Ayla采用灵活的模块化策略，无论平台的任一部分需要变更，都可以快速实现。Ayla的企业级物联网云平台正在服务于众多全球顶级企业，将楼宇控制、暖通空调、家用电器、照明等设备连入智能系统，实现云端管理，以及随时随地的移动应用访问。

## 2、SDK 说明

- Ayla SDK 目录结构与描述如下所示：  
**rt_port** 目录下是 RT-Thread 移植的文件，其中 **rt_port/beken** 下是 **beken** 平台相关的移植文件，**rt_port/ram** 下是使用 RAM 模拟 FLASH 存储 Ayla 参数配置相关的移植文件，目的是用于在其他平台使用 Ayla SDK。  

```
ayla_sdk
│   .style_ok       // Ayla SDK 预置文件
│   Makefile        // Ayla SDK 默认的构建脚本
│   SConscript      // RT-Thread 默认的构建脚本
│   
├───ayla_demo       // Ayla SDK 演示例程
├───ext             // Ayla SDK 外部依赖文件
├───include         // 全局头文件
├───libada          // Ayla 云交互相关文件
├───libadw          // Ayla WiFi 框架相关文件
├───libayla         // Ayla SDK 基础文件
├───libnet          // Ayla SDK 网络相关文件
├───make            // Ayla SDK 默认的构建脚本
└───rt_port         // RT-Thread 移植文件
    ├───beken          // beken 平台移植文件
    └───ram            // RAM 模拟 FLASH 移植文件
```

- Ayla 例程目录结构与描述如下所示：

```
samples             // RT-Thread 提供的演示例程
└───ayla
    │   Kconfig     // RT-Thread menuconfig 配置文件
    │   README.md   // 上手指南
    │   SConscript  // RT-Thread 默认的构建脚本
    └───port
        └───beken   // RT-Thread 移植相关文件
                ayla_flash_port.c
                ayla_flash_port.h
```

## 3、 如何使用

### 3.1 必备条件

- Ayla dashboard 平台账户    
  该平台账户下有设备接入 Ayla 云必备的 OEM ID、OEM Key 和 DSN。    
    - OEM ID
    - OEM Key
    - DSN
- Ayla developer 平台账户  
  该平台下进行设备模板创建等设备相关操作。  
- XML 文件  
  XML 文件从 dashboard 平台获取，存储着 DSN 和 Public Key。

### 3.2 使用 menuconfig 配置

使用 `menuconfig` 进行配置使用 **Ayla Cloud**。

```
Application Samples config --->
        [*] Enable ayla cloud sample
        [ ]   Using rt_ota function
```

为了让开发者快速使用，默认在 **samples/ayla** 目录下提供了配置文件 `.config` 和 `rtconfig.h`，用户只需要将这两个文件拷贝到项目的根目录即可进行编译。

### 3.3 代码修改

**请务必将下面的配置信息修改为您的平台对应的配置。**

- 修改 OEM ID 和 OEM MODEL  
    `ayla_demo/src/conf.h`：#define DEMO_OEM_ID	"0dfc7900"  
    `ayla_demo/src/conf.h`：#define DEMO_LEDEVB_MODEL "ledevb"  
- 修改 OEM 账户的服务器域名  
    `ayla/libada/client.c`：#define CLIENT_SERVER_DOMAIN_US    "aylanetworks.com"   

```
static const struct hostname_info server_region_table[] = {
	{ "US", "aylanetworks.com" },
	{ "CN", "aylanetworks.com" },
};
```

- 修改 Ayla 模型  
  请根据 dashboard 工厂服务里使用的 DSN 来确定您使用的 Ayla 模型。  
  `ayla/libada/al/rtthread/stubs.c`：#define AYLA_MODEL "AY008RTK1"

### 3.4 设备配置

**请务必将下面的配置信息修改为您的平台对应的配置。**

初始使用，需要使用 CLI 命令进行以下配置：

1. 获取设备序列号 (DSN) 和密钥 (Key)  
DSN 和 key 由 Ayla 提供，设备唯一。每个DSN及其密钥都有一个单独的XML文件。  

```
<?xml version="1.0" encoding="UTF-8"?>
<f-device>
<dsn>AC000W00012346</dsn>
<public-key>-----BEGIN RSA PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4uBo4pDNe7aS4CJS0ck1
pKaHCS1fV2Oqr+DnPiA5Ue/6Htac7j9DQyc4qD8sxHip/7aHKkESEd7v9M2HqGKv
FPhkb7SDjbj+v/qYca2cWXhqUYv9H5wEeCUG2MaBYGUZ0HfnBiD/PIULWmYBLZ6D
4yF3dQIi+MCgWJU9GCVR6pCt+02SUfIxOQWPslx5nlkMMpj+E1ORjYPSa7kIeoik
waoPc85UfTA+1TSOq5tNHZu+CuoYBkRPkbXM1bhrQx98TY8FQG/zS+kqmedpV73S
xotItJjtlupKQI6C42Bg867ncuvNufUF+58WZGobaZZIu3I3ouXxGqIgqdOMYi+q
kQIDAQAB
-----END RSA PUBLIC KEY-----
</public-key>
</f-device>
```

2. 设置 dev_id/mfg_model/mfg_serial  

```
msh /> ayla_id dev_id SC000W000123456
msh /> ayla_id mfg_model EVB
msh /> ayla_id mfg_serial EVB1
```

3. 设置 public key  
使用 `ayla_file add` 命令，将 XML 文件中的 public key 逐行写入到 flash 中。

```
msh /> ayla_file start 0
msh /> ayla_file add MIIBIjANBgkq … DNe7aS4CJS0ck1
msh /> ayla_file add pKaHCS1fV2Oq … ESEd7v9M2HqGKv
msh /> ayla_file add FPhkb7SDjbj+ … D/PIULWmYBLZ6D
msh /> ayla_file add 4yF3dQIi+MCg … ORjYPSa7kIeoik
msh /> ayla_file add waoPc85UfTA+ … /zS+kqmedpV73S
msh /> ayla_file add xotItJjtlupK … XxGqIgqdOMYi+q
msh /> ayla_file add kQIDAQAB
```

4. 设置 OEM Key  
使用 Ayla 云的设备都有三个必备属性：**OEM ID**、**OEM Model**、**OEM Key**。  
演示设备不需要 OEM Key。  
使用以下命令设置 OEM Key:

```
msh /> ayla_oem key <secret-string>
```

如果需要任何 OEM Model 都可以使用该密钥，则使用以下命令：

```
msh /> ayla_oem key <secret-string> *
```

5. 设置服务区域  

- 设置中国区域

```
msh /> ayla_client server region CN
```

- 设置美国区域

```
msh /> ayla_client server region US
```

6. 设置当前时间

```
msh /> ayla_time 2018-03-30T10:40:00
time set
time cmd disabled
```

7. 查看当前时间
```
msh /> ayla_time
2017-01-01T00:52:22  3142109 ms since boot
```

8. 保存配置

```
msh /> ayla_conf save
```

9. 查看配置

```
msh /> ayla_conf show
```


10. 设置 WiFi

```
msh /> ayla_wifi profile 0
msh /> ayla_wifi ssid Your-SSID-To-Connect
msh /> ayla_wifi security WPA2_Personal <or none>
msh /> ayla_wifi key your-password
msh /> ayla_conf save
msh /> ayla_wifi join
```

如果需要删除 WiFi 配置，使用以下命令：  

```
msh /> ayla_wifi profile <n>
msh /> ayla_wifi profile erase
msh /> ayla_conf save
```

11. 查看 factory log

```
msh />
msh /> ayla_factory-log
factory-log line:
3,1522406460,2018/03/30 10:41:00 UTC,label,0,AY008RTK1,AC000W000123456,00e04c111801,EVB,EVB1,mac-00e04c111801,,0dfc7900,ledevb,0,ADA demo customer

```

12. 使能 **setup_mode**  
为了保证配置不被意外修改，在设置完成后，及时失能 `setup_mode`，使能的时候需要密码，默认为 `aylacn`。

```
msh /> ayla_setup_mode disable
msh /> ayla_setup_mode enable <your defined key>
```

13. Ayla 授权设备 URI  
将 **OEM ID** 、**OEM MODEL** 发送给 Ayla 进行 `(OEM Model)-(OEM iD)-device.aylanetwoks.com` URI 授权使用，建立 DNS 记录。

14. WiFi 入网

Ayla WiFi 命令暂时没有提供支持，需要使用 RT-Thread 的 WiFi 命令接入网络，命令如下：

```
msh /> wifi w0 join your-ssid your-key
```

在网络连接成功之前，Ayla SDK 会一直尝试连接 Ayla 云，网络连接成功后， Ayla SDK 会自动重连接入 Ayla 云，重连时间约为 30 秒，请耐心等待。

### 3.5 dashboard 查看控制设备

- 登录 dashboard 平台  
  使用 Ayla 提供的 OEM dashboard 链接，登录 OEM 账户。
- 查看设备  
  在 **设备** 菜单下可以查看当前 OEM 账户所有的已注册设备。  
  - 点击设备 DSN 查看设备详细信息
- 控制设备  
  - 在设备的详细信息里，`设备 ---> Device Control` 控制设备
  - 在设备的详细信息里，`属性 ---> Blue_LED ---> 数据点 ---> 创建数据点` 操作设备属性


## 4、 注意事项

### 4.1 finsh 命令长度问题

现象如下，这是因为 finsh 命令的长度超过了配置，使用 menuconfig 修改配置即可。

```
msh />ayla_file add MIIBCgKCAQEAgvK3NV8Ahb+slGaukTgPq2JjdgGhzu+i3mC+KiWuyTvqZhB6cFw9pjKx/Vx65rUN
Kx/Vx65rUNadd: command not found.
```
menuconfig 修改 finsh 命令长度如下所示：  

```
RT-Thread Components  --->
    Command shell  --->
        (128) The command line size for shell
```

### 4.2 设备配置命令顺序问题

请按照 `3.4 设备配置` 章节 中的顺序配置设备。需要在 **OEM Key** 之前配置 **Public Key**

### 4.3 Ayla Flash 参数区问题

如果需要修改已有的 Ayla 配置参数，必须要将 Flash 参数区擦除（不能通过 Ayla 命令擦除），然后重新配置。否则会在 dashboard 平台出现 **`Unknown`** 状态。

## 5、参考

- 艾拉物联官网 [http://www.ayla.com.cn/](http://www.ayla.com.cn/)
- 艾拉物联设备管理平台 [https://dashboard-dev.ayla.com.cn](https://dashboard-dev.ayla.com.cn)
- 艾拉物联开发者平台 [https://developer.ayla.com.cn/](https://developer.ayla.com.cn/)