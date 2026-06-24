#include "PersonSensor.h"
#include "config.h"
#include <Wire.h>

static constexpr float    THRESHOLD_PASSING  = 1.5f;
static constexpr float    THRESHOLD_PRESENT  = 3.5f;
static constexpr uint32_t PRESENT_HOLD_MS    = 1000;
static constexpr uint8_t  TEMP_BYTE_COUNT    = 5;
static constexpr float    BASELINE_ADAPT_EMA = 0.05f;

float PersonSensor::readTemp() {
    Wire.beginTransmission(D6TConfig::I2C_ADDR);
    Wire.write(D6TConfig::I2C_CMD);
    if (Wire.endTransmission(true) != 0) return -1.0f;

    Wire.requestFrom((uint8_t)D6TConfig::I2C_ADDR, (uint8_t)TEMP_BYTE_COUNT);
    uint8_t buf[TEMP_BYTE_COUNT];
    for (int i = 0; i < TEMP_BYTE_COUNT; i++) buf[i] = Wire.read();

    return ((buf[3] << 8) | buf[2]) * 0.1f;
}

void PersonSensor::init() {
    Wire.begin(D6TConfig::SDA_PIN, D6TConfig::SCL_PIN);
    Wire.setClock(D6TConfig::I2C_CLOCK);
    _baseline       = readTemp();
    _presentPending = false;
    _presentSinceMs = 0;
}

PersonState PersonSensor::update() {
    _lastTemp    = readTemp();
    float delta  = _lastTemp - _baseline;

    PersonState rawState;
    if      (delta >= THRESHOLD_PRESENT) rawState = PersonState::PRESENT;
    else if (delta >= THRESHOLD_PASSING) rawState = PersonState::PASSING;
    else                                 rawState = PersonState::NONE;

    PersonState state;
    if (rawState == PersonState::PRESENT) {
        if (!_presentPending) {
            _presentPending = true;
            _presentSinceMs = millis();
        }
        state = (millis() - _presentSinceMs >= PRESENT_HOLD_MS)
                    ? PersonState::PRESENT
                    : PersonState::PASSING;
    } else {
        _presentPending = false;
        state = rawState;
    }

    if (state == PersonState::NONE) {
        _baseline = _baseline * (1.0f - BASELINE_ADAPT_EMA) + _lastTemp * BASELINE_ADAPT_EMA;
    }

    return state;
}
