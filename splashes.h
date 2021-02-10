#ifndef TERMINAL_SPLASHES
#define TERMINAL_SPLASHES
#include <ion.h>

#define SPLASH_COUNT 4

static const char* splashes[SPLASH_COUNT] = {
    "If you don't want your screen to automatically turn off, type daemon kill auto-sleep to kill the daemon",
    "L.E. means Lightweight Emulated, but it is also a reference to the 'le' french determiner",
    "The matrix command will never display characters above the 128th",
    "Imagine if your teacher sees this..."
};

static const char* randomSplash() {
    int random = (int)((float)Ion::random() / (float)0x7FFF * SPLASH_COUNT);
    if (random > SPLASH_COUNT-1) random = SPLASH_COUNT-1;
    else if (random < 0) random = 0;

    return splashes[random];
}

#endif