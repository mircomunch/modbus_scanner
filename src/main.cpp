#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <solarModbus.hpp>
#include <MQTT.hpp>

#include <setup.hpp>
char _host_char[sizeof(MQTT_HOST)] = MQTT_HOST;
#ifdef MQTT_AUTH
	char _host_user[sizeof(MQTT_USER)] = MQTT_USER;
	char _host_psw[sizeof(MQTT_PASSWORD)] = MQTT_PASSWORD;
	MQTT mqtt(BOARD_ID, _host_char, MQTT_PORT, MQTT_PUBLISH_MESSAGE_MAX_SIZE, MQTT_CERT_EN, _host_user, _host_psw);
#else
	MQTT mqtt(BOARD_ID, _host_char, MQTT_PORT);
#endif

TimerHandle_t publishTimer;
SolarModbus TRACER(PIN_RE_DE, PIN_DI, PIN_RO, nIter,
                   TRACER_MODBUS_ID, TRACER_SERIAL_STREAM_SPEED, TRACER_SERIAL_STREAM, TRACER_SERIAL_STREAM);

String message = ""; // address,function_name,function_code,content;
bool publishNow;
uint16_t addr = 0x0000;
bool stopReading = true;
long mqttReconnectAttempt = 0;
long wifiReconnectAttempt = 0;

void publishNowCallback()
{
  publishNow = true;
}

void datetimeCallback(const char *topic, byte *payload, unsigned int length)
{
}

void setup()
{
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWiFi connected\nIP address: ");
	Serial.println(WiFi.localIP());
	Serial.println("Init...");
	// mqtt.client.subscribe(MQTT_DATETIME_TOPIC, MQTT_QOS_SUB);
	// mqtt.client.setCallback(datetimeCallback);

	publishTimer = xTimerCreate("publishTimer", pdMS_TO_TICKS(PUBLISH_PERIOD),
								pdFALSE, (void *)0,
								reinterpret_cast<TimerCallbackFunction_t>(publishNowCallback));
}

void loop()
{
	if (WiFi.status() != WL_CONNECTED)
	{
        long now = millis();
        if (now - wifiReconnectAttempt > 5000)
        {
            wifiReconnectAttempt = now;
            Serial.println("Reconnecting to WiFi...");
            if (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) == WL_CONNECTED)
            {
                wifiReconnectAttempt = 0;
                return;
            }
        }
	}

    if (!mqtt.client.connected())
    {
        long now = millis();
        if (now - mqttReconnectAttempt > 5000)
        {
            mqttReconnectAttempt = now;
            // Attempt to reconnect
            if (mqtt.reconnect())
            {
                mqttReconnectAttempt = 0;
                stopReading = false;
                xTimerStart(publishTimer, 0);
                return;
            }
        }
    }

    char _addr_hex[10];
    sprintf(_addr_hex, "%04X", addr);
    String addr_hex = "0x" + String(_addr_hex);
    if (addr <= 0xFFFF & !stopReading & message.length() <= MQTT_PUBLISH_MESSAGE_MAX_SIZE*0.8)
    {
        uint16_t content = 0x0000;
        uint8_t result = TRACER.readCoils(addr, &content);
        if (result == 0)
        {
            String append = String(addr_hex) + ",Read_Coil,fn_1," + String(content) + "\n";
            message += append;
        }
        delay(10);

        result = TRACER.readDiscreteInputs(addr, &content);
        if (result == 0)
        {
            String append = String(addr_hex) + ",Read_Discrete_Input,fn_2," + String(content) + "\n";
            message += append;
        }
        delay(10);

        result = TRACER.readHoldingRegisters(addr, &content, 1);
        if (result == 0)
        {
            String append = String(addr_hex) + ",Read_Holding_Registers,fn_3," + String(content) + "\n";
            message += append;
        }
        delay(10);

        result = TRACER.readInputRegisters(addr, &content, 1, false);
        if (result == 0)
        {
            String append = String(addr_hex) + ",Read_Input_Registers,fn_4," + String(content) + "\n";
            message += append;
        }
        delay(10);

        Serial.println("Addr: " + addr_hex);
        addr++;
    }

    if (publishNow)
    {
        publishNow = false;

        if (message.isEmpty())
        {
            message = "NO_NEW_DATA;" + String(addr_hex);
        }
        else
        {
            Serial.println("Sending message:");
            Serial.println(message);
        }

        if (!mqtt.publishMessage(MQTT_PUBLISH_TOPIC, message, true))
        {
            stopReading = true;
            Serial.println("!!! PUBLISH ERROR !!!");
        }
        else
        {
            stopReading = false;
            Serial.println("--- MESSAGE PUBLISHED ---");
            message = "";
        }
        xTimerStart(publishTimer, 0);
    }
    mqtt.client.loop();
}