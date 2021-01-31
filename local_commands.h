#ifndef TERMINAL_LOC_CMDS
#define TERMINAL_LOC_CMDS
#include "list.h"
#include "terminal.h"
#include "termscreen.h"
#include "stringx.h"
#include <ion.h>

// Here you can define your own commands
// See docs/adding_command.md

float frandom() {
    return (float)Ion::random() / 0x7FFF;
}

#define LOCAL_COMMANDS DEFCMD("test", command_test) \
DEFCMD("matrix", command_matrix)

void command_test(SecuredStringList* args) {
    Terminal::Screen::writeLn("Hello, World !");
}

void command_matrix(SecuredStringList* args) {
    Terminal::Screen::clear();
    
    while (Terminal::Screen::safeLoop()) {
        for (int x = 0; x < Terminal::Screen::CharWidth; x++) {
            Terminal::Screen::posX = x;
            KDColor color = KDColor::RGB888(0, (uint8_t)(frandom() * 255), 0);

            Terminal::Screen::writeCharSilentlyWithoutRedrawing((char)(128 * frandom()), color);
        }

        Terminal::Screen::newLine();
        Terminal::Screen::redraw();
    }
    Terminal::Screen::clear();
    Terminal::Screen::posX = 0;
    Terminal::Screen::posY = 0;
}

#endif