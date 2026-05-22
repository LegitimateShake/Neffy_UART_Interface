#include "Neffy_Interface.h"

uart_config_t uart_config = {.data_bits  = UART_DATA_8_BITS,
                             .parity     = UART_PARITY_DISABLE,
                             .stop_bits  = UART_STOP_BITS_1,
                             .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
                             .source_clk = UART_SCLK_APB};

NeffyInterface::NeffyInterface() {}

NeffyInterface::~NeffyInterface() {

    if(uart_is_driver_installed(uart_port)) uart_driver_delete(uart_port);
}

int NeffyInterface::initUART(uint8_t RX, uint8_t TX, uint32_t baudrate, uart_port_t port) {

    esp_err_t uart_status;

    //Only install once
    if(!uart_is_driver_installed(port)) {
        
        uart_status = uart_driver_install(port, UART_RX_BUFFER_SIZE, UART_TX_BUFFER_SIZE, 0, NULL, 0);
        
        if(uart_status != ESP_OK) return -1;
    }

    uart_port = port;
    uart_config.baud_rate = baudrate;
    
    int timeout_threshold =  static_cast<int>(std::ceil((UART_RX_TIMEOUT_US * (double)baudrate) / (1000000.0 * UART_BITS_PER_SYMBOL))); 

    //Clamp to range between 1 and 126
    timeout_threshold = std::max(1, std::min(timeout_threshold, 126));

    if(uart_param_config(port, &uart_config)                              == ESP_OK && 
       uart_set_pin(port, TX, RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) == ESP_OK && 
       uart_set_rx_timeout(port, (uint8_t)timeout_threshold)              == ESP_OK ){

        return 0;
    }
    else return -1;
}

int NeffyInterface::readData() {

    if(bytes_in_buffer >= INPUT_BUFFER_SIZE) bytes_in_buffer = 0;

    uint32_t space_in_buffer = INPUT_BUFFER_SIZE - bytes_in_buffer;

    int bytes_read = uart_read_bytes(uart_port, &in_buffer[bytes_in_buffer], space_in_buffer, 0);

    if(bytes_read > 0) {
        
        bytes_in_buffer += bytes_read;
    }

    return bytes_read;
}   

int NeffyInterface::processData() {

    uint16_t bytes_processed = 0;

    //Iterates over all bytes in the buffer
    for(int i = 0 ; i < bytes_in_buffer ; i++) { 

        // 1) CASE: Not a start byte - move to next byte
        if(in_buffer[i] != START_OF_FRAME_IDENTIFIER) {

            bytes_processed = i + 1;
            continue;
        }

        // 2) CASE: Header is incomplete - do not process the START_OF_FRAME_IDENTIFIER byte and wait for the rest of the header
        if(i + MESSAGE_HEADER_SIZE > bytes_in_buffer) {
            
            bytes_processed = i;
            break;
        }

        uint8_t  commandID      = in_buffer[i + 1];
        uint8_t  payload_length = in_buffer[i + 2];
        uint16_t message_length = payload_length + MESSAGE_HEADER_SIZE;
        uint16_t payload_start  = i              + MESSAGE_HEADER_SIZE;

        // 3) CASE: Payload of the package is bigger than the message buffer or the message is longer than the limit - process and ignore packet start and continue
        if(payload_length > PAYLOAD_BUFFER_SIZE || message_length > MAX_MESSAGE_LENGTH) {
         
            bytes_processed = i + 1;
            continue;
        }

        // 4) CASE: Payload is incomplete - do not process the START_OF_FRAME_IDENTIFIER byte and wait for the rest of the message
        if(i + message_length > bytes_in_buffer) {
                        
            bytes_processed = i;
            break;
        }

        // 5) CASE: Too many unprocessed messages are already in the message queue - do do not process the START_OF_FRAME_IDENTIFIER byte and stop processing the buffer
        if(messages_in_buffer >= MESSAGE_BUFFER_SIZE) {
                        
            bytes_processed = i;
            break;
        }

        //-------------------------------------------------------------------------------------------------------------------//
        //   If we arrive here, the message looks valid and we have got enough space to store it - so store the message      //
        //-------------------------------------------------------------------------------------------------------------------//
        messages[messages_in_buffer].command         = commandID;
        messages[messages_in_buffer].bytes_in_buffer = payload_length;

        if(payload_length > 0) std::memcpy(messages[messages_in_buffer].buffer, &in_buffer[payload_start], payload_length);

        messages_in_buffer++;
        bytes_processed = i + message_length;
        i += message_length - 1;
    }

    //-------------------------------------------------------------------------------------------------------------------//
    //   If we arrive here, we processed the entire buffer - Now we need to remove all processed bytes to make space     //
    //-------------------------------------------------------------------------------------------------------------------//
    bytes_in_buffer -= bytes_processed;
    
    if(bytes_in_buffer > 0) std::memmove(in_buffer, &in_buffer[bytes_processed], bytes_in_buffer);

    return messages_in_buffer;
}

void NeffyInterface::executeCommand() {

    //Iterate over all messages in the message buffer
    for(int i = 0 ; i < messages_in_buffer ; i++) {

        uint8_t msg_ID = messages[i].command;
        Message& msg   = messages[i];
        void (*method)(Message&) = nullptr;

        //Find the corresponding method
        for(int j = 0 ; j < COMMAND_AMOUNT ; j ++) {

            if(msg_ID == commandTable[j].commandID) {

                method = commandTable[j].method;
                break;
            }
        }
        if(method != nullptr) method(msg);
    }

    messages_in_buffer = 0;

    return;
}

int NeffyInterface::update() {

    int messages = 0;

    //Read new data from the input buffer
    if(readData()) {

        //Process that data into messages
        messages = processData();

        //Execute the methods that correspond to each message
        executeCommand();
    }
    return messages;
}


int NeffyInterface::writeMessage(Message& msg) {

    uint8_t payload_size = msg.bytes_in_buffer;
    size_t bytes_to_send = MESSAGE_HEADER_SIZE + payload_size;

    if (msg.command == INVALID_MESSAGE_ID || payload_size > PAYLOAD_BUFFER_SIZE) return -1;

    out_buffer[0] = START_OF_FRAME_IDENTIFIER;
    out_buffer[1] = msg.command;
    out_buffer[2] = payload_size;

    if(payload_size > 0) std::memcpy(&out_buffer[3], msg.buffer, payload_size);

    int bytes_sent = uart_write_bytes(uart_port, out_buffer, bytes_to_send);

    return bytes_sent;
}

int NeffyInterface::addMethod(uint8_t id, void (*method)(Message&)) {

    //Find the corresponding method
    for(int i = 0 ; i < COMMAND_AMOUNT ; i ++) {

        if(id == commandTable[i].commandID) {

            commandTable[i].method = method;
            return 0;
        }
    }
    return -1;
}
























