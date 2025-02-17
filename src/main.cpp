#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <solarModbus.hpp>
#include <powerReader.hpp>
#include <MQTT.hpp>
#include <setup.hpp>
#include <Preferences.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <TimeLib.h>

#include <html/home.hpp>
#include <html/change.hpp>

char _host_char[sizeof(MQTT_HOST)] = MQTT_HOST;

#ifdef MQTT_AUTH
    char _host_user[sizeof(MQTT_USER)] = MQTT_USER;
    char _host_psw[sizeof(MQTT_PASSWORD)] = MQTT_PASSWORD;
    MQTT mqtt(BOARD_ID, _host_char, MQTT_PORT, MQTT_PUBLISH_MESSAGE_MAX_SIZE, MQTT_CERT_EN, _host_user, _host_psw);
#else
    MQTT mqtt(BOARD_ID, _host_char, MQTT_PORT);
#endif

TimerHandle_t publishTimer;
TimerHandle_t readPowerTimer;
TimerHandle_t publishPowerTimer;
SolarModbus TRACER(PIN_RE_DE, PIN_DI, PIN_RO, READ_TENTATIVE, TRACER_MODBUS_ID, TRACER_SERIAL_STREAM_SPEED, TRACER_SERIAL_STREAM, TRACER_SERIAL_STREAM);
PowerReader JSY_MK(JSY_PIN_TX, JSY_PIN_RX, JSY_SERIAL_STREAM_SPEED, JSY_MODBUS_ID, READ_TENTATIVE, JSY_SERIAL_STREAM, JSY_SERIAL_STREAM);

String mdbMessage = "";
String pwrMessage = "";
bool publishNow;
bool publishPowerNow;
bool readPowerNow;
uint16_t addr = START_ADDR;
bool stopReading = true;
bool stopPwrReading = true;
long mqttReconnectAttempt = 0;
long wifiReconnectAttempt = 0;

WebServer server(80);

Preferences preferences;
String ssid;
String psw;
const char *ap_ssid = AP_SSID;
const char *ap_psw = AP_PSW;
const char *hostname = HOSTNAME;

String wifiStatus = "-";
String currentTime = "-";
String lastAddress = "-";
String lastPower = "-";

void publishNowCallback()
{
    publishNow = true;
}
void publishPowerNowCallback()
{
    publishPowerNow = true;
}
void readPowerNowCallback()
{
    readPowerNow = true;
}

void datetimeCallback(const char *topic, byte *payload, unsigned int length)
{
    // Convert payload to a string
    String datetimePayload = "";
    for (unsigned int i = 0; i < length; i++)
    {
        datetimePayload += (char)payload[i];
    }
    #ifdef DEBUG
        SERIAL_DEBUG.print("Received datetime: ");
        SERIAL_DEBUG.println(datetimePayload);
    #endif

    uint64_t seconds = datetimePayload.toDouble();

    if (seconds > 0)
    {
        setTime(seconds);
        stopReading = false;
		stopPwrReading = false;
		xTimerStart(publishTimer, 0);
		xTimerStart(publishPowerTimer, 0);
		xTimerStart(readPowerTimer, 0);
	}
    else
    {
        #ifdef DEBUG
            SERIAL_DEBUG.println("Failed to parse datetime payload.");
        #endif
    }
}

void messageHandler(const char *topic, byte *payload, unsigned int length)
{
    #ifdef DEBUG
        SERIAL_DEBUG.print("Message arrived on topic: ");
        SERIAL_DEBUG.println(topic);
    #endif

    // Manage callback for each topic subscribed
    if (String(topic) == MQTT_DATETIME_TOPIC)
    {
        datetimeCallback(topic, payload, length);
    }
    else
    {
        #ifdef DEBUG
            SERIAL_DEBUG.println("No handler for this topic.");
        #endif
    }
}

void handle_homepage()
{
    String homepage = String(html_home);
    wifiStatus = WiFi.status() == WL_CONNECTED ? "Connected" : "Not connected";

    time_t currT = now();
    currentTime = 
        String(hour(currT)) + ":" + String(minute(currT)) + ":" + String(second(currT))
        + " " +
        String(day(currT)) + "/" + String(month(currT)) + "/" + String(year(currT));
    homepage.replace(plh_current_time, currentTime);
    homepage.replace(plh_last_address, lastAddress);
	homepage.replace(plh_last_power, lastPower);
	homepage.replace(plh_wifi_status, wifiStatus);
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", homepage.c_str());
}

