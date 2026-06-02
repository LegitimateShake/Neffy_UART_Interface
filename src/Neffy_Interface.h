#pragma once
#include <cstring>
#include <algorithm>
#include <cmath>
#include <stdint.h>
#include "Neffy_Interface_Types.h"
#include "Supported_Commands.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"

class NeffyInterface {
    
    private:

        uart_port_t _uart_port;

        uint8_t _in_buffer[INPUT_BUFFER_SIZE] = {0};
        uint8_t _bytes_in_buffer             =   0 ;

        Message _messages[MESSAGE_BUFFER_SIZE];
        uint8_t _messages_in_buffer = 0;       

        //Lookup Table of methods for each received command
        void (*_dispatchTable[MAX_COMMAND_ID + 1])(Message&) = {nullptr};

        //Mutex for protecting write operations from different cores
        SemaphoreHandle_t _mutexWriteMessage = NULL;

        /**
         * @brief Checks if UART data is available and copies it into the buffer
         * @return amount of bytes that were read
         */
        int readData();

        /**
         * @brief Checks the input buffer for a valid message
         * @param index the index of the input buffer where the search should be started
         * @return `ParsingResult` struct, containing the result of the search and the total number of bytes that were processed.
         *          These include the full header + payload length, if sucessful
         */
        ParsingResult checkForValidMessage(int index);

        /**
         * @brief copies a message from the input buffer into a message struct. Should only be called after a valid message was found
         * @param data Pointer to the `Start of Frame Identifier` of a valid message in the input buffer
         * @return `None`
         */
        void storeData(const uint8_t* data);

        /**
         * @brief Removes all processed bytes from the input buffer and moves all unprocessed bytes to the front
         * @param dataIndex The index of the first byte that was not processed
         * @return `None`
         */
        void compactInputBuffer(int dataIndex);

        /**
         * @brief Searches the input buffer for valid messages and copies them into into message structs
         * @return The amount of messages that are currently in the message buffer
         */
        int processData();

        /**
         * @brief Converts the input parameters to the needed data and calls the corresponding method
         */
        void executeCommand();
        
    public:

        NeffyInterface();

        ~NeffyInterface();

        /**
         * @brief initialize the hardware for UART communication. Must be called before data can be send/received
         * @param RX The rx-pin
         * @param TX The tx-pin
         * @param baudrate UART baudrate in bits per second
         * @param port The UART hardware that should be used. Can be `UART_NUM_0`, `UART_NUM_1`, `UART_NUM_2`
         * @return `0` on sucess, `-1` if the UART setup failed
         */
        int initUART(uint8_t RX, uint8_t TX, uint32_t baudrate, uart_port_t port = UART_NUM_0);

        /**
         * @brief Add a method that should be executed once a message with the corresponding ID arrives
         * @brief Keep the method short. No blocking behavior! Maybe just set global flags and then do the actual computation in the main loop
         * @param id The message ID
         * @param method A pointer to the method. The method must return void an take a reference to a Message struct as an argument
         * @return `0` on sucess, `-1` if the CommandID was not valid
         */
        int addMethod(uint8_t id, void (*method)(Message&));

        /**
         * @brief Sends the contents of the message over UART
         * @param msg Message struct, that must include `id` and `msg_bytes_in_buffer`
         * @return `-1` if the message was not initialized correctly, else amount of bytes that were transfered to the buffer
         */
        int writeMessage(const Message& msg);

        /**
         * @brief Reads and processes new data over UART and calls the corresponding methods. Should be called in a loop
         * @return amount of messages that were processed
         */
        int update();
};