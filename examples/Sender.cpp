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

//Global message struct that can be filled/used by every method
Message msg;


void setup() {

    //Initialize the UART Communication
    interface.initUART(rx_pin, tx_pin, baudrate, UART_NUM_1);

    //Fill the Message structs that should be sent over UART
    msg.command         = NeffyCommands::MUTE.id;
    msg.bytes_in_buffer = 1;
    msg.buffer[0]       = 0x00;

}

void loop() {

    /**
     * This program sends one message every second.
     * The payload alternates between 0x00 and 0x01, eg. sound on and muted
     */
    now = millis();
    dif = now - prev;

    if(dif > 1000) {

        msg.buffer[0] = (msg.buffer[0] == 0x00) ? 0x01 : 0x00;

        interface.writeMessage(msg);

        prev = now;
    }
}