#pragma once
#include <Arduino.h>

/** ADC voltage reader with multi-sample averaging and EMA smoothing. */
class VoltageReader {
public:
    /**
     * @param pin     ADC pin number to read from
     * @param samples Number of samples averaged per reading
     * @param alpha   EMA smoothing factor (0–1); lower = smoother, higher = faster response
     */
    VoltageReader(uint8_t pin, uint8_t samples = 16, float alpha = 0.1f);

    /** Configures ADC attenuation and resolution. Call once before first read. */
    void init();

    /** Returns the averaged voltage in volts. */
    float readV();

    /** Returns the averaged voltage in millivolts. */
    uint32_t readMv();

    /** Returns the exponential moving average of voltage in volts. */
    float readSmooth();

private:
    uint8_t _pin;
    uint8_t _samples;
    float   _alpha;
    float   _ema;
};
