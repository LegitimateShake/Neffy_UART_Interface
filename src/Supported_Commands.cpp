#include "Supported_Commands.h"

CommandMethods commandTable[COMMAND_AMOUNT] = {

    {NeffyCommands::LED.id                    , nullptr},
    {NeffyCommands::MUTE.id                   , nullptr},
    {NeffyCommands::GET_PRESSURE.id           , nullptr},
    {NeffyCommands::GET_PERSON_READING.id     , nullptr},
    {NeffyCommands::MOVE_BODY_IN_TIME.id      , nullptr},
    {NeffyCommands::MOVE_HEAD_IN_TIME.id      , nullptr},
    {NeffyCommands::MOVE_BODY_HEAD_IN_TIME.id , nullptr},
    {NeffyCommands::DISABLE_MOTORS.id         , nullptr}
};
