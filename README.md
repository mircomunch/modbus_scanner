# modbus_scanner
Understand the solar charge-controller register content by scanning all available addresses.

Usually, commercial charge-controller devices, are already integrated with many sensors and can be setup differently.


## Overview
In this project the hardware and software features will be exploited in order to retreive all available Modbus addresses that can reply to a read or write command and once collected transmitted to an MQTT topic.
After reaching the MQTT broker, those data will be stored by an indipendent server in order to have multiple records over time and try to understand all funcionalities.

### Setup
This system will work with the following needed setup:
- Charge controller with serial port communication RS-485 using Modbus protocol
- ESP32 and related Stripboard for wiring up all compoinents
- Ethernet cable
- USB cable (just for first code upload)
- WiFi router with internet access
- 

## Hardware

### Stripboard

## Software
