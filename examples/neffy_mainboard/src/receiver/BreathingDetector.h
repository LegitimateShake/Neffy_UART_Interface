#pragma once
#include "Neffy_Interface.h"
#include "VoltageReader.h"

/**
 * Measures breathing rate over a fixed 20-second session using the pressure sensor.
 * Counts press events (breaths) and sends the result in BPM via UART when done.
 */
class BreathingDetector {
public:
    /**
     * @param iface   Reference to the UART interface
     * @param voltage Shared VoltageReader for the pressure sensor pin
     */
    BreathingDetector(NeffyInterface& iface, VoltageReader& voltage);

    /** Sets up the outgoing UART message. Call once in setup(). */
    void init();

    /**
     * Starts a new measurement session. Resets all counters and captures current baseline.
     * @param nowMs Current time from millis().
     */
    void start(unsigned long nowMs);

    /**
     * Updates the measurement. Call every loop iteration.
     * @param nowMs Current time from millis().
     */
    void update(unsigned long nowMs);

    /** Returns true when the current session has finished and a result is available. */
    bool isDone() const;

    /** Returns the measured breathing rate in BPM. Only valid after isDone() returns true. */
    uint8_t getBpm() const;

private:
    NeffyInterface& _iface;
    VoltageReader&  _voltage;
    Message         _breathMsg;

    float         _baseline    = 0.0f;
    bool          _running     = false;
    bool          _done        = false;
    unsigned long _startMs     = 0;
    unsigned long _prevReadMs  = 0;
    uint8_t       _breathCount = 0;
    uint8_t       _resultBpm   = 0;
    bool          _prevPressed = false;
};
