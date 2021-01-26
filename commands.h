#ifndef SIGMA_TERMINAL_COMMANDS
#define SIGMA_TERMINAL_COMMANDS
#include "list.h"
#include "termscreen.h"
#include "stringx.h"
#include <sigmap.h>
#include "../code/script_template.h"
#include "../code/script.h"
#include <poincare_layouts.h>
#include <poincare_nodes.h>
#include "../shared/poincare_helpers.h"
#include <poincare/init.h>
#include <poincare/exception_checkpoint.h>
#include "system/users.h"
#include "firmware.h"

void command_uname(SecuredStringList* args) {
    if (check(args->at(1), "-a")) {
        //Print all the system info in the following order :
        //Kernel name - network node hostname - kernel release date - kernel version - machine hard name - hard platform - OS
        Terminal::Screen::write("Ion, numworks, ");
        Terminal::Screen::write(Ion::softwareVersion());
        Terminal::Screen::write(", Numworks Calculator, ARM, ");
        Terminal::Screen::write(FIRMWARE_NAME);
        Terminal::Screen::write(" ");
        Terminal::Screen::write(FIRMWARE_BRANCH);
        Terminal::Screen::write(" ");
        Terminal::Screen::write(FIRMWARE_VERSION);
    } else if (check(args->at(1), "-s")) {
        Terminal::Screen::write("Ion");
    } else if (check(args->at(1), "-n")) {
        Terminal::Screen::write("numworks");
    } else if (check(args->at(1), "-r")) {
        Terminal::Screen::write("unknown");
    } else if (check(args->at(1), "-v")) {
        Terminal::Screen::write(Ion::softwareVersion());
    } else if (check(args->at(1), "-m")) {
        Terminal::Screen::write("Numworks Calculator");
    } else if (check(args->at(1), "-p") || check(args->at(1), "-i")) {
        Terminal::Screen::write("ARM");
    } else if (check(args->at(1), "-o")) {
        Terminal::Screen::write(FIRMWARE_NAME);
        Terminal::Screen::write(" ");
        Terminal::Screen::write(FIRMWARE_BRANCH);
        Terminal::Screen::write(" ");
        Terminal::Screen::write(FIRMWARE_VERSION);
    } else {
        Terminal::Screen::write("Ion");
    }
    Terminal::Screen::newLine();
}

void command_echo(SecuredStringList* args) {
    for (int i = 1; i < args->count(); i++) {
        Terminal::Screen::write(args->at(i));
        Terminal::Screen::write(" ");
    }
    Terminal::Screen::newLine();
}

void command_clear(SecuredStringList* args) {
    Terminal::Screen::clear();
}

void command_history(SecuredStringList* args) {
    if (args->count() == 1) {
        for (int i = 0; i < Terminal::Screen::history->count(); i++) {
            Terminal::Screen::write(Terminal::Screen::history->pointer() == i ? "->| " : "  | ");
            Terminal::Screen::write(Terminal::Screen::history->at(i));
            Terminal::Screen::newLine();
        }
    }
}

void command_whoami(SecuredStringList* args) {
    Terminal::Screen::writeLn(UsersRepository::sharedRepository()->current()->name());
}

