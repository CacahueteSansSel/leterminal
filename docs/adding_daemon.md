# Adding a daemon

L.E. Terminal introduces in version 1.5.0 the concept of daemons.

Like in Linux, in *let*, daemons are piece of code that runs in background

*Here's how we create a new daemon :*

Create a new header file in the `daemons/` folder.

Inside this file, create the below daemon class :

*Here is a example daemon that sets the backlight level to 0 when hitting the power key :*

```c++
#ifndef TERMINAL_MYDAEMON_DAEMON
#define TERMINAL_MYDAEMON_DAEMON
#include "../stringx.h"
#include "daemon.h"
#include <ion.h>

namespace Terminal {

namespace Background {

class MyDaemon : public Daemon {
     
    SecuredString* name() override {
        return SecuredString::fromBufferUnsafe("my-daemon");
    }

    void init() override {
        // Called when the daemon starts up
    }
    
    void stop() override {
        // Called when the daemon shuts down
    }
    
    bool update() override {
        // Called every frame across the terminal
        if (Terminal::Screen::keyPressed(Ion::Keyboard::Key::Power)) {
            Ion::Backlight::setBrightness(0);
        }
        return true;
    }
};

}

}

#endif
```

After that, go into the `terminal.cpp` file and register the daemon : 
```c++
// Registering daemons
// [...]
Terminal::Background::Hell::shared()->summon(new Terminal::Background::MyDaemon());
```

And you now have your little daemon running in *the (s)hell* !