#pragma once
#include <Arduino.h>

enum class PersonState : uint8_t { NONE = 0, PASSING = 1, PRESENT = 2 };

/**
 * Combines the D6T-44L thermal sensor with person detection logic.
 * Reads ambient temperature via I2C and classifies presence as NONE, PASSING, or PRESENT.
 */
class PersonSensor {
public:
    /** Initialises I2C and seeds the temperature baseline from the first reading. */
    void init();

    /**
     * Reads temperature and updates the detection state machine. Call periodically.
     * @return Current PersonState
     */
    PersonState update();

    /** Returns the raw temperature from the last update() call in degrees Celsius. */
    float lastTemp() const { return _lastTemp; }

private:
    float         _baseline       = 0.0f;
    float         _lastTemp       = 0.0f;
    unsigned long _presentSinceMs = 0;
    bool          _presentPending = false;

    float readTemp();
};
