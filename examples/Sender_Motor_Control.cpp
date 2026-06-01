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


void motorResponse(Message &msg) {

    if(msg.bytes_in_buffer < NeffyResponse::MOVE_BODY_IN_TIME.payloadLength) return;

    uint16_t position = (((uint16_t)msg.buffer[0] << 8) | msg.buffer[1]) * NeffyCommands::MOVE_BODY_IN_TIME.scaleFactor;

    Serial.print("Position: ");
    Serial.println(position);

    delay(1000);

    if(position == 85)
        interface.writeMessage(msg_backward);
    if(position == 0)
        interface.writeMessage(msg_forward);

    return;
}

void setup() {

    //Initialize the UART Communication
    interface.initUART(rx_pin, tx_pin, baudrate, UART_NUM_1);
    interface.addMethod(NeffyCommands::MOVE_BODY_IN_TIME.id, &motorResponse);

    uint16_t forwad_target   = 850; //  85mm * 10
    uint16_t backward_target =   0; //   0mm * 10
    uint16_t duration        =  40; //  4sec * 10

    //Fill the Message structs that should be sent over UART
    msg_forward.command         = NeffyCommands::MOVE_BODY_IN_TIME.id;
    msg_forward.bytes_in_buffer = 4;
    msg_forward.buffer[0]       = (forwad_target >> 8) & 0xFF; //Position MSB
    msg_forward.buffer[1]       =  forwad_target       & 0xFF; //Position LSB
    msg_forward.buffer[2]       = (duration >> 8)      & 0xFF; //Duration MSB
    msg_forward.buffer[3]       =  duration            & 0xFF; //Duration LSB
    msg_forward.buffer[4] = 0;
    msg_forward.buffer[5] = 0;
    msg_forward.buffer[6] = 0;
    msg_forward.buffer[7] = 0;

    msg_backward.command         = NeffyCommands::MOVE_BODY_IN_TIME.id;
    msg_backward.bytes_in_buffer = 4;
    msg_backward.buffer[0]       = (backward_target >> 8) & 0xFF; //Position MSB
    msg_backward.buffer[1]       =  backward_target       & 0xFF; //Position LSB
    msg_backward.buffer[2]       = (duration >> 8)        & 0xFF; //Duration MSB
    msg_backward.buffer[3]       =  duration              & 0xFF; //Duration LSB
    msg_backward.buffer[4] = 0;
    msg_backward.buffer[5] = 0;
    msg_backward.buffer[6] = 0;
    msg_backward.buffer[7] = 0;

    delay(5000);

    interface.writeMessage(msg_forward);
}

void loop() {

    interface.update();
}