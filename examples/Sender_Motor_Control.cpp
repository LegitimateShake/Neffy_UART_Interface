#include "Arduino.h"
#include "Neffy_Interface.h"

/*

Change these parameters to the board you are using

*/
uint8_t  rx_pin   = 46;
uint8_t  tx_pin   = 33;
uint32_t baudrate = 115200;

unsigned long prev = 0;
unsigned long now  = 0;
unsigned long dif  = 0;

NeffyInterface interface;

bool swap = true;

//Global message struct that can be filled/used by every method
Message msg_forward;
Message msg_backward;


void setup() {

    //Initialize the UART Communication
    interface.initUART(rx_pin, tx_pin, baudrate, UART_NUM_1);

    uint16_t forwad_target   = 500; // 50mm * 10
    uint16_t backward_target =   0; //  0mm * 10
    uint16_t duration        =  50; // 5sec * 10

    //Fill the Message structs that should be sent over UART
    msg_forward.command         = NeffyCommands::MOVE_BODY_IN_TIME.id;
    msg_forward.bytes_in_buffer = 4;
    msg_forward.buffer[0]       = (forwad_target >> 8) & 0xFF; //Position MSB
    msg_forward.buffer[1]       =  forwad_target       & 0xFF; //Position LSB
    msg_forward.buffer[2]       = (duration >> 8)      & 0xFF; //Duration MSB
    msg_forward.buffer[3]       =  duration            & 0xFF; //Duration LSB

    msg_backward.command         = NeffyCommands::MOVE_BODY_IN_TIME.id;
    msg_backward.bytes_in_buffer = 4;
    msg_backward.buffer[0]       = (backward_target >> 8) & 0xFF; //Position MSB
    msg_backward.buffer[1]       =  backward_target       & 0xFF; //Position LSB
    msg_backward.buffer[2]       = (duration >> 8)        & 0xFF; //Duration MSB
    msg_backward.buffer[3]       =  duration              & 0xFF; //Duration LSB

    delay(5000);
}

void loop() {

    now = millis();
    dif = now - prev;

    if(dif > 6000) {

        if(swap) {
            interface.writeMessage(msg_forward);
            swap = false;
        }
        else {
            interface.writeMessage(msg_backward);
            swap = true;
        }
        prev = now;
    }
}