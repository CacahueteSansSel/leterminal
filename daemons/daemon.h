#ifndef TERMINAL_DAEMON
#define TERMINAL_DAEMON
#include "../stringx.h"
#include "../list.h"

#define MAX_DAEMON_COUNT 16

namespace Terminal::Background {

// Like UNIX, the Daemon is a piece of code who runs in background
// Here, we simulate it, of course
// See docs/adding_daemon.md

class Daemon {
    public:

    Daemon() {}
    virtual SecuredString* name() = 0;
    virtual void init() = 0;
    virtual bool update() = 0;
    virtual void stop() = 0;
};

class Hell {
    public:

    Hell() {}

    static Hell* shared() {
        static Hell hell;

        return &hell;
    }

    int indexOfDaemonNamed(SecuredString* name) {
        for (int i = 0; i < daemonPointer; i++) {
            if (daemons[i] == nullptr) continue;
            if (*daemons[i]->name() == *name) return i;
        }

        return -1;
    }

    int nextFreeIndex() {
        for (int i = 0; i < daemonPointer; i++) {
            if (daemons[i] == nullptr) return i;
        }

        return daemonPointer;
    }

    int runningCount() {
        return daemonPointer;
    }

    Daemon* at(int index) {
        if (index >= daemonPointer) return nullptr;

        return daemons[index];
    }

    bool summon(Daemon* daemon) {
        if (daemonPointer >= MAX_DAEMON_COUNT) return false;
        int index = nextFreeIndex();
        daemons[index] = daemon;
        daemons[index]->init();
        if (index == daemonPointer) daemonPointer++;

        return true;
    }

    bool kill(SecuredString* name) {
        int index = indexOfDaemonNamed(name);
        if (index < 0) return false;

        daemons[index]->stop();
        daemons[index] = nullptr;
        return true;
    }

    void dispatchInit() {
        for (int i = 0; i < daemonPointer; i++) {
            if (daemons[i] == nullptr) continue;
            daemons[i]->init();
        }
    }

    void dispatchStop() {
        for (int i = 0; i < daemonPointer; i++) {
            if (daemons[i] == nullptr) continue;
            daemons[i]->stop();
        }
    }

    void dispatchUpdate() {
        for (int i = 0; i < daemonPointer; i++) {
            if (daemons[i] == nullptr) continue;
            if (!daemons[i]->update()) {
                kill(daemons[i]->name());
            }
        }
    }

    private:
    Daemon* daemons[MAX_DAEMON_COUNT];
    int daemonPointer = 0;
};

}

#endif