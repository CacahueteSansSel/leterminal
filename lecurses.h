#ifndef TERMINAL_LECURSES
#define TERMINAL_LECURSES
#include <kandinsky.h>
#include "termscreen.h"
#include "strings.h"
#include "terminal.h"

using namespace Terminal::Screen;

namespace LECurses {

static void rectangle(KDRect rect, KDColor color, KDColor foreColor = KDColorBlack) {
    for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
        setColorsAt(x, 0, color, foreColor);
    }
    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        setColorsAt(0, y, color, foreColor);
    }
    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        setColorsAt(rect.x() + rect.width(), y, color, foreColor);
    }
    for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
        setColorsAt(x, rect.y() + rect.height(), color, foreColor);
    }
}

static void filledRectangle(KDRect rect, KDColor color, KDColor foreColor = KDColorBlack) {
    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
            setColorsAt(x, y, color, foreColor);
        }
    }
}

static void topBar(KDColor back, KDColor fore, const char* text) {
    filledRectangle(KDRect(0, 0, Terminal::Screen::CharWidth, 1), back, fore);
    writeStringAt(2, 0, text, fore, back);
}

static void popup(SecuredString* message, bool okButton = true, bool wait = true) {
    KDColor backgroundBack = PopupBackground;
    KDColor backgroundText = KDColorWhite;
    KDColor popupBack = KDColorWhite;
    KDColor popupText = KDColorBlack;
    KDColor popupShadow = KDColorBlack;
    KDColor buttonBack = PopupButtonBackground;
    KDColor buttonText = KDColorWhite;
    SecuredString* okStr = Terminal::Strings::ok();
    int okStrWidth = okStr->size() + 4;

    int width = CharWidth - 2;
    int height = CharHeight - 2;
    int textWidth = message->size() + 2; // +2 is the padding
    if (textWidth >= width) textWidth = width-1;
    int textHeight = message->size() / textWidth;
    if (textHeight < 3) textHeight = 3; // we lock at 3 character height for padding
    int popupX = (width / 2 - textWidth / 2);
    int popupY = (height / 2 - textHeight / 2);
    int textX = popupX + 1; // as the padding is 2, we add 1 to center it
    int textY = popupY + 1;
    int okButtonX = popupX + (textWidth / 2 - okStrWidth / 2);
    int okButtonY = popupY + textHeight - 2 + 1;

    clear();
    filledRectangle(KDRect(0, 0, CharWidth, CharHeight), backgroundBack);
    filledRectangle(KDRect(popupX+1, popupY + (okButton ? 0 : 1), textWidth, textHeight+(okButton ? 2 : 0)), popupShadow);
    filledRectangle(KDRect(popupX, popupY-(okButton ? 1 : 0), textWidth, textHeight+(okButton ? 2 : 0)), popupBack);
    if (okButton) filledRectangle(KDRect(okButtonX, okButtonY, okStrWidth, 1), buttonBack);

    posX = 0;
    posY = 0;

    writeB(Terminal::Strings::leterminal_v(), backgroundText, backgroundBack);
    writeB(TERMINAL_VERSION, backgroundText, backgroundBack);

    posX = textX;
    posY = textY-(okButton ? 1 : 0);

    writeB(message, popupText, popupBack);

    if (okButton) {
        posX = okButtonX + 2;
        posY = okButtonY;

        writeB(okStr, buttonText, buttonBack);
    }

    posX = 0;
    posY = height;

    curPeriod = true; //This line forces the cursor to be hidden
    redraw();

    if (!wait) return;
    while (safeLoop()) {
        keyRead();

        if (keyPressed(Ion::Keyboard::Key::OK) || keyPressed(Ion::Keyboard::Key::EXE)) {
            keyEnd();
            break;
        }

        keyEnd();
        Terminal::Background::Hell::shared()->dispatchUpdate();
    }

    clear();
}

}

#endif