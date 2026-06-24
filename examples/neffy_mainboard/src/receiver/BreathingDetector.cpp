#include "BreathingDetector.h"
#include "Arduino.h"

static constexpr float    PRESS_THRESHOLD_V = 0.080f; // same threshold as hand detection
static constexpr uint32_t SESSION_MS        = 20000;
static constexpr uint32_t READ_INTERVAL_MS  = 20;
static constexpr uint8_t  BREATHS_TO_BPM   = 3; // breaths/20s × 3 = breaths/min

BreathingDetector::BreathingDetector(NeffyInterface& iface, VoltageReader& voltage)
    : _iface(iface), _voltage(voltage) {}

void BreathingDetector::init() {
    _breathMsg.command         = NeffyCommands::BREATHING_RATE.id;
    _breathMsg.bytes_in_buffer = NeffyCommands::BREATHING_RATE.payloadLength;
}

void BreathingDetector::start(unsigned long nowMs) {
    _baseline    = _voltage.readSmooth();
    _running     = true;
    _done        = false;
    _startMs     = nowMs;
    _prevReadMs  = nowMs;
    _breathCount = 0;
    _resultBpm   = 0;
    _prevPressed = false;
}

bool    BreathingDetector::isDone() const { return _done; }
uint8_t BreathingDetector::getBpm()  const { return _resultBpm; }

void BreathingDetector::update(unsigned long nowMs) {
    if (!_running || _done) return;
    if (nowMs - _prevReadMs < READ_INTERVAL_MS) return;
    _prevReadMs = nowMs;

    float v       = _voltage.readSmooth();
    bool  pressed = (v - _baseline) > PRESS_THRESHOLD_V;

    if (pressed && !_prevPressed) _breathCount++;
    _prevPressed = pressed;

    if (nowMs - _startMs >= SESSION_MS) {
        _resultBpm           = (uint8_t)((uint32_t)_breathCount * BREATHS_TO_BPM);
        _breathMsg.buffer[0] = _resultBpm;
        _iface.writeMessage(_breathMsg);
        _running = false;
        _done    = true;
    }
}
