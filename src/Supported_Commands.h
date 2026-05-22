#include "Neffy_Interface_Types.h"
#include "stdint.h"
#pragma once

static constexpr uint8_t  COMMAND_AMOUNT = 10; // Amount of currently supported commands. Increment, if you add more

struct CommandMetadata {

    uint8_t id;             //-ID of the Command
    uint8_t payloadLength;  //-Expected amount of payload bytes
    float   scaleFactor;    //-Value for scaling incloming integer values back to floats
};

struct NeffyCommands {

    static constexpr CommandMetadata LED                    = {.id = 0x00, .payloadLength = 1, .scaleFactor =   1};
    static constexpr CommandMetadata MUTE                   = {.id = 0x01, .payloadLength = 1, .scaleFactor =   1};
    static constexpr CommandMetadata PRESSURE_USER_DETECTION = {.id = 0x02, .payloadLength = 1, .scaleFactor =   1};
    static constexpr CommandMetadata GET_PERSON_READING     = {.id = 0x03, .payloadLength = 0, .scaleFactor =   1};
    static constexpr CommandMetadata MOVE_BODY_IN_TIME      = {.id = 0x04, .payloadLength = 4, .scaleFactor = 0.1};
    static constexpr CommandMetadata MOVE_HEAD_IN_TIME      = {.id = 0x05, .payloadLength = 4, .scaleFactor = 0.1};
    static constexpr CommandMetadata MOVE_BODY_HEAD_IN_TIME = {.id = 0x06, .payloadLength = 8, .scaleFactor = 0.1};
    static constexpr CommandMetadata DISABLE_MOTORS         = {.id = 0x07, .payloadLength = 1, .scaleFactor =   1};
    static constexpr CommandMetadata PRESSURE_DOUBLE_CLICK  = {.id = 0x08, .payloadLength = 0, .scaleFactor =   1};
    static constexpr CommandMetadata BREATHING_RATE         = {.id = 0x09, .payloadLength = 1, .scaleFactor =   1};
};

struct CommandMethods {

    uint8_t commandID;
    void (*method)(Message&);
};

extern CommandMethods commandTable[COMMAND_AMOUNT];