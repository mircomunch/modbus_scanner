#define TEST
#define START_ADDR 0x47AF

#ifdef TEST
    #define DEBUG
    #define MQTT_TOPIC "test/modbus_scanner/"
#else
    #define MQTT_AUTH
    #define MQTT_TOPIC "modbus_scanner/"
#endif

// Serial debug
// #define DEBUG
#ifdef DEBUG
    #define SERIAL_DEBUG Serial
    #define SERIAL_DEBUG_SPEED 115200
#endif

// ESP32 Name
#define BOARD_ID "esp32_ccScanner"

// WiFi settings
// #define WIFI_SSID "Greater"
// #define WIFI_PASSWORD "greater24"

// AP Mode Credentials
#define AP_SSID "ESP32_modbus_scanner"
#define AP_PSW "A1234567"
#define HOSTNAME "esp32"

// Homepgae placeholders
#define plh_current_time "{{plh_current_time}}"
#define plh_last_address "{{plh_last_address}}"
#define plh_last_power   "{{plh_last_power}}"
#define plh_wifi_status  "{{plh_wifi_status}}"

//TIMER Settings
#define PUBLISH_PERIOD 10000
#define READ_POWER_PERIOD 5000

// MQTT configurations
#define MQTT_AUTH
#ifdef MQTT_AUTH
	#define MQTT_HOST "350fd0725fa14a069e04d387121e69f7.s2.eu.hivemq.cloud"
	#define MQTT_PORT 8883
	#define MQTT_USER "admin"
	#define MQTT_PASSWORD "Admin123"
	#define MQTT_CERT_EN true // define if certificate needed for broker connection
#else
	#define MQTT_HOST "broker.hivemq.com"
	#define MQTT_PORT 1883
#endif

#define MQTT_PUBLISH_TOPIC MQTT_TOPIC "data"
#define MQTT_PUBLISH_POWER_TOPIC MQTT_TOPIC "power"
#define MQTT_DATETIME_TOPIC MQTT_TOPIC "datetime"
#define MQTT_PUBLISH_MESSAGE_MAX_SIZE 65536/2 // MQTT publish max size = 256MB
#define MQTT_QOS_SUB 1

// Number of tentatives for each readings
#define READ_TENTATIVE 2

// MAX 485 pin DE, not(RE) connected togheter
#define PIN_RE_DE 27
#define PIN_RO 16 // RX
#define PIN_DI 17 // TX

// TRACER Modbus serial comunication
#define TRACER_SERIAL_STREAM Serial2
#define TRACER_SERIAL_STREAM_SPEED 115200
#define TRACER_MODBUS_ID 0x01

// JSY-MK-194G pin
#define JSY_PIN_RX 22  // RX
#define JSY_PIN_TX 23  // TX
#define JSY_CLAMP 1

// JSY-MK-194G Modbus serial comunication
#define JSY_SERIAL_STREAM Serial1
#define JSY_SERIAL_STREAM_SPEED 4800
#define JSY_MODBUS_ID 0x01