#include "Arduino.h"
#include "driver/gpio.h"
#include "Neffy_Interface.h"
#include "MotorController.h"
#include <freertos/FreeRTOS.h>    
#include "MotorControlLoop.h"

NeffyInterface    interface;

uint8_t    rx_pin   = 44;
uint8_t    tx_pin   = 43;
uint32_t   baudrate = 115200;

void setup() {

    //Initialize the UART Communication
    interface.initUART(rx_pin, tx_pin, baudrate, UART_NUM_0);
    
    //Initialize the motor control loop on the second core
    void* taskParameter           = nullptr;
    uint32_t stackSize            = 4096;
    BaseType_t core               = 0;
    UBaseType_t motorLoopPriority = 2;
    TaskHandle_t* taskHandle      = nullptr;
    
    xTaskCreatePinnedToCore(&motorCoreControlLoop, "MotorLoop", stackSize, taskParameter, motorLoopPriority, taskHandle, core);   
}

void loop() {

    interface.update();
}
