#include "pressure_handler.h"
#include "config.h"
#include "Arduino.h"

static constexpr uint32_t BLINK_INTERVAL_MS = 300;

static bool          g_blinkActive = false;
static bool          g_ledState    = false;
static bool          g_ledToggled  = false;
static unsigned long g_prevBlinkMs = 0;

static void onUserDetection(Message& msg, const char* side) {
    if (msg.bytes_in_buffer < 1) return;
    if (msg.buffer[0]) {
        g_blinkActive = true;
        Serial.printf("[PRESSURE %s] hand detected\n", side);
    } else {
        g_blinkActive = false;
        digitalWrite(SenderConfig::LED_PIN, LOW);
        g_ledState = false;
        Serial.printf("[PRESSURE %s] no hand\n", side);
    }
}

static void onPressureUserDetectionL(Message& msg) { onUserDetection(msg, "L"); }
static void onPressureUserDetectionR(Message& msg) { onUserDetection(msg, "R"); }

static void onDoubleClick(Message& msg, const char* side) {
    g_ledToggled  = !g_ledToggled;
    g_blinkActive = false;
    g_ledState    = g_ledToggled;
    digitalWrite(SenderConfig::LED_PIN, g_ledToggled ? HIGH : LOW);
    Serial.printf("[DOUBLE CLICK %s] LED %s\n", side, g_ledToggled ? "on" : "off");
}

static void onPressureDoubleClickL(Message& msg) { onDoubleClick(msg, "L"); }
static void onPressureDoubleClickR(Message& msg) { onDoubleClick(msg, "R"); }

void pressureInit(NeffyInterface& iface) {
    pinMode(SenderConfig::LED_PIN, OUTPUT);
    digitalWrite(SenderConfig::LED_PIN, LOW);
    iface.addMethod(NeffyCommands::PRESSURE_USER_DETECTION_L.id, &onPressureUserDetectionL);
    iface.addMethod(NeffyCommands::PRESSURE_USER_DETECTION_R.id, &onPressureUserDetectionR);
    iface.addMethod(NeffyCommands::PRESSURE_DOUBLE_CLICK_L.id,   &onPressureDoubleClickL);
    iface.addMethod(NeffyCommands::PRESSURE_DOUBLE_CLICK_R.id,   &onPressureDoubleClickR);
}

void pressureUpdate(unsigned long nowMs) {
    if (g_blinkActive && nowMs - g_prevBlinkMs >= BLINK_INTERVAL_MS) {
        g_ledState    = !g_ledState;
        digitalWrite(SenderConfig::LED_PIN, g_ledState);
        g_prevBlinkMs = nowMs;
    }
}
