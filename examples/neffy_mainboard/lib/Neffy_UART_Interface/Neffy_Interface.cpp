#include "Neffy_Interface.h"

uart_config_t uart_config = {.data_bits  = UART_DATA_8_BITS,
                             .parity     = UART_PARITY_DISABLE,
                             .stop_bits  = UART_STOP_BITS_1,
                             .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
                             .source_clk = UART_SCLK_APB};

NeffyInterface::NeffyInterface() : _mutexWriteMessage(xSemaphoreCreateMutex()){}

NeffyInterface::~NeffyInterface() {

    if(uart_is_driver_installed(_uart_port)) uart_driver_delete(_uart_port);
}

int NeffyInterface::initUART(uint8_t RX, uint8_t TX, uint32_t baudrate, uart_port_t port) {

    esp_err_t uart_status;

    //Only install once
    if(!uart_is_driver_installed(port)) {
        
        uart_status = uart_driver_install(port, UART_RX_BUFFER_SIZE, UART_TX_BUFFER_SIZE, 0, NULL, 0);
        
        if(uart_status != ESP_OK) return -1;
    }

    _uart_port = port;
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

    if(_bytes_in_buffer >= INPUT_BUFFER_SIZE) _bytes_in_buffer = 0;

    uint32_t space_in_buffer = INPUT_BUFFER_SIZE - _bytes_in_buffer;

    int bytes_read = uart_read_bytes(_uart_port, &_in_buffer[_bytes_in_buffer], space_in_buffer, 0);

    if(bytes_read > 0) {
        
        _bytes_in_buffer += bytes_read;
    }

    return bytes_read;
}   

ParsingResult NeffyInterface::checkForValidMessage(int index) {

    // 1) CASE: Not a start byte - move to next byte
    if(_in_buffer[index] != START_OF_FRAME_IDENTIFIER) 
        return {.state = ParsingState::no_valid_message_found, .bytesProcessed = 1};

    // 2) CASE: Header is incomplete - do not process the START_OF_FRAME_IDENTIFIER byte and wait for the rest of the header
    if(index + MESSAGE_HEADER_SIZE > _bytes_in_buffer) 
        return {.state = ParsingState::wait_for_remaining_data, .bytesProcessed = 0};

    uint8_t  payload_length = _in_buffer[index + 2];
    uint16_t message_length = payload_length + MESSAGE_HEADER_SIZE;

    // 3) CASE: Payload of the package is bigger than the message buffer or the message is longer than the limit - process and ignore packet start and continue
    if(payload_length > PAYLOAD_BUFFER_SIZE || message_length > MAX_MESSAGE_LENGTH) 
        return {.state = ParsingState::no_valid_message_found, .bytesProcessed = 1};

    // 4) CASE: Payload is incomplete - do not process the START_OF_FRAME_IDENTIFIER byte and wait for the rest of the message
    if(index + message_length > _bytes_in_buffer) 
        return {.state = ParsingState::wait_for_remaining_data, .bytesProcessed = 0};
    
    // 5) CASE: Too many unprocessed messages are already in the message queue - do do not process the START_OF_FRAME_IDENTIFIER byte and stop processing the buffer
    if(_messages_in_buffer >= MESSAGE_BUFFER_SIZE) 
        return {.state = ParsingState::message_buffer_full, .bytesProcessed = 0};

    //Valid Message in the Buffer
    return {.state = ParsingState::valid_message_found, .bytesProcessed = static_cast<uint8_t>(message_length)};
}

void NeffyInterface::storeData(const uint8_t* messageStart) {

    Message& msg = _messages[_messages_in_buffer]; 

    uint8_t commandID            =  messageStart[1];
    uint8_t payloadLength        =  messageStart[2];
    const uint8_t* payloadStart  = &messageStart[3];

    msg.command         = commandID;
    msg.bytes_in_buffer = payloadLength;

    if(payloadLength > 0) 
        std::memcpy(msg.buffer, payloadStart, payloadLength);

    _messages_in_buffer++;
}

void NeffyInterface::compactInputBuffer(int dataIndex) {

    _bytes_in_buffer -= dataIndex;

    if(_bytes_in_buffer > 0 && dataIndex > 0) 
        std::memmove(_in_buffer, &_in_buffer[dataIndex], _bytes_in_buffer);
}

int NeffyInterface::processData() {

    int index = 0;
    ParsingResult messageState;

    while(index < _bytes_in_buffer) {

        messageState = checkForValidMessage(index);

        switch (messageState.state)
        {
        case ParsingState::valid_message_found:
            storeData(&_in_buffer[index]);
            index += messageState.bytesProcessed;
            break;
        
        case ParsingState::no_valid_message_found:    
            index += messageState.bytesProcessed;
            break;
        
        case ParsingState::message_buffer_full:
        case ParsingState::wait_for_remaining_data:
            compactInputBuffer(index);
            return _messages_in_buffer;
        }
    }

    compactInputBuffer(index);
    return _messages_in_buffer;
}

void NeffyInterface::executeCommand() {

    //Iterate over all messages in the message buffer
    for(int i = 0 ; i < _messages_in_buffer ; i++) {

        //The messageID is the index of the corresponding method
        uint8_t msg_ID = _messages[i].command;
        void (*method)(Message&) = nullptr;

        //Prevents accessing data outside the array if a corrupted commandID is received
        if(msg_ID <= MAX_COMMAND_ID)
            method = _dispatchTable[msg_ID];
        
        if(method != nullptr) method(_messages[i]);
    }
    _messages_in_buffer = 0;

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

int NeffyInterface::writeMessage(const Message& msg) {

    uint8_t payload_size = msg.bytes_in_buffer;
    size_t bytes_to_send = MESSAGE_HEADER_SIZE + payload_size;

    if (msg.command == INVALID_MESSAGE_ID || payload_size > PAYLOAD_BUFFER_SIZE) return -1;

    uint8_t out_buffer[MAX_MESSAGE_LENGTH];

    out_buffer[0] = START_OF_FRAME_IDENTIFIER;
    out_buffer[1] = msg.command;
    out_buffer[2] = payload_size;

    if(payload_size > 0) std::memcpy(&out_buffer[3], msg.buffer, payload_size);

    xSemaphoreTake(_mutexWriteMessage, portMAX_DELAY);
    int bytes_sent = uart_write_bytes(_uart_port, out_buffer, bytes_to_send);
    xSemaphoreGive(_mutexWriteMessage);

    return bytes_sent;
}

int NeffyInterface::addMethod(uint8_t id, void (*method)(Message&)) {

    if(id <= MAX_COMMAND_ID) {

        _dispatchTable[id] = method;
        return 0;
    }
    return -1;
}