// 'ion' command
// Options : 
// -s : sleep
// -D : dfu mode
// -S : standby
// -r : reboot (= reboot command)
// -b : battery info
// -u : usb info
void command_ion(SecuredStringList* args) {
    if (check(args->at(1), "-s")) {
        Ion::Power::suspend();
        Terminal::Screen::redraw(true);
    } else if (check(args->at(1), "-D")) {
        if (UsersRepository::sharedRepository()->current()->level() != ExecutionLevel::Root) {
            Terminal::Screen::writeLn("ion: access denied");
            return;
        }
        if (!Ion::USB::isPlugged()) {
            Terminal::Screen::writeLn("ion: device is not plugged (see ion -u for more info)");
            return;
        }
        Ion::USB::enable();
        Terminal::Screen::writeLn("USB enabled", KDColorWhite);
        Terminal::Screen::writeLn("--- Entering DFU mode (calculator is now frozen) ---", KDColorYellow);
        Terminal::Screen::writeLn("Press [BACK] to cancel", KDColorWhite);
        Ion::USB::DFU();
        Ion::LED::updateColorWithPlugAndCharge();
        Terminal::Screen::writeLn("Left DFU mode", KDColorWhite);
        Ion::USB::disable();
        Terminal::Screen::writeLn("USB disabled", KDColorWhite);
    } else if (check(args->at(1), "-S")) {
        if (UsersRepository::sharedRepository()->current()->level() < ExecutionLevel::High) {
            Terminal::Screen::writeLn("ion: access denied");
            return;
        }
        Ion::Power::standby();
        Terminal::Screen::redraw(true);
    } else if (check(args->at(1), "-r")) {
        Terminal::Screen::writeLn("ion: not supported");
    } else if (check(args->at(1), "-b")) {
        char* text = new char[16];

        if (Ion::Battery::level() == Ion::Battery::Charge::EMPTY) {
            text = "[---] Empty";
        } else if (Ion::Battery::level() == Ion::Battery::Charge::LOW) {
            text = "[#--] Low";
        } else if (Ion::Battery::level() == Ion::Battery::Charge::SOMEWHERE_INBETWEEN) {
            text = "[##-] Idle";
        } else if (Ion::Battery::level() == Ion::Battery::Charge::FULL) {
            text = "[###] Full";
        }

        Terminal::Screen::writeLn("Battery info : ");
        Terminal::Screen::write("  | Battery level : ");
        Terminal::Screen::write(text);
        Terminal::Screen::newLine();
        Terminal::Screen::write("  | Charging : ");
        if (Ion::Battery::isCharging()) Terminal::Screen::write("yes");
        else Terminal::Screen::write("no");
        Terminal::Screen::newLine();
    } else if (check(args->at(1), "-u")) {
        Terminal::Screen::write("USB info : ");
        Terminal::Screen::write("  | Plugged : ");
        if (Ion::USB::isPlugged()) Terminal::Screen::write("yes");
        else Terminal::Screen::write("no");
        Terminal::Screen::newLine();
        Terminal::Screen::write("  | Enumerated : ");
        if (Ion::USB::isEnumerated()) Terminal::Screen::write("yes");
        else Terminal::Screen::write("no");
        Terminal::Screen::newLine();
    } else {
        Terminal::Screen::write("ion: unknown option: ");
        Terminal::Screen::write(args->at(1));
        Terminal::Screen::newLine();
    }
}

void command_ls(SecuredStringList* args) {
    for (char* p : *Ion::Storage::sharedStorage()) {
        Terminal::Screen::write(p+2);
        Terminal::Screen::write(" ");
    }
    Terminal::Screen::newLine();
}

void command_rm(SecuredStringList* args) {
    if (args->count() == 1) {
        Terminal::Screen::writeLn("rm: no filename specified");
        return;
    }
    if (UsersRepository::sharedRepository()->current()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("rm: access denied");
        return;
    }
    auto record = Ion::Storage::sharedStorage()->recordNamed(args->at(1).c_str());

    if (record.isNull()) {
        Terminal::Screen::write("rm: no such file or directory: ");
        Terminal::Screen::write(args->at(1));
        Terminal::Screen::newLine();
        return;
    }

    record.destroy();
}

void command_touch(SecuredStringList* args) {
    if (args->count() == 1) {
        Terminal::Screen::writeLn("touch: no filename specified");
        return;
    }
    if (UsersRepository::sharedRepository()->current()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("touch: access denied");
        return;
    }
    auto filename = args->at(1).c_str();

    char data[1];
    data[0] = '\0';
    Ion::Storage::sharedStorage()->createRecordWithFullName((const char*)filename, (char*)data, 256);
}

void command_cp(SecuredStringList* args) {
    if (args->count() != 3) {
        Terminal::Screen::writeLn("usage : cp <from> <to>");
        return;
    }
    if (UsersRepository::sharedRepository()->current()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("cp: access denied");
        return;
    }

    auto from = args->at(1).c_str();
    auto to = args->at(2).c_str();

    auto fromRec = Ion::Storage::sharedStorage()->recordNamed(from);
    if (fromRec.isNull()) {
        Terminal::Screen::write("cp: ");
        Terminal::Screen::write(from);
        Terminal::Screen::write(" : no such file or directory");
        Terminal::Screen::newLine();
        return;
    }

    Ion::Storage::sharedStorage()->createRecordWithFullName(to, fromRec.value().buffer, fromRec.value().size);
}

void writeTemplate(const Code::ScriptTemplate* script) {
    size_t valueSize = Code::Script::StatusSize() + strlen(script->content()) + 1; // (auto importation status + content fetched status) + scriptcontent size + null-terminating char
    Ion::Storage::sharedStorage()->createRecordWithFullName(script->name(), script->value(), valueSize);
}

