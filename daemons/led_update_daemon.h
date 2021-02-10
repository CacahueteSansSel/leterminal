#ifndef TERMINAL_LED_DAEMON
#define TERMINAL_LED_DAEMON
#include "daemon.h"
#include <ion.h>
#include <apps/global_preferences.h>

namespace Terminal {

namespace Background {

class LEDUpdateDaemon : public Daemon {
    int frameCounter = 0;

    SecuredString* name() override {
        return SecuredString::fromBufferUnsafe("led-update");
    }

    void init() override { }
    void stop() override { }
    bool update() override {

        if (Ion::USB::isPlugged()) {
            if (Ion::Battery::level() == Ion::Battery::Charge::FULL) Ion::LED::setColor(KDColorGreen);
            else Ion::LED::setColor(Ion::Battery::isCharging() ? KDColorOrange : KDColorGreen);
        } else if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode()) {
            frameCounter++;
            if (frameCounter >= 1000) {
                frameCounter = 0;
            }

            Ion::LED::setColor(frameCounter == 0 ? KDColorRed : KDColorBlack);
        } else Ion::LED::setColor(KDColorBlack);

        return true;
    }
};

}

}

#endif