#pragma once
#include "Neffy_Interface.h"

/** Configures the LED pin and registers UART callbacks for pressure events. */
void pressureInit(NeffyInterface& iface);

/** Drives LED blinking when a hand is present. Call every loop iteration. */
void pressureUpdate(unsigned long nowMs);
