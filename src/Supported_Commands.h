#include "Neffy_Interface_Types.h"
#include "stdint.h"
#pragma once

static constexpr uint8_t  COMMAND_AMOUNT =   10; // Amount of currently supported commands. Increment, if you add more
static constexpr uint8_t  MAX_COMMAND_ID = 0x09; // Biggest commandID. Is used for length of command lookup-table. Increment if you add more 

static constexpr uint8_t  START_OF_FRAME_IDENTIFIER = 0xFA; // Must be the first byte of each message
static constexpr uint8_t  INVALID_MESSAGE_ID        = 0xFF; // This is used to check if a commandID was set before sending a message out

struct MessageMetadata {

    uint8_t id;             //-ID of the Command
    uint8_t payloadLength;  //-Expected amount of payload bytes
    float   scaleFactor;    //-Value for scaling incloming integer values back to floats or outgoing floats to integer values
};

struct NeffyCommands {

    static constexpr MessageMetadata LED                     = {.id = 0x00, .payloadLength = 1, .scaleFactor =   1};
    static constexpr MessageMetadata MUTE                    = {.id = 0x01, .payloadLength = 1, .scaleFactor =   1};
    static constexpr MessageMetadata PRESSURE_USER_DETECTION = {.id = 0x02, .payloadLength = 1, .scaleFactor =   1};
    static constexpr MessageMetadata GET_PERSON_READING      = {.id = 0x03, .payloadLength = 0, .scaleFactor =   1};
    static constexpr MessageMetadata MOVE_BODY_IN_TIME       = {.id = 0x04, .payloadLength = 4, .scaleFactor = 0.1};
    static constexpr MessageMetadata MOVE_HEAD_IN_TIME       = {.id = 0x05, .payloadLength = 4, .scaleFactor = 0.1};
    static constexpr MessageMetadata MOVE_BODY_HEAD_IN_TIME  = {.id = 0x06, .payloadLength = 8, .scaleFactor = 0.1};
    static constexpr MessageMetadata DISABLE_MOTORS          = {.id = 0x07, .payloadLength = 1, .scaleFactor =   1};
    static constexpr MessageMetadata PRESSURE_DOUBLE_CLICK   = {.id = 0x08, .payloadLength = 0, .scaleFactor =   1};
    static constexpr MessageMetadata BREATHING_RATE          = {.id = 0x09, .payloadLength = 1, .scaleFactor =   1};
};

struct NeffyResponse {

    static constexpr MessageMetadata MOVE_BODY_IN_TIME       = {.id = 0x04, .payloadLength = 2, .scaleFactor =  10};
    static constexpr MessageMetadata MOVE_HEAD_IN_TIME       = {.id = 0x05, .payloadLength = 2, .scaleFactor =  10};
};
