#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <rtthread.h>
#include "rtos_pub.h"
#include "error.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME    "mqtt.service"
#define DBG_LEVEL            DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include <cJSON.h>
#include <cJSON_util.h>
#include "id.h"

#include "mqtt_client.h"
#include "temp_detect_pub.h"
#include "wlan_dev.h"
#include "video_transfer.h"
#include "tiny_base64.h"

/**
 * MQTT URI farmat:
 * domain mode
 * tcp://iot.eclipse.org:1883
 *
 * ipv4 mode
 * tcp://192.168.10.1:1883
 * ssl://192.168.10.1:1884
 *
 * ipv6 mode
 * tcp://[fe80::20c:29ff:fe9a:a07e]:1883
 * ssl://[fe80::20c:29ff:fe9a:a07e]:1884
 */

#define MQTT_CONFIG_FILE        "/sd/config/mqtt_settings.json"
#define MQTT_URI                "tcp://192.168.10.1:1883" //"tcp://iot.eclipse.org:1883"
//#define MQTT_SUBTOPIC           "/mqtt/test1"
//#define MQTT_PUBTOPIC           "/mqtt/test1"
#define MQTT_PUBTOPIC_PREFIX    "opencam"
#define MQTT_HASS_PREFIX        "homeassistant"

//#define MQTT_WILL_SUBTOPIC      "/mqtt/will"
//#define MQTT_WILLMSG            "Goodbye!"
//#define MQTT_USERNAME           "admin"
//#define MQTT_PASSWORD           "admin"
//#define MQTT_CLIENTID           "rtthread-mqtt1"

/* define MQTT client context */
static mqtt_client client;
static int is_started = 0;
//static uint32_t reciver_num=0;

// Metrics thread
beken_thread_t  mqtt_metrics_thread_handle;
beken_thread_t  mqtt_camera_feed_thread_handle;

static int mqtt_metrics_thread(void *arg);
static int mqtt_camera_feed_thread(void *arg);

static void mqtt_hass_discover();

