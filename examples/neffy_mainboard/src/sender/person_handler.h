#pragma once
#include "Neffy_Interface.h"

/** Configures the LED pin and registers the UART callback for person state updates. */
void personInit(NeffyInterface& iface);

/** Drives LED blinking when in PASSING state. Call every loop iteration. */
void personUpdate(unsigned long nowMs);
