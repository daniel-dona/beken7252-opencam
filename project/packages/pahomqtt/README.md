# paho-mqtt
[MQTT](http://mqtt.org/)（Message Queuing Telemetry Transport，消息队列遥测传输协议），是一种基于发布/订阅（publish/subscribe）模式的“轻量级”通讯协议，该协议构建于TCP/IP协议上，由 IBM 在 1999 年发布。

## 1、介绍
[Paho MQTT](http://www.eclipse.org/paho/downloads.php) 是 Eclipse 实现的 MQTT 协议的客户端，本软件包是在 Eclipse [paho-mqtt](https://github.com/eclipse/paho.mqtt.embedded-c) 源码包的基础上设计的一套 MQTT 客户端程序。

RT-Thread MQTT 客户端功能特点：
- 断线自动重连
- pipe 模型，非阻塞 API
- 事件回调机制
- TLS 加密传输

## 2、获取方式
- 使用 menuconfig
```
  RT-Thread online packages --->
      IOT internet of things --->
          [*] Paho MQTT: Eclipse Paho MQTT C/C++ client for Embedded platforms
          [*]     Enable MQTT example
          [ ]     Enable MQTT test
          [ ]     Enable support tls protocol
          (1)     Max pahomqtt subscribe topic handlers
                  pahomqtt version (latest_version)
```

## 3、示例介绍
### 3.1 获取示例

- 配置使能示例选项 `Enable MQTT example`
- 配置使能测试例程 `Enable MQTT test`
- 配置使能 TLS 安全传输选项 `Enable support tls protocol`
- 设置 MQTT 能订阅的最大 topic 主题数量 `Max pahomqtt subscribe topic handlers`
- 配置包版本选为最新版 `latest_version`

![](./doc/image/paho-mqtt-menuconfig.png)

### 3.2 运行示例
`samples/mqtt_sample.c` 例程提供了一个基础的 MQTT 发布订阅演示，在 RT-Thread MSH 中运行 MQTT 示例，需要开启 MSH 的支持。

测试服务器使用 Eclipse 的测试服务器，地址 `iot.eclipse.org` ，端口 `1883`.

- 开启 MSH  
使用 menuconfig 开启 MSH 支持：
```
  RT-Thread Components --->
      Command shell --->
          [*] Using module shell
          [*]     Using module shell in default
          [ ]     Only using module shell
```
- MSH log

启动 MQTT 客户端：
```
msh />
msh />mq_start
inter mqtt_connect_callback! 
ipv4 address port: 1883
[MQTT] HOST =  'iot.eclipse.org'
msh />[MQTT] Subscribe #0 /mqtt/test OK!
inter mqtt_online_callback! 
msh />
```
发布消息：
```
msh />
msh />mq_pub hello-rtthread
msh />mqtt sub callback: /mqtt/test hello-rtthread

msh />
```

## 4、 测试介绍

### 4.1 使能测试程序

`tests/mqtt_test.c` 测试程序提供了一个`客户端`、`服务器`稳定性测试的例程，
用户可以通过简单配置 MQTT 服务器 URI，QOS 消息服务质量，既可完成 MQTT 客户端和服务器端的测试。

使用 MQTT 测试程序需要在 `menuconfig`  中使能 `Enable MQTT test`。

- 配置使能测试例程选项 `Enable MQTT test`

### 4.2 修改测试程序

将 `tests/mqtt_test.c` 程序中的如下配置，对应修改为您要测试的 MQTT 服务器配置信息即可。

```
#define MQTT_TEST_SERVER_URI    "tcp://iot.eclipse.org:1883"
#define MQTT_CLIENTID           "rtthread-mqtt"
#define MQTT_USERNAME           "admin"
#define MQTT_PASSWORD           "admin"
#define MQTT_SUBTOPIC           "/mqtt/test"
#define MQTT_PUBTOPIC           "/mqtt/test"
#define MQTT_WILLMSG            "Goodbye!"
#define MQTT_TEST_QOS           1
```

### 4.3 运行测试程序

- 使用 `mqtt_test start` 命令启动 MQTT 测试程序
- 使用 `mqtt_test stop` 命令停止 MQTT 测试程序

运行日志如下所示：

```
msh />mqtt_test start
[tls]mbedtls client struct init success...
[MQTT] ipv4 address port: 1884

...

[tls]Certificate verified success...
[MQTT] tls connect success...
[MQTT] Subscribe #0 /mqtt/test OK!
test start at '946725803'
==== MQTT Stability test ====
Server: ssl://yourserverhost.com:1884
QoS   : 1
Test duration(sec)            : 49463
Number of published  packages : 98860
Number of subscribed packages : 98860
Packet loss rate              : 0.00%
Number of reconnections       : 0
==== MQTT Stability test stop ====
```

## 5、 关键代码及 API 说明

### 5.1 订阅列表

paho mqtt 中采用订阅列表的形式进行多个 Topic 的订阅，订阅列表存储在 `MQTTClient` 结构体实例中，在 MQTT 启动前配置，如下所示：

```
... // 省略代码

MQTTClient client;

... // 省略代码

/* set subscribe table and event callback */
client.messageHandlers[0].topicFilter = MQTT_SUBTOPIC;
client.messageHandlers[0].callback = mqtt_sub_callback;
client.messageHandlers[0].qos = QOS1;
```
详细的代码参考 `samples/mqtt_sample.c` 中的 `mq_start` 函数。

订阅列表的最大数量是由 `menuconfig` 中的 `Max pahomqtt subscribe topic handlers` 选项进行配置的。 

### 5.2 callback
paho-mqtt 使用 callback 的方式向用户提供 MQTT 的工作状态以及相关事件的处理， 在 `MQTTClient` 结构体实例中注册使用。

|callback 名称                           |描述|
|:-----                                  |:----|
|connect_callback                        |MQTT 连接成功的回调|
|online_callback                         |MQTT 客户端成功上线的回调|
|offline_callback                        |MQTT 客户端掉线的回调|
|defaultMessageHandler                   |默认的订阅消息接收回调|
|messageHandlers[x].callback             |订阅列表中对应的订阅消息接收回调|

用户可以使用 `defaultMessageHandler` 回调默认处理接收到的订阅消息，也可以使用 `messageHandlers` 订阅列表，为 `messageHandlers` 数组中对应的每一个 topic 提供一个独立的订阅消息接收回调。

### 5.3 MQTT_URI

paho-mqtt 中提供了 uri 解析功能，可以解析域名地址、ipv4和ipv6地址，可解析 `tcp://` 和 `ssl://` 类型的 URI，用户需要按照要求填写可用的 URI 即可。

示例 URI：
```
domain 类型
tcp://iot.eclipse.org:1883

ipv4 类型
tcp://192.168.10.1:1883
ssl://192.168.10.1:1884

ipv6 类型
tcp://[fe80::20c:29ff:fe9a:a07e]:1883
ssl://[fe80::20c:29ff:fe9a:a07e]:1884
```

### 5.4 paho_mqtt_start 接口
- 功能： 启动 MQTT 客户端。

- 函数原型：
```C
int paho_mqtt_start(MQTTClient *client)
```
- 函数参数：

|参数                               |描述|
|:-----                             |:----|
|client                             |MQTT 客户端实例对象|
|return                             |0 : 成功; 其他 : 失败|

### 5.5 MQTTPublish 接口
- 功能： 向指定的 topic 主题发布 MQTT 消息。

- 函数原型：
```C
int MQTTPublish(MQTTClient *c, const char *topicName, MQTTMessage *message)
```
- 函数参数：

|参数                               |描述|
|:-----                             |:----|
|c                                  |MQTT 客户端实例对象|
|topicName                          |MQTT 消息发布主题|
|message                            |MQTT 消息内容|
|return                             |0 : 成功; 其他 : 失败|

## 6、注意事项

- 正确填写 `MQTT_USERNAME` 和 `MQTT_PASSWORD`  
如果 `MQTT_USERNAME` 和 `MQTT_PASSWORD` 填写错误，MQTT 客户端无法正确连接到 MQTT 服务器。

## 7、参考资料

- [MQTT 官网](http://mqtt.org/)
- [Paho 官网](http://www.eclipse.org/paho/downloads.php)
- [IBM MQTT 介绍](https://www.ibm.com/developerworks/cn/iot/iot-mqtt-why-good-for-iot/index.html)
- [Eclipse paho.mqtt 源码](https://github.com/eclipse/paho.mqtt.embedded-c)