#if 0
static void mqtt_sub_callback(mqtt_client *c, message_data *msg_data)
{
    reciver_num++;
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    #if 1
    LOG_D("mqtt sub callback: num:%d  %.*s %.*s",reciver_num,
               msg_data->topic_name->lenstring.len,
               msg_data->topic_name->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
    #endif
    //rt_kprintf("reciver_num:%d,topic:%.*s\r\n",reciver_num,msg_data->topic_name->lenstring.len,msg_data->topic_name->lenstring.len);
}

static void mqtt_sub_default_callback(mqtt_client *c, message_data *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt sub default callback: %.*s %.*s",
               msg_data->topic_name->lenstring.len,
               msg_data->topic_name->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
}
#endif

static void mqtt_connect_callback(mqtt_client *c){
    LOG_D("MQTT service connecting...");
}

static void mqtt_online_callback(mqtt_client *c){
    LOG_D("MQTT service connected!\r\n");

    mqtt_hass_discover();
}

static void mqtt_offline_callback(mqtt_client *c){
    LOG_D("MQTT service disconnected!");
}

static int mqtt_service_start(int argc, char **argv){
    
    /* init condata param by using MQTTPacket_connectData_initializer */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;

    if (argc != 1)
    {
        rt_kprintf("mqtt_start    --start a mqtt worker thread.\n");
        return -1;
    }

    if (is_started)
    {
        LOG_E("mqtt client is already connected.");
        return -1;
    }
    /* config MQTT context param */
    {
        client.isconnected = 0;
        client.uri = MQTT_URI;

        /* config connect param */
        memcpy(&client.condata, &condata, sizeof(condata));
        client.condata.clientID.cstring = get_device()->name;
        //client.condata.username.cstring = MQTT_USERNAME;
        //client.condata.password.cstring = MQTT_PASSWORD;
        client.condata.keepAliveInterval = 60;
        client.condata.cleansession = 1;

        /* config MQTT will param. */
        //client.condata.willFlag = 1;
        //client.condata.will.qos = 1;
        //client.condata.will.retained = 0;
        //client.condata.will.topicName.cstring = MQTT_WILL_SUBTOPIC;
        //client.condata.will.message.cstring = MQTT_WILLMSG;

        /* malloc buffer. */
        client.buf_size = client.readbuf_size = 30 * 1024;
        client.buf = rt_calloc(1, client.buf_size);
        client.readbuf = rt_calloc(1, client.readbuf_size);
        if (!(client.buf && client.readbuf))
        {
            LOG_E("no memory for MQTT client buffer!");
            return -1;
        }

        /* set event callback function */
        client.connect_callback = mqtt_connect_callback;
        client.online_callback = mqtt_online_callback;
        client.offline_callback = mqtt_offline_callback;

        /* set subscribe table and event callback */
        //client.message_handlers[0].topicFilter = rt_strdup(MQTT_SUBTOPIC);
        //client.message_handlers[0].callback = mqtt_sub_callback;
        //client.message_handlers[0].qos = QOS1;

        /* set default subscribe event callback */
        //client.default_message_handlers = mqtt_sub_default_callback;
    }
    
    {
      int value;
      uint16_t u16Value;
      value = 5;
      paho_mqtt_control(&client, MQTT_CTRL_SET_CONN_TIMEO, &value);
      value = 5;
      paho_mqtt_control(&client, MQTT_CTRL_SET_MSG_TIMEO, &value);
      value = 5;
      paho_mqtt_control(&client, MQTT_CTRL_SET_RECONN_INTERVAL, &value);
      value = 30;
      paho_mqtt_control(&client, MQTT_CTRL_SET_KEEPALIVE_INTERVAL, &value);
      u16Value = 3;
      paho_mqtt_control(&client, MQTT_CTRL_SET_KEEPALIVE_COUNT, &u16Value);
    }

    /* run mqtt client */
    paho_mqtt_start(&client, 8196, 20);
    is_started = 1;

    OSStatus ret = rtos_create_thread(&mqtt_metrics_thread_handle,
                             10,
                             "mqtt_metrics",
                             (beken_thread_function_t)mqtt_metrics_thread,
                             (unsigned short) 2*1024,
                             (beken_thread_arg_t)0);
    if (kNoErr != ret){
        LOG_E("Metrics thread start failed\r\n");
        
    }else{
        LOG_I("Metrics thread start\r\n");
    }

    #if 1

    OSStatus ret2 = rtos_create_thread(&mqtt_camera_feed_thread_handle,
                             20,
                             "mqtt_metrics",
                             (beken_thread_function_t)mqtt_camera_feed_thread,
                             (unsigned short) 2*1024,
                             (beken_thread_arg_t)0);
    if (kNoErr != ret2){
        LOG_E("Camera feed thread start failed\r\n");
        
    }else{
        LOG_I("Camera feed thread start\r\n");
    }

    #endif

    //mqtt_hass_discover();

    return 0;
}

static int mqtt_service_stop(int argc, char **argv){
    if (argc != 1)
    {
        rt_kprintf("mqtt_stop    --stop mqtt worker thread and free mqtt client object.\n");
    }

    is_started = 0;

    return paho_mqtt_stop(&client);
}

#if 0
static int mqtt_publish(int argc, char **argv)
{
    if (is_started == 0)
    {
        LOG_E("mqtt client is not connected.");
        return -1;
    }

    if (argc == 2)
    {
        paho_mqtt_publish(&client, QOS0, MQTT_PUBTOPIC, argv[1], strlen(argv[1]));
    }
    else if (argc == 3)
    {
        paho_mqtt_publish(&client, QOS0, argv[1], argv[2],strlen(argv[2]));
    }
    else
    {
        rt_kprintf("mqtt_publish <topic> [message]  --mqtt publish message to specified topic.\n");
        return -1;
    }

    return 0;
}

static void mqtt_new_sub_callback(mqtt_client *client, message_data *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt new subscribe callback: %.*s %.*s",
               msg_data->topic_name->lenstring.len,
               msg_data->topic_name->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
}

static int mqtt_subscribe(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("mqtt_subscribe [topic]  --send an mqtt subscribe packet and wait for suback before returning.\n");
        return -1;
    }
	
	if (is_started == 0)
    {
        LOG_E("mqtt client is not connected.");
        return -1;
    }

    return paho_mqtt_subscribe(&client, QOS0, argv[1], mqtt_new_sub_callback);
}

static int mqtt_unsubscribe(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("mqtt_unsubscribe [topic]  --send an mqtt unsubscribe packet and wait for suback before returning.\n");
        return -1;
    }
	
	if (is_started == 0)
    {
        LOG_E("mqtt client is not connected.");
        return -1;
    }

    return paho_mqtt_unsubscribe(&client, argv[1]);
}
#endif

