#include "kilo.h"

namespace Kilo {

using namespace Terminal::Screen;

void refreshTopBar() {
    LECurses::topBar(KDColorYellow, KDColorWhite, "LE Kilo 1.0");

}

void run(SecuredString filename) {
    clear();
    
}

}

void command_kilo(SecuredStringList* args) {
    auto record = Ion::Storage::sharedStorage()->recordNamed(args->at(1).c_str());
    if (record.isNull()) {
        Ion::Storage::sharedStorage()->createRecordWithFullName(args->at(1).c_str(), "", 0);
    }

    Kilo::run(args->at(1));
}