#include "terminal.h"
#include "../../ion/include/ion.h"
#include <sigmap.h>
#include "list.h"
#include "stringx.h"
#include "commands.h"
#include "software/kilo.h"
#include <poincare/init.h>
#include <poincare/exception_checkpoint.h>
#include "system/users.h"
#include "system/power.h"
#include "firmware.h"

#include "local_commands.h"
#ifndef LOCAL_COMMANDS
#define LOCAL_COMMANDS 
#endif

void terminal_main(int argc, const char * const argv[]) {
    Terminal::isLaunchedFromFirmware = argc > 0 && check(argv[0], "-firm");

    if (!Terminal::isLaunchedFromFirmware) Ion::Backlight::init();
    else Terminal::Screen::redraw(true);

    Ion::Display::pushRectUniform(KDRect(0, 0, 320, 240), KDColorBlack);

    Terminal::Screen::write("L.E. Terminal ", KDColorGreen);
    Terminal::Screen::writeLn(TERMINAL_VERSION, KDColorGreen);
    Terminal::Screen::writeLn("(Lightweight Emulated)", KDColorGreen);
    Terminal::Screen::write(FIRMWARE_NAME, KDColorWhite);
    Terminal::Screen::write(" ", KDColorWhite);
    Terminal::Screen::write(FIRMWARE_BRANCH, FIRMWARE_BRANCH_COLOR);
    Terminal::Screen::write(" ", KDColorWhite);
    Terminal::Screen::write(FIRMWARE_VERSION, KDColorWhite);
    Terminal::Screen::newLine();

    char buffer[256];
    while (true) {
        Terminal::Screen::write(UsersRepository::sharedRepository()->current()->name(), KDColorGreen);
        Terminal::Screen::write("@numworks:/ ", KDColorGreen);
        Terminal::Screen::write("$ ", KDColorBlue);
        int readCount = Terminal::Screen::readLn(buffer);

        if (readCount < 0) break;

        SecuredStringList* argList = split(buffer, readCount, ' ');
        auto cmd = argList->at(0);

        Poincare::ExceptionCheckpoint ecp;
        if (ExceptionRun(ecp)) {
            if (check(cmd, "exit")) {
                return;
            } 
            LOCAL_COMMANDS
            DEFCMD("uname", command_uname)
            DEFCMD("echo", command_echo)
            DEFCMD("clear", command_clear)
            DEFCMD("history", command_history)
            DEFCMD("whoami", command_whoami)
            DEFCMD("ion", command_ion)
            DEFCMD("ls", command_ls)
            DEFCMD("touch", command_touch)
            DEFCMD("rm", command_rm)
            DEFCMD("cp", command_cp)
            DEFCMD("pyscr", command_pyscr)
            DEFCMD("cat", command_cat)
            DEFCMD("args", command_args)
            DEFCMD("chars", command_chars)
            DEFCMD("poincare", command_poincare)
            //DEFCMD("kilo", command_kilo)
            DEFCMD("su", command_su)
            DEFCMD("useradd", command_useradd)
            DEFCMD("users", command_users)
            DEFCMD("id", command_id)
            DEFCMD("neofetch", command_neofetch)
            else {
                if (argList->at(0).size() == 0) continue;
                Terminal::Screen::write("le: ");
                Terminal::Screen::write(cmd);
                Terminal::Screen::writeLn(" : invalid command");
            }
        } else {
            Terminal::Screen::write("le: ");
            Terminal::Screen::write(cmd);
            Terminal::Screen::writeLn(" : unknown error");
        }

        // Check if power button is pressed
        auto scan = Ion::Keyboard::scan();
        if (scan.keyDown(Ion::Keyboard::Key::OnOff)) { // Its ONOFF, NOT POWER !!!!!!!
            terminalSleep();
            Terminal::Screen::redraw(true);
        }
        
        // Update the LED
        Ion::LED::updateColorWithPlugAndCharge();
    }
}