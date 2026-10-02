#include "Arduino.h"
#include "driver/gpio.h"
#include "Neffy_Interface.h"

/*

Change these parameters to the pins of the board you are using

*/
uint8_t    rx_pin   = 44;
uint8_t    tx_pin   = 43;
gpio_num_t led_pin  = GPIO_NUM_34;
uint32_t   baudrate = 115200;

//Communication Interface Class
NeffyInterface interface;

//Global flag that the function uses
bool pin_configured = false;


//This function is executed every time a MUTE command is received and mirrors it on a pin
void blink(Message& msg) {

    //Configure the Pin on the first call
    if(!pin_configured) {

        gpio_reset_pin(led_pin);
        gpio_set_direction(led_pin, GPIO_MODE_OUTPUT);
        pin_configured = true;
    }
     
    if(msg.bytes_in_buffer < 1) return;

    //Just for safety. Only allows 0x00 and 0x01
    uint32_t led_state = msg.buffer[0] & 0x01;

    gpio_set_level(led_pin, led_state);

    return;
}


void setup() {

    //Initialize the UART Communication
    interface.initUART(rx_pin, tx_pin, baudrate, UART_NUM_0);

    //Add methods for commands
    interface.addMethod(NeffyCommands::MUTE.id, &blink);
}

void loop() {

    /**
     * This program waits for messages
     * For the command MUTE a method is registered. All other commands will be ignored
     * The method is executed once a message with the corresponding CommandID is received
     */
    interface.update();
}