// pyscr command
// Creates the default python scripts found in epsilon
void command_pyscr(SecuredStringList* args) {
    if (UsersRepository::sharedRepository()->current()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("pyscr: access denied");
        return;
    }
    writeTemplate(Code::ScriptTemplate::Squares());
    writeTemplate(Code::ScriptTemplate::Parabola());
    writeTemplate(Code::ScriptTemplate::Mandelbrot());
    writeTemplate(Code::ScriptTemplate::Polynomial());
}

void command_cat(SecuredStringList* args) {
    if (UsersRepository::sharedRepository()->current()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("cat: access denied");
        return;
    }
    if (args->count() != 2) {
        Terminal::Screen::writeLn("usage : cat <file>");
        return;
    }

    auto fromRec = Ion::Storage::sharedStorage()->recordNamed(args->at(1).c_str());
    if (fromRec.isNull()) {
        Terminal::Screen::write("ls: ");
        Terminal::Screen::write(args->at(1));
        Terminal::Screen::write(" : no such file or directory");
        Terminal::Screen::newLine();
        return;
    }
    Terminal::Screen::writeLn((const char*)fromRec.value().buffer, KDColorWhite, fromRec.value().size);
}

void command_args(SecuredStringList* args) {
    for (int i = 0; i < args->count(); i++) {
        Terminal::Screen::write("  | ");
        Terminal::Screen::write(args->at(i));
        Terminal::Screen::newLine();
    }
    Terminal::Screen::newLine();
}

void command_chars(SecuredStringList* args) {
    for (int i = 0; i < 256; i++) {
        Terminal::Screen::write("  | ");
        Terminal::Screen::writeChar((char)i);
        Terminal::Screen::newLine();
    }
    Terminal::Screen::newLine();
}

void command_poincare(SecuredStringList* args) {
    char buffer[256];
    char calculationBuffer[256];
    char secCalculationBuffer[256];
    KDContext* ctx = KDIonContext::sharedContext();
    
    Terminal::Screen::writeLn("Poincare interactive shell");
    Terminal::Screen::writeLn("Type 'exit' to exit");
    while (true) {
        Terminal::Screen::write(">>> ");
        int c = Terminal::Screen::readLn(buffer);
        if (check(buffer, "exit")) break;
        Poincare::ExceptionCheckpoint ecp;
        if (ExceptionRun(ecp)) {
            Poincare::Expression simplified = Poincare::Expression();
            Poincare::Expression approximate = Poincare::Expression();
            Shared::PoincareHelpers::ParseAndSimplifyAndApproximate(buffer, &simplified, &approximate, nullptr);
            Poincare::Layout layout = approximate.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7);
            SecuredString approxStr = SecuredString(layout.serializeParsedExpression(calculationBuffer, 256, nullptr), calculationBuffer);
            Poincare::Layout simLayout = simplified.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7);
            SecuredString simplStr = SecuredString(simLayout.serializeParsedExpression(secCalculationBuffer, 256, nullptr), secCalculationBuffer);
            Terminal::Screen::write("  | ");
            Terminal::Screen::write(simplStr, KDColorOrange);
            Terminal::Screen::write(" ");
            Terminal::Screen::writeLn(approxStr, KDColorYellow);
        } else {
            Terminal::Screen::write("  | ");
            Terminal::Screen::writeLn("Error", KDColorRed);
        }
    }
}

void command_su(SecuredStringList* args) {
    if (args->count() == 1) {
        // Login into root
        bool result = UsersRepository::sharedRepository()->switchUser(*SecuredString::fromBufferUnsafe("root"));
        if (!result) {
            Terminal::Screen::writeLn("su: root: unable to login");
            return;
        }
        return;
    }

    SecuredString name = args->at(1);
    bool result = UsersRepository::sharedRepository()->switchUser(name);
    if (!result) {
        Terminal::Screen::write("su: ");
        Terminal::Screen::write(name);
        Terminal::Screen::writeLn(": unable to login");
        return;
    }
}

