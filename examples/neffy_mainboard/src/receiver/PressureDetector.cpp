#include "PressureDetector.h"
#include "Arduino.h"

static constexpr uint32_t READ_INTERVAL_MS      = 20;
static constexpr float    HAND_THRESHOLD_V       = 0.080f; // above ambient_baseline → hand present
static constexpr float    CLICK_THRESHOLD_V      = 0.020f; // above fast_baseline → click
static constexpr uint32_t HAND_OFF_DELAY_MS      = 1500;
static constexpr uint32_t DOUBLE_CLICK_WINDOW_MS = 600;
static constexpr float    AMBIENT_ADAPT          = 0.005f;
static constexpr float    FAST_ADAPT             = 0.1f;

PressureDetector::PressureDetector(NeffyInterface& iface, VoltageReader& voltage, bool isLeft)
    : _iface(iface), _voltage(voltage), _isLeft(isLeft) {}

void PressureDetector::init() {
    float v          = _voltage.readSmooth();
    _ambientBaseline = v;
    _fastBaseline    = v;

    _presenceMsg.command         = _isLeft ? NeffyCommands::PRESSURE_USER_DETECTION_L.id            : NeffyCommands::PRESSURE_USER_DETECTION_R.id;
    _presenceMsg.bytes_in_buffer = _isLeft ? NeffyCommands::PRESSURE_USER_DETECTION_L.payloadLength : NeffyCommands::PRESSURE_USER_DETECTION_R.payloadLength;

    _doubleClickMsg.command         = _isLeft ? NeffyCommands::PRESSURE_DOUBLE_CLICK_L.id            : NeffyCommands::PRESSURE_DOUBLE_CLICK_R.id;
    _doubleClickMsg.bytes_in_buffer = _isLeft ? NeffyCommands::PRESSURE_DOUBLE_CLICK_L.payloadLength : NeffyCommands::PRESSURE_DOUBLE_CLICK_R.payloadLength;
}

bool PressureDetector::handOn() const { return _handLast; }

bool PressureDetector::click() {
    if (_clickPending) { _clickPending = false; return true; }
    return false;
}

bool PressureDetector::doubleClick() {
    if (_doubleClickPending) { _doubleClickPending = false; return true; }
    return false;
}

void PressureDetector::update(unsigned long nowMs) {
    if (nowMs - _prevReadMs < READ_INTERVAL_MS) return;
    _prevReadMs = nowMs;

    float v       = _voltage.readSmooth();
    bool  handNow = (v - _ambientBaseline) > HAND_THRESHOLD_V;

    // ambient_baseline: only adapts when no hand → tracks true ambient
    if (!handNow) {
        _ambientBaseline = _ambientBaseline * (1.0f - AMBIENT_ADAPT) + v * AMBIENT_ADAPT;
    }

    // fast_baseline: always adapts (~200ms time constant)
    // follows current resting pressure regardless of hand state
    // a ~100ms click spike is only ~40% absorbed → spike still detectable above CLICK_THRESHOLD_V
    _fastBaseline = _fastBaseline * (1.0f - FAST_ADAPT) + v * FAST_ADAPT;

    bool clickNow = (v - _fastBaseline) > CLICK_THRESHOLD_V;

    if (handNow) {
        _handOffSince = 0;
        if (!_handLast) {
            _handLast              = true;
            _presenceMsg.buffer[0] = 1;
            _iface.writeMessage(_presenceMsg);
        }
    } else if (_handLast) {
        if (_handOffSince == 0) _handOffSince = nowMs;
        if (nowMs - _handOffSince >= HAND_OFF_DELAY_MS) {
            _handLast              = false;
            _handOffSince          = 0;
            _clickLast             = false;
            _lastClickMs           = 0;
            _presenceMsg.buffer[0] = 0;
            _iface.writeMessage(_presenceMsg);
        }
    }

    if (clickNow && !_clickLast) {
        _clickPending = true;
        if (_lastClickMs != 0 && nowMs - _lastClickMs < DOUBLE_CLICK_WINDOW_MS) {
            _doubleClickPending = true;
            _iface.writeMessage(_doubleClickMsg);
            _lastClickMs = 0;
        } else {
            _lastClickMs = nowMs;
        }
    }
    _clickLast = clickNow;
}
