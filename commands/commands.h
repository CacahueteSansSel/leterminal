#ifndef TERMINAL_COMMANDS
#define TERMINAL_COMMANDS
#include "../list.h"
#include "../termscreen.h"
#include "../stringx.h"
#include <apps/code/script_template.h>
#include <apps/code/script.h>
#include <poincare_layouts.h>
#include <poincare_nodes.h>
#include <apps/shared/poincare_helpers.h>
#include <poincare/init.h>
#include <poincare/exception_checkpoint.h>
#include "../system/users.h"
#include "../firmware.h"
#include "../vfs/vfs.h"
#include "../palette.h"
#include "../daemons/daemon.h"
#include "../daemons/led_update_daemon.h"
#include "../daemons/powercheck_daemon.h"
#include "../daemons/sleep_daemon.h"
#include "../lecurses.h"

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
        Terminal::Screen::write(Ion::patchLevel());
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
    Terminal::Screen::writeLn(Terminal::Users::currentUser()->name());
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
        if (Terminal::Users::currentUser()->level() != ExecutionLevel::Root) {
            Terminal::Screen::writeLn("ion: access denied");
            return;
        }
        if (!Ion::USB::isPlugged()) {
            Terminal::Screen::writeLn("ion: device is not plugged (see ion -u for more info)");
            return;
        }
        Ion::USB::enable();
        LECurses::popup(Terminal::Strings::dfuMode(), false, false);
        Ion::USB::DFU();
        Terminal::Screen::clear();
        Ion::LED::updateColorWithPlugAndCharge();
        Ion::USB::disable();
    } else if (check(args->at(1), "-S")) {
        if (Terminal::Users::currentUser()->level() < ExecutionLevel::High) {
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
    bool longListing = args->count() > 1 && args->at(1)->matches("-l"); 

    auto node = Terminal::VFS::current();
    if (node == nullptr) {
        Terminal::Screen::writeLn("ls: current VFS node is nullptr");
        return;
    } else if (node->type() != Terminal::VFS::VFSNodeType::NodeContainer) {
        Terminal::Screen::writeLn("ls: current VFS node is not a container");
        return;
    }

    for (int i = 0; i < node->childCount(); i++) {
        if (node->provideChild(i) == nullptr) continue;
        if (longListing) {
            Terminal::Screen::write("  | ");
            Terminal::Screen::write(node->provideChild(i)->isExecutable() ? "x " : "- ");
            Terminal::Screen::write(node->provideChild(i)->name());
            Terminal::Screen::newLine();
        } else {
            KDColor color = node->provideChild(i)->type() == Terminal::VFS::VFSNodeType::NodeContainer
            ? TerminalBlue : 
            (node->isExecutable() ? TerminalYellow : KDColorWhite);

            Terminal::Screen::write(node->provideChild(i)->name(), color);
            Terminal::Screen::write(" ");
        }
    }
    Terminal::Screen::newLine();
}

void command_rm(SecuredStringList* args) {
    if (args->count() == 1) {
        Terminal::Screen::writeLn("rm: no filename specified");
        return;
    }
    if (Terminal::Users::currentUser()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("rm: access denied");
        return;
    }
    
    auto node = Terminal::VFS::current();
    if (node == nullptr) {
        Terminal::Screen::writeLn("rm: current VFS node is nullptr");
        return;
    } else if (node->type() != Terminal::VFS::VFSNodeType::NodeContainer) {
        Terminal::Screen::writeLn("rm: current VFS node is not a container");
        return;
    }

    bool result = node->remove(args->at(1)->c_str());
    if (!result) Terminal::Screen::writeLn("rm: failed to delete the file");
}

void command_touch(SecuredStringList* args) {
    if (args->count() == 1) {
        Terminal::Screen::writeLn("touch: no filename specified");
        return;
    }
    if (Terminal::Users::currentUser()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("touch: access denied");
        return;
    }
    
    auto node = Terminal::VFS::current();
    if (node == nullptr) {
        Terminal::Screen::writeLn("touch: current VFS node is nullptr");
        return;
    } else if (node->type() != Terminal::VFS::VFSNodeType::NodeContainer) {
        Terminal::Screen::writeLn("touch: current VFS node is not a container");
        return;
    }

    char data[1];
    data[0] = 0;
    bool result = node->write(new Terminal::VFS::VFSNode(args->at(1)->c_str(), (void*)data, 1));
    if (!result) {
        Terminal::Screen::writeLn("touch: failed to create the file");
    }
}

void command_cp(SecuredStringList* args) {
    if (args->count() != 3) {
        Terminal::Screen::writeLn("usage : cp <from> <to>");
        return;
    }
    if (Terminal::Users::currentUser()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("cp: access denied");
        return;
    }

    auto from = args->at(1);
    auto to = args->at(2);
    
    auto node = Terminal::VFS::current();
    if (node == nullptr) {
        Terminal::Screen::writeLn("cp: current VFS node is nullptr");
        return;
    } else if (node->type() != Terminal::VFS::VFSNodeType::NodeContainer) {
        Terminal::Screen::writeLn("cp: current VFS node is not a container");
        return;
    }

    auto originNode = Terminal::VFS::fetch(from);
    if (originNode->type() != Terminal::VFS::VFSNodeType::Data) {
        Terminal::Screen::writeLn("cp: input node is not a file");
        return;
    }
    auto targetNode = new Terminal::VFS::VFSNode(to->c_str(), originNode->data(), originNode->dataLength());

    bool result = node->write(targetNode);
    if (!result) {
        Terminal::Screen::writeLn("cp: failed to create the file");
    }
}

bool writeTemplate(Terminal::VFS::VFSNode* node, const Code::ScriptTemplate* script) {
    size_t valueSize = Code::Script::StatusSize() + strlen(script->content()) + 1; // (auto importation status + content fetched status) + scriptcontent size + null-terminating char
    bool success = node->write(new Terminal::VFS::VFSNode(script->name(), script->value(), valueSize));
    return success;
}

// pyscr command
// Creates the default python scripts found in epsilon
void command_pyscr(SecuredStringList* args) {
    if (Terminal::Users::currentUser()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("pyscr: access denied");
        return;
    }
    
    auto node = Terminal::VFS::current();
    if (node == nullptr) {
        Terminal::Screen::writeLn("pyscr: current VFS node is nullptr");
        return;
    } else if (node->type() != Terminal::VFS::VFSNodeType::NodeContainer) {
        Terminal::Screen::writeLn("pyscr: current VFS node is not a container");
        return;
    }

    writeTemplate(node, Code::ScriptTemplate::Squares());
    writeTemplate(node, Code::ScriptTemplate::Parabola());
    writeTemplate(node, Code::ScriptTemplate::Mandelbrot());
    writeTemplate(node, Code::ScriptTemplate::Polynomial());
}

void command_cat(SecuredStringList* args) {
    if (Terminal::Users::currentUser()->level() < ExecutionLevel::Normal) {
        Terminal::Screen::writeLn("cat: access denied");
        return;
    }
    if (args->count() != 2) {
        Terminal::Screen::writeLn("usage : cat <file>");
        return;
    }
    
    auto node = Terminal::VFS::current();
    if (node == nullptr) {
        Terminal::Screen::writeLn("cat: current VFS node is nullptr");
        return;
    } else if (node->type() != Terminal::VFS::VFSNodeType::NodeContainer) {
        Terminal::Screen::writeLn("cat: current VFS node is not a container");
        return;
    }

    auto fileNode = Terminal::VFS::fetch(args->at(1));
    if (fileNode == nullptr) {
        Terminal::Screen::writeLn("cat: no such file or directory");
        return;
    } else if (fileNode->type() != Terminal::VFS::VFSNodeType::Data) {
        Terminal::Screen::writeLn("cat: input node is not a file");
        return;
    }

    Terminal::Screen::writeLn((char*)fileNode->data(), KDColorWhite, fileNode->dataLength());
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
    while (Terminal::Screen::safeLoop()) {
        Terminal::Screen::write(">>> ");
        int c = Terminal::Screen::readLn(buffer);
        if (c < 0) break;
        if (check(buffer, "exit")) break;
        Poincare::ExceptionCheckpoint ecp;
        if (ExceptionRun(ecp)) {
            Poincare::Expression simplified = Poincare::Expression();
            Poincare::Expression approximate = Poincare::Expression();
            Shared::PoincareHelpers::ParseAndSimplifyAndApproximate(buffer, &simplified, &approximate, nullptr);
            Poincare::Layout layout = approximate.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7);
            SecuredString* approxStr = new SecuredString(layout.serializeParsedExpression(calculationBuffer, 256, nullptr), calculationBuffer);
            Poincare::Layout simLayout = simplified.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7);
            SecuredString* simplStr = new SecuredString(simLayout.serializeParsedExpression(secCalculationBuffer, 256, nullptr), secCalculationBuffer);
            Terminal::Screen::write("  | ");
            Terminal::Screen::write(simplStr, TerminalBeige);
            Terminal::Screen::write(" ");
            Terminal::Screen::writeLn(approxStr, TerminalIndigo);
        } else {
            Terminal::Screen::write("  | ");
            Terminal::Screen::writeLn("Error", TerminalRed);
        }
    }
}