void command_useradd(SecuredStringList* args) {
    if (UsersRepository::sharedRepository()->current()->level() != ExecutionLevel::Root) {
        Terminal::Screen::writeLn("useradd: access denied");
        return;
    }
    if (args->count() != 3) {
        Terminal::Screen::writeLn("usage : useradd <username> <level>");
        return;
    }
    SecuredString username = args->at(1);
    SecuredString levelname = args->at(2);
    ExecutionLevel level = ExecutionLevel::Normal;

    if (levelname == *SecuredString::fromBufferUnsafe("low")) {
        level = ExecutionLevel::Low;
    } else if (levelname == *SecuredString::fromBufferUnsafe("normal")) {
        level = ExecutionLevel::Normal;
    } else if (levelname == *SecuredString::fromBufferUnsafe("high")) {
        level = ExecutionLevel::High;
    } else if (levelname == *SecuredString::fromBufferUnsafe("root")) {
        level = ExecutionLevel::Root;
    }

    bool result = UsersRepository::sharedRepository()->addUser(new User(username, level));
    if (!result) {
        Terminal::Screen::writeLn("useradd: unable to add user");
    }
}

void command_id(SecuredStringList* args) {
    User* usr = UsersRepository::sharedRepository()->current();

    SecuredString* levelStr;
    if (usr->level() == ExecutionLevel::Low) levelStr = SecuredString::fromBufferUnsafe("low");
    else if (usr->level() == ExecutionLevel::Normal) levelStr = SecuredString::fromBufferUnsafe("normal");
    else if (usr->level() == ExecutionLevel::High) levelStr = SecuredString::fromBufferUnsafe("high");
    else if (usr->level() == ExecutionLevel::Root) levelStr = SecuredString::fromBufferUnsafe("root");
    else levelStr = SecuredString::fromBufferUnsafe("unknown");
    Terminal::Screen::write(" |  uid=");
    Terminal::Screen::writeChar(intToString(usr->uid()));
    Terminal::Screen::write("(");
    Terminal::Screen::write(usr->name());
    Terminal::Screen::write(")");
    Terminal::Screen::write(" level=");
    Terminal::Screen::writeChar(intToString((int)usr->level()));
    Terminal::Screen::write("(");
    Terminal::Screen::write(*levelStr);
    Terminal::Screen::write(")");
    Terminal::Screen::newLine();
}

void command_users(SecuredStringList* args) {
    bool detailed = args->count() > 1 && args->at(1) == *SecuredString::fromBufferUnsafe("-d");
    for (int i = 0; i < UsersRepository::sharedRepository()->count(); i++) {
        User* usr = UsersRepository::sharedRepository()->at(i);
        if (detailed) {
            SecuredString* levelStr;
            if (usr->level() == ExecutionLevel::Low) levelStr = SecuredString::fromBufferUnsafe("low");
            else if (usr->level() == ExecutionLevel::Normal) levelStr = SecuredString::fromBufferUnsafe("normal");
            else if (usr->level() == ExecutionLevel::High) levelStr = SecuredString::fromBufferUnsafe("high");
            else if (usr->level() == ExecutionLevel::Root) levelStr = SecuredString::fromBufferUnsafe("root");
            else levelStr = SecuredString::fromBufferUnsafe("unknown");
            Terminal::Screen::write(" |  uid=");
            Terminal::Screen::writeChar(intToString(usr->uid()));
            Terminal::Screen::write("(");
            Terminal::Screen::write(usr->name());
            Terminal::Screen::write(")");
            Terminal::Screen::write(" level=");
            Terminal::Screen::writeChar(intToString((int)usr->level()));
            Terminal::Screen::write("(");
            Terminal::Screen::write(*levelStr);
            Terminal::Screen::write(")");
            Terminal::Screen::newLine();
        } else {
            Terminal::Screen::write(usr->name());
            Terminal::Screen::write(" ");
        }
    }
    if (!detailed) Terminal::Screen::newLine();
}

void command_neofetch(SecuredStringList* args) {
    Terminal::Screen::writeBitmap(neofetch_logo, NEOFETCH_LOGO_WIDTH, NEOFETCH_LOGO_HEIGHT, FIRMWARE_MAIN_COLOR);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write(UsersRepository::sharedRepository()->current()->name(), FIRMWARE_MAIN_COLOR);
    Terminal::Screen::writeLn("@numworks", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::writeLn("-----------------------", KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("Model: ", FIRMWARE_MAIN_COLOR);
    // Add a way, if possible, to fetch automatically the calculator revision
    Terminal::Screen::writeLn("Numworks Calculator", KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("OS: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::writeLn(FIRMWARE_NAME, KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("Kernel: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::write("Ion ", KDColorWhite);
    Terminal::Screen::writeLn(Ion::softwareVersion(), KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("Terminal: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::writeLn("L.E. Terminal ", KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
}

#endif