#ifndef TERMINAL_STARTUP
#define TERMINAL_STARTUP
#include <ion.h>
#include <kandinsky.h>
#include "terminal.h"

static void terminal_startup_check(int argc, const char * const argv[]) {
    Ion::Display::pushRectUniform(KDRect(0, 0, 320, 240), KDColorBlack);
    Ion::Backlight::init();

    // Check if home key pressed to launch terminal
    for (int i = 0; i < 10; i++) {
        auto keys = Ion::Keyboard::scan();
        if (keys.keyDown(Ion::Keyboard::Key::Home)) {
        // Launch terminal "subfirmware"
        terminal_main(argc, argv);
        break;
        }
        Ion::Timing::msleep(100);
    }
}

#endif