#include "person_handler.h"
#include "config.h"
#include "Arduino.h"

static constexpr uint32_t BLINK_INTERVAL_MS = 300;

static bool          g_blinkActive = false;
static bool          g_ledState    = false;
static unsigned long g_prevBlinkMs = 0;

static void onPersonReading(Message& msg) {
    if (msg.bytes_in_buffer < 1) return;
    switch (msg.buffer[0]) {
        case 0:
            g_blinkActive = false;
            digitalWrite(SenderConfig::LED_PIN, LOW);
            Serial.println("[PERSON] nobody present");
            break;
        case 1:
            g_blinkActive = true;
            Serial.println("[PERSON] passing by");
            break;
        case 2:
            g_blinkActive = false;
            digitalWrite(SenderConfig::LED_PIN, HIGH);
            Serial.println("[PERSON] sitting in front");
            break;
    }
}

void personInit(NeffyInterface& iface) {
    pinMode(SenderConfig::LED_PIN, OUTPUT);
    digitalWrite(SenderConfig::LED_PIN, LOW);
    iface.addMethod(NeffyCommands::GET_PERSON_READING.id, &onPersonReading);
}

void personUpdate(unsigned long nowMs) {
    if (g_blinkActive && nowMs - g_prevBlinkMs >= BLINK_INTERVAL_MS) {
        g_ledState    = !g_ledState;
        digitalWrite(SenderConfig::LED_PIN, g_ledState);
        g_prevBlinkMs = nowMs;
    }
}
