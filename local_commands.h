#ifndef TERMINAL_LOC_CMDS
#define TERMINAL_LOC_CMDS
#include "list.h"
#include "terminal.h"
#include "termscreen.h"
#include "stringx.h"

// Here you can define your own commands
// See docs/adding_command.md

#define LOCAL_COMMANDS DEFCMD("test", command_test)

void command_test(SecuredStringList* args) {
    Terminal::Screen::writeLn("Hello, World !");
}

#endif