void command_su(SecuredStringList* args) {
    if (args->count() == 1) {
        // Login into root
        bool result = Terminal::Users::switchUser(Terminal::Users::getUserUidFromUsername(SecuredString::fromBufferUnsafe("root")));
        if (!result) {
            Terminal::Screen::writeLn("su: root: unable to login");
            return;
        }
        return;
    }

    SecuredString* name = args->at(1);
    bool result = Terminal::Users::switchUser(Terminal::Users::getUserUidFromUsername(name));
    if (!result) {
        Terminal::Screen::write("su: ");
        Terminal::Screen::write(name);
        Terminal::Screen::writeLn(": unable to login");
        return;
    }
}

void command_useradd(SecuredStringList* args) {
    if (Terminal::Users::currentUser()->level() != ExecutionLevel::Root) {
        Terminal::Screen::writeLn("useradd: access denied");
        return;
    }
    if (args->count() != 3) {
        Terminal::Screen::writeLn("usage : useradd <username> <level>");
        return;
    }
    SecuredString* username = args->at(1);
    SecuredString* levelname = args->at(2);
    ExecutionLevel level = ExecutionLevel::Normal;

    if (*levelname == *SecuredString::fromBufferUnsafe("low")) {
        level = ExecutionLevel::Low;
    } else if (*levelname == *SecuredString::fromBufferUnsafe("normal")) {
        level = ExecutionLevel::Normal;
    } else if (*levelname == *SecuredString::fromBufferUnsafe("high")) {
        level = ExecutionLevel::High;
    } else if (*levelname == *SecuredString::fromBufferUnsafe("root")) {
        level = ExecutionLevel::Root;
    }

    bool result = Terminal::Users::add(new User(username, level));
    if (!result) {
        Terminal::Screen::writeLn("useradd: unable to add user");
    } else {
        Terminal::Screen::write("+ ", KDColorGreen);
        Terminal::Screen::writeLn(username);
    }
}

