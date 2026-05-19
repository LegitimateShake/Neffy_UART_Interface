#include "Supported_Commands.h"

CommandMethods commandTable[COMMAND_AMOUNT] = {

    {CMD_LED                          , nullptr},
    {CMD_SET_MUTE                     , nullptr},
    {CMD_GET_PRESSURE                 , nullptr},
    {CMD_GET_PERSON_READING           , nullptr},
    {CMD_MOVE_MOTOR_BODY_IN_TIME      , nullptr},
    {CMD_MOVE_MOTOR_HEAD_IN_TIME      , nullptr},
    {CMD_MOVE_MOTOR_BODY_HEAD_IN_TIME , nullptr},
    {CMD_DISABLE_MOTORS               , nullptr}
};
