#include <ModbusMaster.h>

class SolarModbus {
public:
    ModbusMaster node;
    SolarModbus(Stream &serial = Serial2);
    uint8_t writeSingleCoil(uint16_t data, uint16_t address);
    uint8_t readCoils(uint16_t address, void *variable);
    uint8_t writeMultipleRegisters(uint16_t *data, uint16_t address, uint8_t nReg);
    uint8_t readHoldingRegisters(uint16_t address, void *variable, uint8_t nReg);
    uint8_t readInputRegisters(uint16_t address, void *variable, uint8_t nReg, bool oneHundredTimes);
    uint8_t readDiscreteInputs(uint16_t address, void *variable);
    uint8_t readSingleInputRegisters(uint16_t address, void *variable, bool oneHundredTimes);

    String exceptionDescription(uint8_t exception);
    // String exceptionHandler(uint8_t result, String mode, String address);
    void exceptionHandler(uint8_t result, String mode, String address);

private:
    static void setPin();
    static void resetPin();
};