void command_id(SecuredStringList* args) {
    User* usr = Terminal::Users::currentUser();

    static SecuredString* levelStr;
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
    Terminal::Screen::write(levelStr);
    Terminal::Screen::write(")");
    Terminal::Screen::newLine();
}

void command_users(SecuredStringList* args) {
    bool detailed = args->count() > 1 && *args->at(1) == *SecuredString::fromBufferUnsafe("-d");
    for (useruid_t i = 0; i < Terminal::Users::count(); i++) {
        User* usr = Terminal::Users::at(i);
        if (detailed) {
            SecuredString* levelStr;
            if (usr->level() == ExecutionLevel::Low) levelStr = SecuredString::fromBufferUnsafe("low");
            else if (usr->level() == ExecutionLevel::Normal) levelStr = SecuredString::fromBufferUnsafe("normal");
            else if (usr->level() == ExecutionLevel::High) levelStr = SecuredString::fromBufferUnsafe("high");
            else if (usr->level() == ExecutionLevel::Root) levelStr = SecuredString::fromBufferUnsafe("root");
            else levelStr = SecuredString::fromBufferUnsafe("unknown");
            Terminal::Screen::write(" |  uid=");
            Terminal::Screen::writeChar(uintToString(usr->uid()));
            Terminal::Screen::write(" (");
            Terminal::Screen::write(usr->name());
            Terminal::Screen::write(")");
            Terminal::Screen::write(" level=");
            Terminal::Screen::writeChar(intToString((int)usr->level()));
            Terminal::Screen::write("(");
            Terminal::Screen::write(levelStr);
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
    Terminal::Screen::clear();
    int y = Terminal::Screen::posY;

    Terminal::Screen::writeBitmap(neofetch_logo, NEOFETCH_LOGO_WIDTH, NEOFETCH_LOGO_HEIGHT, FIRMWARE_MAIN_COLOR);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write(Terminal::Users::currentUser()->name(), FIRMWARE_MAIN_COLOR);
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
    Terminal::Screen::write("HAL: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::write("Ion ", KDColorWhite);
    Terminal::Screen::writeLn(Ion::softwareVersion(), KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("Terminal: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::writeLn("L.E. Terminal ", KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("Serial N.: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::writeLn(Ion::serialNumber(), KDColorWhite);
    Terminal::Screen::posX = NEOFETCH_LOGO_WIDTH;
    Terminal::Screen::write("Patch Level: ", FIRMWARE_MAIN_COLOR);
    Terminal::Screen::writeLn(Ion::patchLevel(), KDColorWhite);
    Terminal::Screen::posX = 0;
    Terminal::Screen::posY = y + NEOFETCH_LOGO_HEIGHT;
    Terminal::Screen::newLine();
}

void command_cd(SecuredStringList* args) {
    if (args->count() == 1) {
        Terminal::Screen::writeLn("usage: cd <name>");
        return;
    }
    if (check(args->at(1), "..")) {
        Terminal::VFS::goBackwards();
        return;
    }
    bool success = Terminal::VFS::warp(args->at(1)->c_str());
    if (!success) {
        Terminal::Screen::writeLn("cd: unable to warp to the specified destination");
    }
}

void command_mkdir(SecuredStringList* args) {
    if (args->count() == 1) {
        Terminal::Screen::writeLn("usage: mkdir <name>");
        return;
    }
    Terminal::VFS::mount(new Terminal::VFS::VFSNode(args->at(1)->c_str()));
}

// Usage : 
// daemon
// -> Displays all running daemons
// -------------------------------
// daemon kill <daemon-name>
// -> Kill the daemon named <daemon-name>
// -------------------------------
// daemon summon <daemon-name>
// -> Summon a daemon
// -------------------------------
void command_daemon(SecuredStringList* args) {
    Terminal::Background::Hell* hell = Terminal::Background::Hell::shared();

    if (args->count() == 1) {
        Terminal::Screen::writeLn("List of running daemons :");
        for (int i = 0; i < hell->runningCount(); i++) {
            if (hell->at(i) == nullptr) continue;
            Terminal::Screen::write("  | ");
            Terminal::Screen::write(hell->at(i)->name());
            Terminal::Screen::newLine();
        }
    } else if (args->at(1)->matches("kill")) {
        SecuredString* daemonName = args->at(2);
        bool success = hell->kill(daemonName);
        if (!success) Terminal::Screen::writeLn("daemon: unable to kill this daemon");
    } else if (args->at(1)->matches("summon")) {
        SecuredString* daemonName = args->at(2);

        // TODO : Please change this piece of garbage by a sweet auto-fetching system
        Terminal::Background::Daemon* daemon;
        if (daemonName->matches("led-update")) {
            daemon = new Terminal::Background::LEDUpdateDaemon();
        } else if (daemonName->matches("power-check")) {
            daemon = new Terminal::Background::PowerCheckDaemon();
        } else if (daemonName->matches("auto-sleep")) {
            daemon = new Terminal::Background::SleepDaemon();
        }

        bool success = hell->summon(daemon);
        if (!success) Terminal::Screen::writeLn("daemon: unable to summon this daemon");
    } else {
        Terminal::Screen::write("daemon: unknown option: ");
        Terminal::Screen::write(args->at(1));
        Terminal::Screen::newLine();
    }
}

void command_chmod(SecuredStringList* args) {
    if (args->count() != 3) {
        Terminal::Screen::writeLn("usage: chmod (flags) <file>");
        return;
    }

    auto file = args->at(2);
    if (args->at(1)->matches("+x")) {
        auto fileNode = Terminal::VFS::fetch(file);
        if (fileNode == nullptr) return;
        fileNode->setExecutable(true);
    } else if (args->at(1)->matches("-x")) {
        auto fileNode = Terminal::VFS::fetch(file);
        if (fileNode == nullptr) return;
        fileNode->setExecutable(false);
    } else {
        Terminal::Screen::write("chmod: unknown flag: ");
        Terminal::Screen::write(args->at(1));
        Terminal::Screen::newLine();
    }
}

void command_popup(SecuredStringList* args) {
    if (args->count() < 2) {
        Terminal::Screen::writeLn("usage: popup <text> [-ok]");
        return;
    }

    LECurses::popup(args->at(1), args->count() > 2 && args->at(2)->matches("-ok"));
}

#endif