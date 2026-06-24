#include "Arduino.h"
#include "Neffy_Interface.h"
#include "driver/gpio.h"
#include "config.h"
#include "VoltageReader.h"
#include "PressureDetector.h"
#include "BreathingDetector.h"

NeffyInterface    interface;
VoltageReader     voltageL(ReceiverConfig::PRESSURE_PIN_L, 8, 0.3f);
VoltageReader     voltageR(ReceiverConfig::PRESSURE_PIN_R, 8, 0.3f);
PressureDetector  pressureL(interface, voltageL, true);
PressureDetector  pressureR(interface, voltageR, false);
BreathingDetector breathing(interface, voltageL);

bool          prevHandL      = false;
bool          prevHandR      = false;
unsigned long breathingArmMs = 0;

void setup() {
    Serial.begin(ReceiverConfig::BAUDRATE);
    gpio_reset_pin(ReceiverConfig::LED_PIN);
    gpio_set_direction(ReceiverConfig::LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ReceiverConfig::LED_PIN, 0);
    interface.initUART(ReceiverConfig::RX_PIN, ReceiverConfig::TX_PIN, ReceiverConfig::BAUDRATE, UART_NUM_0);
    voltageL.init();
    voltageR.init();
    pressureL.init();
    pressureR.init();
    breathing.init();
    Serial.println("=== Pressure Test ===");
}

void loop() {
    interface.update();
    unsigned long nowMs = millis();
    pressureL.update(nowMs);
    pressureR.update(nowMs);
    breathing.update(nowMs);

    bool handL = pressureL.handOn();
    bool handR = pressureR.handOn();

    if (handL != prevHandL) {
        Serial.println(handL ? "[PRESSURE L] hand detected" : "[PRESSURE L] no hand");
        prevHandL = handL;
    }
    if (handR != prevHandR) {
        Serial.println(handR ? "[PRESSURE R] hand detected" : "[PRESSURE R] no hand");
        prevHandR = handR;
    }

    if (pressureL.click()) Serial.println("[CLICK L]");
    if (pressureR.click()) Serial.println("[CLICK R]");

    bool dcL = pressureL.doubleClick();
    bool dcR = pressureR.doubleClick();
    if (dcL || dcR) {
        breathingArmMs = nowMs;
        Serial.println("[DOUBLE CLICK] breathing starts in 3s...");
    }

    if (breathingArmMs != 0 && nowMs - breathingArmMs >= ReceiverConfig::BREATHING_DELAY_MS) {
        breathingArmMs = 0;
        breathing.start(nowMs);
        Serial.println("[BREATHING] session started (20s)");
    }

    static bool breathingDonePrev = false;
    bool        breathingDoneNow  = breathing.isDone();
    if (breathingDoneNow && !breathingDonePrev) {
        Serial.print("[BREATHING DONE] ");
        Serial.print(breathing.getBpm());
        Serial.println(" BPM");
    }
    breathingDonePrev = breathingDoneNow;
}
