#ifndef TERMINAL_POWER
#define TERMINAL_POWER
#include <ion.h>

static void terminalSleep() {
    Ion::Power::suspend(true);
}

#endif