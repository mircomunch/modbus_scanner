# modbus_scanner
Understand the solar charge-controller register content by scanning all available addresses.


Usually, commercial charge-controller devices, are already integrated with current/voltage sensors and can also be configured differently.

To interact with it you should know in which registers read or write; if the vendor is not providing any technical documentation you should scan them by yourself.

Using the modbus scanner, you will not only understand the available register but also monitor them trying to figure out the content of each one.

## Overview
In this project the hardware and software features will be exploited in order to retreive all available Modbus addresses that can reply to a read or write command and once collected transmitted to an MQTT topic.
After reaching the MQTT broker, those data will be stored by an indipendent Node-RED application in order to have multiple records over time and try to understand all funcionalities.

### Setup
This system will work with the following needed setup:
- Charge controller with serial port communication RS-485 using Modbus protocol
- ESP32 and related Stripboard for wiring up all components
- - MAX-485 board
  - Ethernet port
  - Wires, pin header
- Ethernet cable
- USB cable (just for first code upload)
- WiFi router with internet access
- PC / Raspberry Pi with Node-RED installed and internet connection

To obtain a working system:
1) Have a solar running system with a charge controller integrated with serial communication port (Modbus over RS-485)
2) Have a router WiFi with known SSID and password
3) Create the stripboard for ESP32 in order to connect the Ethernet cable
4) Connect the charge controller to the ethernet port on the stripboard
5) Download the project and modify the "setup.h" file in the firmware code with your WiFi parameters
6) Plug the ESP32 on the stripboard and upload the firmware with Arduino IDE
7) Check if the code is being executed correctly with the serial monitor
8) Install Node-RED on a separate server machine and upload the Node-RED application "modbus_scanner.json" 

## Stripboard

## Software
