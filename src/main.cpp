#include <solarModbus.hpp>
#include <setup.hpp>

SolarModbus TRACER(PIN_RE_DE, PIN_DI, PIN_RO, nIter, MODBUS_ID, SERIAL_STREAM_SPEED, SERIAL_STREAM);
bool run = true;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if(run){
    run = false;

    String header = "address,function_name,function_code,error,content";
    Serial.println(header);
    uint16_t content = 0x0000;
    for(uint16_t addr = 0x0000; addr <= 0xFFFF; addr++) {
    // uint16_t addr = 0x9003;
      char addr_hex[5];
      sprintf(addr_hex, "0x%04x", addr);
      String fn1 = String(addr_hex) + "," + "Read Coil"               + "," + "fn_1" + ",";
      String fn2 = String(addr_hex) + "," + "Read Discrete Input"     + "," + "fn_2" + ",";
      String fn3 = String(addr_hex) + "," + "Read Holding Registers"  + "," + "fn_3" + ",";
      String fn4 = String(addr_hex) + "," + "Read Input Registers"    + "," + "fn_4" + ",";

      uint8_t result = TRACER.readCoils(addr, &content);
      if(result!=0){
        fn1 += TRACER.exceptionDescription(result) + ",";
      }
      else {
        fn1 += "Success,"+ String(content);
      }
      Serial.println(fn1);
      delay(10);
    
      result = TRACER.readDiscreteInputs(addr, &content);
      if(result!=0){
        fn2 += TRACER.exceptionDescription(result) + ",";
      }
      else {
        fn2 += "Success,"+ String(content);
      }
      Serial.println(fn2);
      delay(10);
    
      result = TRACER.readHoldingRegisters(addr, &content, 1);
      if(result!=0){
        fn3 += TRACER.exceptionDescription(result) + ",";
      }
      else {
        fn3 += "Success,"+ String(content);
      }
      Serial.println(fn3);
      delay(10);
    
      result = TRACER.readInputRegisters(addr, &content, 1, false);
      if(result!=0){
        fn4 += TRACER.exceptionDescription(result) + ",";
      }
      else {
        fn4 += "Success,"+ String(content);
      }
      Serial.println(fn4);
      delay(10);
    }
  }
}