void handle_change_wifi()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", html_change);
    String new_ssid = server.arg("ssid");
    String new_psw = server.arg("psw");
    #ifdef DEBUG
        SERIAL_DEBUG.println("New Wi-Fi credentials: " + new_ssid + " - " + new_psw);
    #endif
    preferences.begin("credentials", false);
    preferences.putString("ssid", new_ssid);
    preferences.putString("psw", new_psw);
    preferences.end();
    delay(3000);
    ESP.restart();
}

void handle_NotFound()
{
    server.sendHeader("Connection", "close");
    server.send(404, "text/plain", "Not found");
}

void handleUpdate()
{
    size_t fsize = UPDATE_SIZE_UNKNOWN;
    if (server.hasArg("size"))
    {
        fsize = server.arg("size").toInt();
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Receiving Update: %s, Size: %d\n", upload.filename.c_str(), fsize);
        if (!Update.begin(fsize))
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
            Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
        }
        else
        {
            Serial.printf("%s\n", Update.errorString());
        }
    }
}

void handleUpdateEnd()
{
    server.sendHeader("Connection", "close");
    if (Update.hasError())
    {
        server.send(502, "text/plain", Update.errorString());
    }
    else
    {
        server.sendHeader("Refresh", "10");
        server.sendHeader("Location", "/");
        server.send(307);
        ESP.restart();
    }
}

// SETUP
void set()
{
    #ifdef DEBUG
        SERIAL_DEBUG.begin(SERIAL_DEBUG_SPEED);
    #endif
    preferences.begin("credentials", false);
    ssid = preferences.getString("ssid", "");
    psw = preferences.getString("psw", "");
    preferences.end();

    if (ssid == "" || psw == "")
    {
        #ifdef DEBUG
            SERIAL_DEBUG.println("No values saved for ssid or password");
        #endif
    }
    else
    {
        #ifdef DEBUG
            SERIAL_DEBUG.println("Searching for Wi-Fi: " + ssid);
        #endif
    }
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid, ap_psw);
    WiFi.softAPsetHostname(hostname);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname);
    /*use mdns for host name resolution*/
    if (!MDNS.begin(hostname))
    {
        #ifdef DEBUG
            SERIAL_DEBUG.println("Error setting up MDNS responder!");
        #endif
        while (1)
        {
            delay(10000);
            ESP.restart();
        }
    }
    #ifdef DEBUG
        SERIAL_DEBUG.println("mDNS responder started");
    #endif
    /*return index page which is stored in serverIndex */
    server.on("/", HTTP_GET, handle_homepage);
    server.on("/change-wifi", HTTP_GET, handle_change_wifi);
    server.onNotFound(handle_NotFound);
    /*handling uploading firmware file */
    server.on("/update-firmware", HTTP_POST, []()
              { handleUpdateEnd(); }, []()
              { handleUpdate(); });
    server.on("/favicon.ico", HTTP_GET, []()
              {
        server.sendHeader("Content-Encoding", "gzip");
        server.send_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len); });

    server.begin();
	JSY_MK.begin();

	// Start timer
    publishTimer = xTimerCreate("publishTimer", pdMS_TO_TICKS(PUBLISH_PERIOD), pdFALSE, (void *)0,
                                reinterpret_cast<TimerCallbackFunction_t>(publishNowCallback));
    publishPowerTimer = xTimerCreate("publishPowerTimer", pdMS_TO_TICKS(PUBLISH_PERIOD), pdFALSE, (void *)0,
                                reinterpret_cast<TimerCallbackFunction_t>(publishPowerNowCallback));
    readPowerTimer = xTimerCreate("readPowerTimer", pdMS_TO_TICKS(READ_POWER_PERIOD), pdFALSE, (void *)0,
                                reinterpret_cast<TimerCallbackFunction_t>(readPowerNowCallback));
}

