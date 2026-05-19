#include "Neffy_Interface_Types.h"
#include "stdint.h"
#pragma once

static constexpr uint8_t  COMMAND_AMOUNT = 8; // Amount of currently supported commands. Increment, if you add more

enum CommandID : uint8_t {

    CMD_LED                          = 0x00,
    CMD_SET_MUTE                     = 0x01,
    CMD_GET_PRESSURE                 = 0x02,
    CMD_GET_PERSON_READING           = 0x03,
    CMD_MOVE_MOTOR_BODY_IN_TIME      = 0x04,
    CMD_MOVE_MOTOR_HEAD_IN_TIME      = 0x05,
    CMD_MOVE_MOTOR_BODY_HEAD_IN_TIME = 0x06,
    CMD_DISABLE_MOTORS               = 0x07
};

struct CommandMethods {

    uint8_t commandID;
    void (*method)(Message&);
};

extern CommandMethods commandTable[COMMAND_AMOUNT];