static void mqtt_hass_discover(void){

    char discover_topic[64];
    char camera_topic[64];
    char metrics_topic[64];

    char sensor_uuid[32];


    DeviceInfo *device = get_device();

    snprintf(discover_topic, sizeof(discover_topic), "%s/device/%s/config", MQTT_HASS_PREFIX, device->name);

    snprintf(camera_topic, sizeof(camera_topic), "%s/%s/feed", MQTT_PUBTOPIC_PREFIX, device->name);
    snprintf(metrics_topic, sizeof(camera_topic), "%s/%s/metrics", MQTT_PUBTOPIC_PREFIX, device->name);


    //<discovery_prefix>/<component>/[<node_id>/]<object_id>/config

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        // Handle error
        return;
    }

    // Create "dev" object
    cJSON *dev = cJSON_CreateObject();
    cJSON_AddStringToObject(dev, "ids", device->name);
    cJSON_AddStringToObject(dev, "name", device->name);
    cJSON_AddStringToObject(dev, "mf", "OpenCam");
    cJSON_AddStringToObject(dev, "mdl", "BK7252");
    cJSON_AddStringToObject(dev, "sw", "1.0");
    //cJSON_AddStringToObject(dev, "hw", "1.0rev2");
    cJSON_AddItemToObject(root, "dev", dev);

    // Create "o" object
    cJSON *o = cJSON_CreateObject();
    cJSON_AddStringToObject(o, "name", "OpenCam HASS Integration");
    cJSON_AddStringToObject(o, "sw", "1.0");
    cJSON_AddStringToObject(o, "url", "https://github.com/daniel-dona/beken7252-opencam");
    cJSON_AddItemToObject(root, "o", o);

    // Create "cmps" object with nested component
    cJSON *cmps = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "cmps", cmps);

    cJSON *camera_component = cJSON_CreateObject();
    cJSON_AddStringToObject(camera_component, "p", "camera");
    cJSON_AddStringToObject(camera_component, "name", "Camera feed");
    cJSON_AddStringToObject(camera_component, "device_class", "camera");
    cJSON_AddStringToObject(camera_component, "topic", camera_topic);
    snprintf(sensor_uuid, sizeof(sensor_uuid), "%s-0001", device->name);
    cJSON_AddStringToObject(camera_component, "unique_id", sensor_uuid);
    //cJSON_AddStringToObject(camera_component, "image_encoding", "b64");
    cJSON_AddItemToObject(cmps, "camera_feed", camera_component);
    
    cJSON *temperature_component = cJSON_CreateObject();
    cJSON_AddStringToObject(temperature_component, "p", "sensor");
    cJSON_AddStringToObject(temperature_component, "name", "Chip raw temperature");
    cJSON_AddStringToObject(temperature_component, "device_class", "temperature");
    cJSON_AddStringToObject(temperature_component, "state_topic", metrics_topic);
    cJSON_AddStringToObject(temperature_component, "unit_of_measurement", "°C");
    cJSON_AddStringToObject(temperature_component, "value_template", "{{ value_json.temperature}}");
    snprintf(sensor_uuid, sizeof(sensor_uuid), "%s-0002", device->name);
    cJSON_AddStringToObject(temperature_component, "unique_id", sensor_uuid);
    cJSON_AddItemToObject(cmps, "sensor_temperature", temperature_component);
    //cJSON_AddItemToObject(root, "cmps", cmps);

    cJSON *heap_component = cJSON_CreateObject();
    cJSON_AddStringToObject(heap_component, "p", "sensor");
    cJSON_AddStringToObject(heap_component, "name", "Heap size");
    cJSON_AddStringToObject(heap_component, "device_class", "data_size");
    cJSON_AddStringToObject(heap_component, "state_topic", metrics_topic);
    cJSON_AddStringToObject(heap_component, "unit_of_measurement", "B");
    cJSON_AddStringToObject(heap_component, "value_template", "{{ value_json.memory.pools[1].free}}");
    snprintf(sensor_uuid, sizeof(sensor_uuid), "%s-0003", device->name);
    cJSON_AddStringToObject(heap_component, "unique_id", sensor_uuid);
    cJSON_AddItemToObject(cmps, "sensor_heap", heap_component);
    //cJSON_AddItemToObject(root, "cmps", cmps);

    cJSON *uptime_component = cJSON_CreateObject();
    cJSON_AddStringToObject(uptime_component, "p", "sensor");
    cJSON_AddStringToObject(uptime_component, "name", "Uptime");
    cJSON_AddStringToObject(uptime_component, "device_class", "duration");
    cJSON_AddStringToObject(uptime_component, "state_topic", metrics_topic);
    cJSON_AddStringToObject(uptime_component, "unit_of_measurement", "s");
    cJSON_AddStringToObject(uptime_component, "value_template", "{{ value_json.uptime}}");
    snprintf(sensor_uuid, sizeof(sensor_uuid), "%s-0004", device->name);
    cJSON_AddStringToObject(uptime_component, "unique_id", sensor_uuid);
    cJSON_AddItemToObject(cmps, "sensor_uptime", uptime_component);

    
    // Add remaining top-level fields
    //cJSON_AddStringToObject(root, "state_topic", "sensor/state");
    //cJSON_AddNumberToObject(root, "qos", 2);

    // Convert to string and use it
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str != NULL) {
        // Use the JSON string
        
        if(client.isconnected){
            LOG_I("Publishing HASS discovery to topic %s\n", discover_topic);

            paho_mqtt_publish(&client, QOS1, discover_topic, json_str, strlen(json_str));
        }
        cJSON_free(json_str);
    }

    // Clean up
    cJSON_Delete(root);

    free(device);

}

