#include "breathing_handler.h"
#include "Arduino.h"

static void onBreathingRate(Message& msg) {
    if (msg.bytes_in_buffer < NeffyCommands::BREATHING_RATE.payloadLength) return;
    Serial.print("[BREATHING] ");
    Serial.print(msg.buffer[0]);
    Serial.println(" BPM");
}

void breathingInit(NeffyInterface& iface) {
    iface.addMethod(NeffyCommands::BREATHING_RATE.id, &onBreathingRate);
}
