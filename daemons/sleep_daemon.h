#ifndef TERMINAL_SLEEP_DAEMON
#define TERMINAL_SLEEP_DAEMON
#include "daemon.h"
#include "../termscreen.h"
#include "../system/power.h"
#include <ion.h>

namespace Terminal {

namespace Background {

#define SLEEP_STAGE_PRESLEEP 8000
#define SLEEP_STAGE_SLEEP 10500

class SleepDaemon : public Daemon {
    int frameCounter = 0;

    SecuredString* name() override {
        return SecuredString::fromBufferUnsafe("auto-sleep");
    }

    void init() override { }
    void stop() override { }
    bool update() override {
        if (frameCounter > SLEEP_STAGE_SLEEP) {
            frameCounter = 0;
            Ion::Power::suspend(false);
            Ion::Backlight::setBrightness(Ion::Backlight::MaxBrightness);
            Terminal::Screen::redraw();
        } else if (frameCounter >= SLEEP_STAGE_PRESLEEP) {
            Ion::Backlight::setBrightness(Ion::Backlight::MaxBrightness - (uint8_t)(frameCounter / 10500.0f * 240));
        } else if (frameCounter < SLEEP_STAGE_PRESLEEP) {
            Ion::Backlight::setBrightness(Ion::Backlight::MaxBrightness);
        }

        frameCounter++;
        if (Terminal::Screen::anyKeyPressed()) {
            frameCounter = 0;
        }
        return true;
    }
};

}

}

#endif