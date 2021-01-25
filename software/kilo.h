#ifndef TERMINAL_KILO
#define TERMINAL_KILO
#include "../list.h"
#include "../termscreen.h"
#include "../stringx.h"
#include <ion.h>
#include "../lecurses.h"

using namespace Terminal::Screen;

namespace Kilo {

void refreshTopBar();
void run(SecuredString filename);

}

void command_kilo(SecuredStringList* args);

#endif