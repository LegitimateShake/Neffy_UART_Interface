#include "Arduino.h"
#include "Neffy_Interface.h"
#include "config.h"
#include "pressure_handler.h"
#include "breathing_handler.h"

NeffyInterface interface;

void setup() {
    Serial.begin(SenderConfig::BAUDRATE);
    interface.initUART(SenderConfig::RX_PIN, SenderConfig::TX_PIN, SenderConfig::BAUDRATE, UART_NUM_1);
    pressureInit(interface);
    breathingInit(interface);
}

void loop() {
    interface.update();
    pressureUpdate(millis());
}
