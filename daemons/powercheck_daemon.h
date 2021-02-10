#ifndef TERMINAL_POWERCHECK_DAEMON
#define TERMINAL_POWERCHECK_DAEMON
#include "daemon.h"
#include "../termscreen.h"
#include "../system/power.h"
#include <ion.h>

namespace Terminal {

namespace Background {

class PowerCheckDaemon : public Daemon {
     
    SecuredString* name() override {
        return SecuredString::fromBufferUnsafe("power-check");
    }

    void init() override { }
    void stop() override { }
    bool update() override {
        if (Terminal::Screen::keyPressed(Ion::Keyboard::Key::OnOff)) {
            terminalSleep();
            Terminal::Screen::redraw();
        }
        return true;
    }
};

}

}

#endif