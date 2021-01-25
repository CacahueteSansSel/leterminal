#ifndef SIGMA_TERMINAL
#define SIGMA_TERMINAL
#include "termscreen.h"

#define DEFCMD(cmdname, cmdfunc) else if (check(cmd, cmdname)) { cmdfunc(argList); }
#define TERMINAL_VERSION "1.3.0"

void terminal_main(int argc, const char * const argv[]);

#endif