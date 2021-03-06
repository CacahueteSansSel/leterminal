#ifndef TERMINAL_H
#define TERMINAL_H
#include "termscreen.h"

#define DEFCMD(cmdname, cmdfunc) else if (check(cmd, cmdname)) { cmdfunc(argList); }
#define TERMINAL_VERSION "1.6.0"

void terminal_main(int argc, const char * const argv[]);

#endif