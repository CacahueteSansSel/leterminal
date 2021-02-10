#include "terminal.h"
#include "../../ion/include/ion.h"
#include "../../apps/global_preferences.h"
#include "list.h"
#include "stringx.h"
#include "commands/commands.h"
#include "software/kilo.h"
#include <poincare/init.h>
#include <poincare/exception_checkpoint.h>
#include "system/users.h"
#include "system/power.h"
#include "firmware.h"
#include "vfs/vfs.h"
#include "events.h"
#include "external/heap.h"
#include "palette.h"
#include "daemons/daemon.h"
#include "daemons/powercheck_daemon.h"
#include "daemons/sleep_daemon.h"
#include "daemons/led_update_daemon.h"
#include "splashes.h"

#include "commands/local_commands.h"
#ifndef LOCAL_COMMANDS
#define LOCAL_COMMANDS 
#endif

void terminal_main(int argc, const char * const argv[]) {
    Terminal::isLaunchedFromFirmware = argc > 0 && check(argv[0], "-firm");

    if (!Terminal::isLaunchedFromFirmware) Ion::Backlight::init();
    else Terminal::Screen::redraw(true);

    Ion::Display::pushRectUniform(KDRect(0, 0, 320, 240), KDColorBlack);

    // Registering daemons
    Terminal::Background::Hell::shared()->summon(new Terminal::Background::PowerCheckDaemon());
    Terminal::Background::Hell::shared()->summon(new Terminal::Background::SleepDaemon());
    Terminal::Background::Hell::shared()->summon(new Terminal::Background::LEDUpdateDaemon());

    Terminal::Background::Hell::shared()->dispatchInit();
    Terminal::VFS::VirtualFS::sharedVFS()->init();
    Terminal::Screen::init();
    Terminal::Screen::clear();
    Terminal::Screen::write("L.E. Terminal v", TerminalGreen);
    Terminal::Screen::writeLn(TERMINAL_VERSION, TerminalGreen);
    Terminal::Screen::newLine();

    SecuredStringList* argList = new SecuredStringList();
    static char commandBuffer[256];
    while (true) {
        Terminal::Screen::keyRead();

        if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode()) {
            Terminal::Screen::write("EXAM ", TerminalIndigo);
        }
        Terminal::Screen::write(UsersRepository::sharedRepository()->current()->name(), TerminalGreen);
        Terminal::Screen::write("@numworks:", TerminalGreen);
        Terminal::Screen::write(Terminal::VFS::VirtualFS::sharedVFS()->current()->name(), TerminalGreen);
        Terminal::Screen::write(" $ ", TerminalBlue);
        int readCount = Terminal::Screen::readLn(commandBuffer);

        if (readCount == ESOFTRESET) {
            Terminal::Screen::clear();
            Terminal::Screen::history->clear();
        }
        if (readCount == EBREAK) continue;

        if (readCount != 0) {
            split(argList, commandBuffer, readCount, ' ');
            auto cmd = argList->at(0);

            Poincare::ExceptionCheckpoint ecp;
            if (ExceptionRun(ecp)) {
                if (check(cmd, "exit")) {
                    return;
                } else if (startsWith(cmd->c_str(), "./") || startsWith(cmd->c_str(), "*./")) {
                    bool forceExecute = startsWith(cmd->c_str(), "*./");
                    char* buffer = cmd->c_str() + (forceExecute ? 3 : 2);
                    int size = cmd->size() - (forceExecute ? 3 : 2);

                    auto node = Terminal::VFS::VirtualFS::sharedVFS()->fetch(SecuredString::fromBufferUnsafe(buffer));
                    if (node == nullptr) {
                        Terminal::Screen::write("le: ");
                        Terminal::Screen::write(buffer, KDColorWhite, size);
                        Terminal::Screen::writeLn(" : no such executable file");
                    } else if (!node->isExecutable() && !forceExecute) {
                        Terminal::Screen::write("le: ");
                        Terminal::Screen::write(buffer, KDColorWhite, size);
                        Terminal::Screen::writeLn(" : not a executable file");
                    } else {
                        // Allocating the heap
                        if (!allocateHeap()) {
                            Terminal::Screen::writeLn("le: failed to allocate heap : not enough memory available");
                        } else {
                            int exitCode = node->execute(forceExecute);
                            // Freeing the heap
                            freeHeap();

                            Terminal::Screen::write(buffer, KDColorWhite, size);
                            Terminal::Screen::write(": exit (");
                            Terminal::Screen::writeChar(intToString(exitCode));
                            Terminal::Screen::write(")");
                            Terminal::Screen::newLine();
                        }
                    }
                    Ion::LED::updateColorWithPlugAndCharge();
                    Terminal::Screen::keyEnd();
                    argList->dispose();
                    continue;
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
                DEFCMD("cd", command_cd)
                DEFCMD("mkdir", command_mkdir)
                DEFCMD("md", command_mkdir)
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
                DEFCMD("daemon", command_daemon)
                DEFCMD("chmod", command_chmod)
                else {
                    if (argList->at(0)->size() == 0) continue;
                    Terminal::Screen::write("le: ");
                    Terminal::Screen::write(cmd);
                    Terminal::Screen::writeLn(" : invalid command");
                }
            } else {
                Terminal::Screen::write("le: ");
                Terminal::Screen::write(cmd);
                Terminal::Screen::writeLn(" : unknown error");
            }
        }

        // Check if power button is pressed
        if (keyPressed(Ion::Keyboard::Key::OnOff)) { // Its ONOFF, NOT POWER !!!!!!!
            terminalSleep();
            Terminal::Screen::redraw(true);
        }
        
        // Update the LED
        Ion::LED::updateColorWithPlugAndCharge();
        Terminal::Screen::keyEnd();
        argList->dispose();
    }
    Terminal::Background::Hell::shared()->dispatchStop();
}