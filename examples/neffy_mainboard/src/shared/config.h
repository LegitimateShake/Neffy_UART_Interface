#pragma once
#include <stdint.h>
#include "driver/gpio.h"

/** Hardware constants for the receiver board (ESP32-S3). */
namespace ReceiverConfig {
    constexpr uint8_t    RX_PIN             = 44;
    constexpr uint8_t    TX_PIN             = 43;
    constexpr uint32_t   BAUDRATE           = 115200;
    constexpr gpio_num_t LED_PIN            = GPIO_NUM_34;
    constexpr uint8_t    PRESSURE_PIN_L     = 14;
    constexpr uint8_t    PRESSURE_PIN_R     = 12;
    constexpr uint32_t   BREATHING_DELAY_MS = 3000;
}

/** Hardware constants for the sender board (ESP32). */
namespace SenderConfig {
    constexpr uint8_t  RX_PIN   = 16;
    constexpr uint8_t  TX_PIN   = 17;
    constexpr uint32_t BAUDRATE = 115200;
    constexpr uint8_t  LED_PIN  = 2;
}

/** D6T-44L thermal sensor I2C constants. */
namespace D6TConfig {
    constexpr uint8_t  I2C_ADDR  = 0x0A;
    constexpr uint8_t  I2C_CMD   = 0x4C;
    constexpr uint8_t  SDA_PIN   = 37;
    constexpr uint8_t  SCL_PIN   = 38;
    constexpr uint32_t I2C_CLOCK = 10000;
}
