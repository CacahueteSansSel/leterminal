#ifndef TERMINAL_LECURSES
#define TERMINAL_LECURSES
#include <kandinsky.h>
#include "termscreen.h"

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
            setColorsAt(x, rect.y() + rect.height(), color, foreColor);
        }
    }
}

static void topBar(KDColor back, KDColor fore, const char* text) {
    filledRectangle(KDRect(0, 0, Terminal::Screen::CharWidth, 1), back, fore);
    writeStringAt(2, 0, text, fore, back);
}

}

#endif