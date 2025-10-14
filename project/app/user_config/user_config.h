// Init script

#define INIT_SCRIPT_FILE "/sd/init.msh"



// JSON and/or EasyFlash config locations

// This include camera board definitions relating to GPIOs
#define BOARD_CONFIG_FILE "/sd/config/board.json"
#define BOARD_CONFIG_EF_KEY "board"

// Enabled services
#define SERVICES_CONFIG_FILE "/sd/config/services.json"
#define SERVICES_CONFIG_EF_KEY "services"

// WLAN STA and AP config
#define WLAN_CONFIG_FILE "/sd/config/wlan.json"
#define WLAN_CONFIG_EF_KEY "wlan"

// Camera sensor config
#define SENSOR_CONFIG_FILE "/sd/config/sensor.json"
#define SENSOR_CONFIG_EF_KEY "sensor"

// Telnet service config
#define TELNET_CONFIG_FILE "/sd/config/telnet.json"
#define TELNET_CONFIG_EF_KEY "telnet"

// MQTT service config (needed for HA)
#define MQTT_CONFIG_FILE "/sd/config/mqtt.json"
#define MQTT_CONFIG_EF_KEY "mqtt"

// Video and audio recording (to SD) config
#define RECORD_CONFIG_FILE "/sd/config/record.json"
#define RECORD_CONFIG_EF_KEY "record"

// Video streaming config
#define VIDEO_STREAM_CONFIG_FILE "/sd/config/video_stream.json"
#define VIDEO_STREAM_CONFIG_EF_KEY "video_stream"

// Audio streaming config
#define AUDIO_STREAM_CONFIG_FILE "/sd/config/audio_stream.json"
#define AUDIO_STREAM_CONFIG_EF_KEY "audio_stream"

// HomeAssistant integration config
#define HOMEASSISTANT_CONFIG_FILE "/sd/config/homeassistant.json"
#define HOMEASSISTANT_CONFIG_EF_KEY "homeassistant"


typedef struct {
    const char* file_path;
    const char* ef_key;
} config_mapping_t;

const config_mapping_t config_mappings[] = {
    {WLAN_CONFIG_FILE, WLAN_CONFIG_EF_KEY},
    {SENSOR_CONFIG_FILE, SENSOR_CONFIG_EF_KEY},
    {TELNET_CONFIG_FILE, TELNET_CONFIG_EF_KEY},
    {MQTT_CONFIG_FILE, MQTT_CONFIG_EF_KEY},
    {RECORD_CONFIG_FILE, RECORD_CONFIG_EF_KEY},
    {VIDEO_STREAM_CONFIG_FILE, VIDEO_STREAM_CONFIG_EF_KEY},
    {AUDIO_STREAM_CONFIG_FILE, AUDIO_STREAM_CONFIG_EF_KEY},
    {SERVICES_CONFIG_FILE, SERVICES_CONFIG_EF_KEY},
    {HOMEASSISTANT_CONFIG_FILE, HOMEASSISTANT_CONFIG_EF_KEY},
    {BOARD_CONFIG_FILE, BOARD_CONFIG_EF_KEY}
};

#define CONFIG_MAPPINGS_COUNT (sizeof(config_mappings) / sizeof(config_mappings[0]))
