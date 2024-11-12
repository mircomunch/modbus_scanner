#include <solarModbus.hpp>
#include <setup.hpp>

SolarModbus TRACER(PIN_RE_DE, PIN_DI, PIN_RO, nIter, MODBUS_ID, SERIAL_STREAM_SPEED, SERIAL_STREAM, SERIAL_STREAM);
String message;
String header = "address,function_name,function_code,content\r\n";

void setup() {
  Serial.begin(115200);
}

void loop() {
  message = header;
  uint16_t content = 0x0000;
  for(uint16_t addr = 0x0000; addr <= 0xFFFF; addr++) {
    char addr_hex[5];
    sprintf(addr_hex, "0x%04x", addr);

    uint8_t result = TRACER.readCoils(addr, &content);
    if(result==0){
      String append = String(addr_hex) + ",Read_Coil,fn_1," + String(content) + "\r\n";
      message += append;
    }
    delay(10);

    result = TRACER.readDiscreteInputs(addr, &content);
    if(result==0){
      String append = String(addr_hex) + ",Read_Discrete_Input,fn_2," + String(content) + "\r\n";
      message += append;
    }
    delay(10);

    result = TRACER.readHoldingRegisters(addr, &content, 1);
    if(result==0){
      String append = String(addr_hex) + ",Read_Holding_Registers,fn_3," + String(content) + "\r\n";
      message += append;
    }
    delay(10);

    result = TRACER.readInputRegisters(addr, &content, 1, false);
    if(result==0){
      String append = String(addr_hex) + ",Read_Input_Registers,fn_4," + String(content) + "\r\n";
      message += append;
    }
    delay(10);

    if(message.length() > 100) {
      Serial.println(message);
      Serial.println("SEND!");
      message = header;
    }
  }
}