#pragma once
#include "Neffy_Interface.h"
#include "VoltageReader.h"

/**
 * Detects hand presence, single clicks, and double clicks using a capacitive pressure sensor.
 * Sends UART messages on state changes.
 */
class PressureDetector {
public:
    /**
     * @param iface   Reference to the UART interface
     * @param voltage Shared VoltageReader for the pressure sensor pin
     * @param isLeft  true = left hand (uses _L commands), false = right hand (uses _R commands)
     */
    PressureDetector(NeffyInterface& iface, VoltageReader& voltage, bool isLeft);

    /** Seeds the voltage baselines. Call once after VoltageReader::init(). */
    void init();

    /**
     * Reads the sensor and updates detection state. Call every loop iteration.
     * @param nowMs Current time from millis().
     */
    void update(unsigned long nowMs);

    /** Returns true if a hand is currently resting on the sensor. */
    bool handOn() const;

    /** Returns true and consumes the event if a click was detected since last call. */
    bool click();

    /** Returns true and consumes the event if a double-click was detected since last call. */
    bool doubleClick();

private:
    NeffyInterface& _iface;
    VoltageReader&  _voltage;
    bool            _isLeft;
    Message         _presenceMsg;
    Message         _doubleClickMsg;

    bool          _handLast           = false;
    float         _ambientBaseline    = 0.0f;
    float         _fastBaseline       = 0.0f;
    bool          _clickLast          = false;
    bool          _clickPending       = false;
    bool          _doubleClickPending = false;
    unsigned long _handOffSince       = 0;
    unsigned long _lastClickMs        = 0;
    unsigned long _prevReadMs         = 0;
};