// LOOP
void exec()
{
    // WiFi reconnect logic
    if (WiFi.status() != WL_CONNECTED)
    {
        long now = millis();
        if (now - wifiReconnectAttempt > 5000)
        {
            wifiReconnectAttempt = now;
            #ifdef DEBUG
                SERIAL_DEBUG.println("Reconnecting to WiFi...");
            #endif
            if (WiFi.begin(ssid, psw) == WL_CONNECTED)
            {
                #ifdef DEBUG
                    SERIAL_DEBUG.println("Connected to: " + ssid);
                #endif
                wifiReconnectAttempt = 0;
                return;
            }
        }
    }

    // MQTT reconnect logic
    if (!mqtt.client.connected())
    {
        long now = millis();
        if (now - mqttReconnectAttempt > 5000)
        {
            mqttReconnectAttempt = now;
            if (mqtt.reconnect())
            {
                // Manage subscriptions
                if (mqtt.client.subscribe(MQTT_DATETIME_TOPIC, MQTT_QOS_SUB))
                {
                    mqtt.client.setCallback(messageHandler);
                    #ifdef DEBUG
                        SERIAL_DEBUG.println("Subscribed to: " + String(MQTT_DATETIME_TOPIC));
                    #endif
                }
                mqttReconnectAttempt = 0;
                return;
            }
        }
    }

    mqtt.client.loop();
    server.handleClient();

    // Data reading logic
    char _addr_hex[10];
    sprintf(_addr_hex, "%04X", addr);
    String addr_hex = "0x" + String(_addr_hex);
    lastAddress = addr_hex;
	String resultsMsg = "";
	String resultsPwrMsg = "";

	if (addr <= 0xFFFF & !stopReading & mdbMessage.length() <= MQTT_PUBLISH_MESSAGE_MAX_SIZE * 0.8)
	{
        uint16_t content = 0x0000;
        uint8_t result = TRACER.readCoils(addr, &content);
		resultsMsg = "";
		resultsMsg += "Addr:" + addr_hex + "-Read_Coil-" + TRACER.exceptionDescription(result) + "\n";
        #ifdef DEBUG
            SERIAL_DEBUG.println("Addr: " + addr_hex + " - Read_Coil\t\t- " + TRACER.exceptionDescription(result)) + "\n";
        #endif
        lastAddress += "<br><br>Read_Coil - " + TRACER.exceptionDescription(result) + "<br>";
		if (result == 0)
        {
            mdbMessage += String((int)now()) + "," + String(addr_hex) + ",Read_Coil,fn_1," + String(content) + "\n";
        }
        delay(10);

        result = TRACER.readDiscreteInputs(addr, &content);
		resultsMsg += "Addr:" + addr_hex + "-Read_Discrete_Input-" + TRACER.exceptionDescription(result) + "\n";
        #ifdef DEBUG
            SERIAL_DEBUG.println("Addr: " + addr_hex + " - Read_Discrete_Input\t- " + TRACER.exceptionDescription(result));
        #endif
        lastAddress += "Read_Discrete_Input - " + TRACER.exceptionDescription(result) + "<br>";
		if (result == 0)
        {
            mdbMessage += String((int)now()) + "," + String(addr_hex) + ",Read_Discrete_Input,fn_2," + String(content) + "\n";
        }
        delay(10);

        result = TRACER.readHoldingRegisters(addr, &content, 1);
		resultsMsg += "Addr:" + addr_hex + "-Read_Holding_Registers-" + TRACER.exceptionDescription(result) + "\n";
        #ifdef DEBUG
            SERIAL_DEBUG.println("Addr: " + addr_hex + " - Read_Holding_Registers\t- " + TRACER.exceptionDescription(result));
        #endif
        lastAddress += "Read_Holding_Registers - " + TRACER.exceptionDescription(result) + "<br>";
		if (result == 0)
        {
            mdbMessage += String((int)now()) + "," + String(addr_hex) + ",Read_Holding_Registers,fn_3," + String(content) + "\n";
        }
        delay(10);

        result = TRACER.readInputRegisters(addr, &content, 1, false);
		resultsMsg += "Addr:" + addr_hex + "-Read_Input_Registers-" + TRACER.exceptionDescription(result) + "\n";
        #ifdef DEBUG
            SERIAL_DEBUG.println("Addr: " + addr_hex + " - Read_Input_Registers\t- " + TRACER.exceptionDescription(result));
        #endif
        lastAddress += "Read_Input_Registers - " + TRACER.exceptionDescription(result) + "<br>";
		if (result == 0)
        {
            mdbMessage += String((int)now()) + "," + String(addr_hex) + ",Read_Input_Registers,fn_4," + String(content) + "\n";
        }
        delay(10);

        #ifdef DEBUG
            // SERIAL_DEBUG.println("Addr: " + addr_hex);
        #endif
        addr++;
    }

    // Publish logic
    if (publishNow)
    {
        publishNow = false;

        if (mdbMessage.isEmpty())
        {
            mdbMessage = String((int)now()) + "," + String(addr_hex) + ",NO_NEW_DATA," + resultsMsg;
        }
        else
        {
            #ifdef DEBUG
                SERIAL_DEBUG.println("Sending mdbMessage:");
                SERIAL_DEBUG.println(mdbMessage);
            #endif
        }

        if (!mqtt.publishMessage(MQTT_PUBLISH_TOPIC, mdbMessage, true))
        {
            stopReading = true;
            #ifdef DEBUG
                SERIAL_DEBUG.println("!!! PUBLISH ERROR !!!");
            #endif
        }
        else
        {
            stopReading = false;
            #ifdef DEBUG
                SERIAL_DEBUG.println("--- MESSAGE PUBLISHED ---");
            #endif
            mdbMessage = "";
        }
        xTimerStart(publishTimer, 0);
    }

	if (readPowerNow & !stopPwrReading & pwrMessage.length() <= MQTT_PUBLISH_MESSAGE_MAX_SIZE * 0.8)
	{
		
		// uint8_t result = JSY_MK.test();
		// if (result == 0)
		// {
			pwrMessage += String((int)now()) + ",";
			float voltage, current, power, energy, powerFactor, powerDirection, frequency;
			
			uint8_t result = JSY_MK.readVoltage(JSY_CLAMP, &voltage);
			if (result == 0)
			{
				pwrMessage += String(voltage);
			}
			pwrMessage += ",";
			result = JSY_MK.readCurrent(JSY_CLAMP, &current);
			if (result == 0)
			{
				pwrMessage += String(current);
			}
			pwrMessage += ",";
			result = JSY_MK.readPower(JSY_CLAMP, &power);
			resultsPwrMsg = JSY_MK.exceptionDescription(result);
			if (result == 0)
			{
				pwrMessage += String(power);
				lastPower = String(power);
			}
			else {
				lastPower = resultsPwrMsg;
			}
			pwrMessage += ",";
			result = JSY_MK.readEnergy(JSY_CLAMP, &energy);
			if (result == 0)
			{
				pwrMessage += String(energy);
			}
			pwrMessage += ",";
			result = JSY_MK.readPowerfactor(JSY_CLAMP, &powerFactor);
			if (result == 0)
			{
				pwrMessage += String(powerFactor);
			}
			pwrMessage += ",";
			result = JSY_MK.readPowerdirection(&powerDirection);
			if (result == 0)
			{
				pwrMessage += String(powerDirection);
			}
			pwrMessage += ",";
			result = JSY_MK.readFrequency(&frequency);
			if (result == 0)
			{
				pwrMessage += String(frequency);
			}
			pwrMessage += "\n";
		// }
		// else {
		// 	resultsPwrMsg = JSY_MK.exceptionDescription(result);
		// 	lastPower = resultsPwrMsg;
		// }
	}
    
	if (publishPowerNow || pwrMessage.length() >= MQTT_PUBLISH_MESSAGE_MAX_SIZE * 0.7)
    {
		publishPowerNow = false;

		if (pwrMessage.isEmpty())
		{
			pwrMessage = String((int)now()) + ",NO_NEW_DATA," + resultsPwrMsg;
		}
        else
        {
			#ifdef DEBUG
            SERIAL_DEBUG.println("Sending pwrMessage:");
            SERIAL_DEBUG.println(pwrMessage);
			#endif
        }

		if (!mqtt.publishMessage(MQTT_PUBLISH_POWER_TOPIC, pwrMessage, true))
		{
            stopPwrReading = true;
			#ifdef DEBUG
            SERIAL_DEBUG.println("!!! PUBLISH POWER ERROR !!!");
			#endif
        }
        else
        {
            stopPwrReading = false;
            #ifdef DEBUG
            SERIAL_DEBUG.println("--- MESSAGE POWER PUBLISHED ---");
            #endif
			pwrMessage = "";
		}
        xTimerStart(publishPowerTimer, 0);
    }
}

// Standard loop and setup with loop inside for better watchdog management
void setup() {
    set();
    while(1) {
        exec();
    }
}
void loop() {}