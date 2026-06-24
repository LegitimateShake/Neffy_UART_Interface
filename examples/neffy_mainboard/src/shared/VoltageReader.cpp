#include "VoltageReader.h"

static constexpr uint32_t ADC_MAX_VALUE = 4095;
static constexpr uint32_t VREF_MV       = 3300;

VoltageReader::VoltageReader(uint8_t pin, uint8_t samples, float alpha)
    : _pin(pin), _samples(samples), _alpha(alpha), _ema(-1.0f) {}

void VoltageReader::init() {
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);
}

uint32_t VoltageReader::readMv() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < _samples; i++) {
        sum += analogRead(_pin);
    }
    uint32_t avg = sum / _samples;
    return (avg * VREF_MV) / ADC_MAX_VALUE;
}

float VoltageReader::readV() {
    return readMv() / 1000.0f;
}

float VoltageReader::readSmooth() {
    float v = readV();
    if (_ema < 0.0f) _ema = v; // seed with first value to avoid ramp-in noise
    _ema = _alpha * v + (1.0f - _alpha) * _ema;
    return _ema;
}