#define MAX_BUF_SIZE 30 * 1024

static int mqtt_camera_feed_thread(void *arg){

    void *buffer = malloc(MAX_BUF_SIZE);
    //char *base64_buffer = malloc(MAX_BUF_SIZE);

    char *device_name = get_device()->name;
    
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/feed", MQTT_PUBTOPIC_PREFIX, device_name);

    if(buffer != NULLPTR){

        while(1){

            uint32_t frame_size = 0;

            frame_size = video_buffer_read_frame(buffer, MAX_BUF_SIZE);

            //tiny_base64_encode((unsigned char*) base64_buffer, (int * )&encoded_size, (unsigned char*) buffer, frame_size);

            //bk_printf("Frame size: %d \r\n", frame_size);

            if(client.isconnected && frame_size != 0){

                int res = paho_mqtt_publish(&client, QOS1, topic, buffer, frame_size);

                if(res != 0){

                    LOG_D("Frame size: %d, send result %d \r\n", frame_size, res);

                }

            }

            rt_thread_delay(30000);

        }

    }else{
        LOG_E("Unable to reserve buffer!");
    }

    return 0;

}

static cJSON *mqtt_metrics_memory(void){
    // Memory info

    cJSON *mem = cJSON_CreateObject();

    struct rt_object_information *info = (struct rt_object_information*) rt_object_get_information(RT_Object_Class_MemHeap);

    struct rt_list_node *list = &info->object_list;

    struct rt_memheap *mh;
    struct rt_list_node *node;

    cJSON_AddItemToObject(mem, "pools", cJSON_CreateArray());

    for (node = list->next; node != list; node = node->next){
        mh = (struct rt_memheap *)rt_list_entry(node, struct rt_object, list);

        cJSON *item = cJSON_CreateObject();

        cJSON_AddStringToObject(item, "name", mh->parent.name);
        cJSON_AddNumberToObject(item, "size", mh->pool_size);
        cJSON_AddNumberToObject(item, "used", mh->max_used_size);
        cJSON_AddNumberToObject(item, "free", mh->available_size);


        cJSON_AddItemToArray(cJSON_GetObjectItem(mem, "pools"), item);
    }

    return mem;
}

static int mqtt_metrics_thread( void *arg ) {
    int index = 0;

    char *device_name = get_device()->name;
    
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/metrics", MQTT_PUBTOPIC_PREFIX, device_name);

    while (1) {
        // Create cJSON object
        cJSON *root = cJSON_CreateObject();
        cJSON *mem = mqtt_metrics_memory();
        
        uint32_t temperature = 0;
        uint32_t status = temp_single_get_current_temperature(&temperature);

        if (status != 0){
            temperature = 0;
        }

        struct rt_wlan_device *wlan;
        wlan = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_STA_NAME);

        uint32_t rssi = rt_wlan_get_rssi(wlan);

        // Add all fields to JSON object
        cJSON_AddStringToObject(root, "device", device_name);
        cJSON_AddItemReferenceToObject(root, "memory", mem);
        cJSON_AddNumberToObject(root, "temperature", temperature);
        cJSON_AddNumberToObject(root, "rssi", rssi);
        cJSON_AddNumberToObject(root, "uptime", ((double) rt_tick_get())/1000.0);
        
        // Convert JSON object to string
        char *json_str = cJSON_PrintUnformatted(root);
        if (json_str != NULL) {
            // Check if the string fits in our buffer
            if(client.isconnected){
                paho_mqtt_publish(&client, QOS1, topic, json_str, strlen(json_str));
            }
            cJSON_free(json_str);

        } else {
            rt_kprintf("Error: Failed to convert JSON to string\r\n");
        }
        
        // Clean up
        cJSON_Delete(root);
        cJSON_Delete(mem);
        
        index++;
        rt_thread_delay(60000);
    }
    return 0;
}

//MSH_CMD_EXPORT(mq_pub_test, publish mqtt msg);

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(mqtt_service_start, Start MQTT service);
MSH_CMD_EXPORT(mqtt_service_stop, Stop MQTT service);

MSH_CMD_EXPORT(mqtt_hass_discover, mqtt_hass_discover);
MSH_CMD_EXPORT(mqtt_camera_feed_thread, mqtt_camera_feed_thread);
//MSH_CMD_EXPORT(mqtt_stop, stop mqtt client);
//MSH_CMD_EXPORT(mqtt_publish, mqtt publish message to specified topic);
//MSH_CMD_EXPORT(mqtt_subscribe,  mqtt subscribe topic);
//MSH_CMD_EXPORT(mqtt_unsubscribe, mqtt unsubscribe topic);
#endif /* FINSH_USING_MSH */

