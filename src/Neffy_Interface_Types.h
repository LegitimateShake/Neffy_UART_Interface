#pragma once
#include <stdint.h>
#include "Supported_Commands.h"

static constexpr uint16_t INPUT_BUFFER_SIZE    =  256; // Amount of bytes for incoming data
static constexpr uint16_t PAYLOAD_BUFFER_SIZE  =   16; // Amount of bytes of payload per message
static constexpr uint8_t  MESSAGE_BUFFER_SIZE  =   16; // Amount of messages that can be stored at the same time
static constexpr uint8_t  MESSAGE_HEADER_SIZE  =    3; // Amount of bytes in the message header [0xFA][CommandID][PayloadLength]

static constexpr uint8_t  MAX_MESSAGE_LENGTH   = MESSAGE_HEADER_SIZE + PAYLOAD_BUFFER_SIZE; 

static constexpr uint16_t UART_RX_BUFFER_SIZE  = 256; // Internal Ring Buffer Size
static constexpr uint16_t UART_TX_BUFFER_SIZE  = 256; // Internal Ring Buffer Size
static constexpr uint16_t UART_RX_TIMEOUT_US   = 200; // Time after which the input buffer is checked, even if no new bits arrived
static constexpr uint16_t UART_BITS_PER_SYMBOL =  10; // The amount of bits per caracter. Is used for computation. Dont touch!

struct Message {
    
    uint8_t command = INVALID_MESSAGE_ID;
    uint8_t buffer[PAYLOAD_BUFFER_SIZE] = {0};
    uint8_t bytes_in_buffer = 0;
};

enum class ParsingState {

    no_valid_message,
    no_storage_space,
    wait_for_data,
    valid_message
};

struct ParsingResult {

    ParsingState state;
    uint8_t bytesProcessed;
};



