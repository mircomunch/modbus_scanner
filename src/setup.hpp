// ESP32 Name
#define BOARD_ID "esp32_mirco"

// WiFi settings
#define WIFI_SSID "Greater"
#define WIFI_PASSWORD "greater23"

//TIMER Settings
#define PUBLISH_PERIOD 2000

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

#define MQTT_TOPIC "modbus_scanner/"
#define MQTT_PUBLISH_TOPIC MQTT_TOPIC "data"
#define MQTT_DATETIME_TOPIC MQTT_TOPIC "datetime"
#define MQTT_PUBLISH_MESSAGE_SIZE 1024
#define MQTT_QOS_SUB 1


// MAX 485 pin DE, not(RE) connected togheter
#define PIN_RE_DE 27
#define PIN_RO 16 // RX
#define PIN_DI 17 //TX

// Number of tentatives for each readings
#define nIter 2

// TRACER Modbus serial comunication
#define TRACER_SERIAL_STREAM Serial2
#define TRACER_SERIAL_STREAM_SPEED 115200

#define TRACER_MODBUS_ID 0x01