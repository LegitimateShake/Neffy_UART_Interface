#include "Arduino.h"
#include "driver/gpio.h"
#include "Neffy_Interface.h"
#include "MotorController.h"
#include <atomic>
#include <esp_task_wdt.h>         // Multiprocessing
#include <freertos/FreeRTOS.h>    // Code Blocks for Multicore applications
#include <freertos/semphr.h>      

/**
 * @brief This program runs on the second core of the ESP32. It takes care of motor actuation and limit checks
 * @brief Variables that are shared with the main program need to be protected through eigther mutual exclusion or atomic flags
 * @brief This program owns all motor objects. Parameters are shared over helper structs
 */
void motorCoreControlLoop(void* args);

struct ExchangeMotorData{

    float position = 0;
    float duration = 0;
    std::atomic<bool> data_copied {true};
    std::atomic<bool> homing_complete {false};
};

ExchangeMotorData sharedMotorCommand_body;
ExchangeMotorData sharedMotorCommand_head;
NeffyInterface    interface;

uint8_t    rx_pin   = 44;
uint8_t    tx_pin   = 43;
uint32_t   baudrate = 115200;

void moveBody(Message& msg) {
    
    if(!sharedMotorCommand_body.homing_complete.load() || msg.bytes_in_buffer < NeffyCommands::MOVE_BODY_IN_TIME.payloadLength) return;

    int timeout = 0;
    
    //Block until the last instruction was copied by the second core. Most likely overkill
    while(sharedMotorCommand_body.data_copied.load() == false) {

        vTaskDelay(1);
        timeout++;

        if(timeout > 10) return;
    }

    sharedMotorCommand_body.position = (((uint16_t)msg.buffer[0] << 8) | msg.buffer[1]) * NeffyCommands::MOVE_BODY_IN_TIME.scaleFactor;
    sharedMotorCommand_body.duration = (((uint16_t)msg.buffer[2] << 8) | msg.buffer[3]) * NeffyCommands::MOVE_BODY_IN_TIME.scaleFactor;
    sharedMotorCommand_body.data_copied.store(false);
    
    return;
}

void moveHead(Message& msg) {

    if(!sharedMotorCommand_head.homing_complete.load() || msg.bytes_in_buffer < NeffyCommands::MOVE_HEAD_IN_TIME.payloadLength) return;

    int timeout = 0;

    //Block until the last instruction was copied by the second core. Most likely overkill
    while(sharedMotorCommand_head.data_copied.load() == false) {

        vTaskDelay(1);
        timeout++;

        if(timeout > 10) return;
    }

    sharedMotorCommand_head.position = (((uint16_t)msg.buffer[0] << 8) | msg.buffer[1]) * NeffyCommands::MOVE_HEAD_IN_TIME.scaleFactor;
    sharedMotorCommand_head.duration = (((uint16_t)msg.buffer[2] << 8) | msg.buffer[3]) * NeffyCommands::MOVE_HEAD_IN_TIME.scaleFactor;
    sharedMotorCommand_head.data_copied.store(false);
    
    return;
}

void moveBodyAndHead(Message& msg) {

    if(!sharedMotorCommand_body.homing_complete.load() || !sharedMotorCommand_head.homing_complete.load() || 
        msg.bytes_in_buffer < NeffyCommands::MOVE_BODY_HEAD_IN_TIME.payloadLength) return;

    int timeout = 0;

    while(sharedMotorCommand_body.data_copied.load() == false || sharedMotorCommand_head.data_copied.load() == false) {

        vTaskDelay(1);
        timeout++;

        if(timeout > 10) return;
    }

    sharedMotorCommand_body.position = (((uint16_t)msg.buffer[0] << 8) | msg.buffer[1]) * NeffyCommands::MOVE_BODY_IN_TIME.scaleFactor;
    sharedMotorCommand_body.duration = (((uint16_t)msg.buffer[2] << 8) | msg.buffer[3]) * NeffyCommands::MOVE_BODY_IN_TIME.scaleFactor;

    sharedMotorCommand_head.position = (((uint16_t)msg.buffer[4] << 8) | msg.buffer[5]) * NeffyCommands::MOVE_HEAD_IN_TIME.scaleFactor;
    sharedMotorCommand_head.duration = (((uint16_t)msg.buffer[6] << 8) | msg.buffer[7]) * NeffyCommands::MOVE_HEAD_IN_TIME.scaleFactor;

    sharedMotorCommand_body.data_copied.store(false);
    sharedMotorCommand_head.data_copied.store(false);

    return;
}


void setup() {

    //Initialize the UART Communication
    interface.initUART(rx_pin, tx_pin, baudrate, UART_NUM_0);
    interface.addMethod(NeffyCommands::MOVE_BODY_IN_TIME.id, &moveBody);        
    interface.addMethod(NeffyCommands::MOVE_HEAD_IN_TIME.id, &moveHead);
    
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

void motorCoreControlLoop(void* args) {

    esp_task_wdt_init(30, false);
    
    LinearActuator  motorBody(4, 8, STEP_BODY, DIR_BODY, EN_BODY, -1, -1, BTN_BODY);
    MotorController controller;

    controller.addMotor(&motorBody);
    controller.begin(true);
    controller.enableStepping();

    motorBody.configHoming(BACKWARD, 4, 5, 60);

    while(!motorBody.home()) {}

    motorBody.tare();
    sharedMotorCommand_body.homing_complete.store(true);


    while (true) {

        if(sharedMotorCommand_body.data_copied.load() == false) {

            motorBody.moveToPositionInTime(sharedMotorCommand_body.position,  sharedMotorCommand_body.duration);
            sharedMotorCommand_body.data_copied.store(true);
        }

        if(sharedMotorCommand_head.data_copied.load() == false) {

            //Add the function for the head here. Not added yet for testing purposes
            sharedMotorCommand_head.data_copied.store(true);
        }

        motorBody.update();   
    }
}