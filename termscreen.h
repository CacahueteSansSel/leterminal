#ifndef SIGMA_TERMINAL_SCREEN
#define SIGMA_TERMINAL_SCREEN
#include <kandinsky.h>
#include <string.h>
#include <ion.h>
#include "list.h"
#include "stringx.h"
#include "font.h"
#include "system/power.h"

namespace Terminal {

static bool isLaunchedFromFirmware = false;

class ReadLineSettings {
    private:
    bool m_hideText;
    bool m_historyEnabled;
    public:
    ReadLineSettings(bool hideText, bool historyEnabled) : m_hideText(hideText), m_historyEnabled(historyEnabled)
    {

    }

    bool doHideText() const {return m_hideText;}
    bool isHistoryEnabled() const {return m_historyEnabled;}

    static ReadLineSettings* defaultSettings() {
        return new ReadLineSettings(false, true);
    }

    static ReadLineSettings* password() {
        return new ReadLineSettings(true, false);
    }
};

namespace Keyboard {

static uint8_t keyboardMode;

static bool isAlpha() { return keyboardMode == 1; }
static bool isAlphaCaps() { return keyboardMode == 3; }
static bool isShift() { return keyboardMode == 2; }
static bool isReleased() { return keyboardMode == 0; }

static void setAlpha() { keyboardMode = 1; }
static void setAlphaCaps() { keyboardMode = 3; }
static void setShift() { keyboardMode = 2; }
static void setReleased() { keyboardMode = 0; }

static void toggleAlpha() {
    if (isAlphaCaps()) setReleased();
    else if (isAlpha()) setAlphaCaps();
    else {
        if (isShift()) setAlphaCaps();
        else setAlpha();
    }
}

static void toggleShift() {
    if (isShift()) setReleased();
    else setShift();
}

}
    
namespace Screen {

const int CharWidth = 40;
const int CharHeight = 20;

// Terminal cursor position
static int posX = 0;
static int posY = 0;
// Terminal input history
static StringPositionalList* history = 0;
static ReadLineSettings* readLnSettings;

static uint8_t lastKey = 0;

struct terminal_cell_t {
    uint16_t back;
    uint16_t fore;
    char character[1];
};

static bool operator==(terminal_cell_t left, terminal_cell_t right) {
    return left.character == right.character && left.back == right.back && left.fore == right.fore;
}

static terminal_cell_t Screen[CharWidth * CharHeight];
static terminal_cell_t screenInternal[CharWidth * CharHeight];

#define LMAP(key, nonshift, shift, alpha, alphacaps) if (scan.keyDown(key) && lastKey != (uint8_t)key) { \
if (Terminal::Keyboard::isShift()) { buffer[ptr] = shift; writeChar(settings->doHideText() ? '*' : shift); ptr++; redraw(); } \
else if (Terminal::Keyboard::isAlphaCaps()) { buffer[ptr] = alphacaps; writeChar(settings->doHideText() ? '*' : alphacaps); ptr++; redraw(); } \
else if (Terminal::Keyboard::isAlpha()) { buffer[ptr] = alpha; writeChar(settings->doHideText() ? '*' : alpha); ptr++; redraw(); } \
else { buffer[ptr] = nonshift; writeChar(settings->doHideText() ? '*' : nonshift); ptr++; redraw(); } \
lastKey = (uint8_t)key; \
} else if (!scan.keyDown(key) && lastKey == (uint8_t)key) { lastKey = 0; } \

#define LMAPSTR(key, nonshift, shift, alpha, alphacaps) if (scan.keyDown(key) && lastKey != (uint8_t)key) { \
if (Terminal::Keyboard::isShift()) { memcpy(buffer + ptr, shift, strlen(shift)); write(settings->doHideText() ? "*" : shift); ptr += strlen(shift); redraw(); } \
else if (Terminal::Keyboard::isAlphaCaps()) { memcpy(buffer + ptr, alphacaps, strlen(alphacaps)); write(settings->doHideText() ? "*" : alphacaps); ptr += strlen(alphacaps); redraw(); } \
else if (Terminal::Keyboard::isAlpha()) { memcpy(buffer + ptr, alpha, strlen(alpha)); write(settings->doHideText() ? "*" : alpha); ptr += strlen(alpha); redraw(); } \
else { memcpy(buffer + ptr, nonshift, strlen(nonshift)); write(settings->doHideText() ? "*" : nonshift); ptr += strlen(nonshift); redraw(); } \
lastKey = (uint8_t)key; \
} else if (!scan.keyDown(key) && lastKey == (uint8_t)key) { lastKey = 0; } \

static KDPoint screenCursorPosition() {
    return KDPoint(posX * 8, posY * 12);
}

static void drawChar(KDContext* ctx, char c, KDColor back, KDColor color, KDPoint pos) {
    // Just to be sure ;)
    if (c == '\0' || c == 0) {
        return;
    }
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 8; x++) {
            uint8_t alpha = Font::generatedGlyphData[(uint8_t)(c) * 96 + (y * 8 + x)];
            if (alpha != 0xFF) continue;
            
            ctx->setPixel(pos.translatedBy(KDPoint(x, y)), color);
        }
    }
}

static void redraw(bool complete = false) {
    KDContext* ctx = KDIonContext::sharedContext();

    for (int y = 0; y < CharHeight; y++)
    {
        for (int x = 0; x < CharWidth; x++) {
            terminal_cell_t cell = Screen[y * CharWidth + x];
            if (cell == screenInternal[y * CharWidth + x] && !complete) continue;
            screenInternal[y * CharWidth + x] = cell;
            KDRect screenRect = KDRect(x * 8, y * 12, 8, 12);
            ctx->fillRect(screenRect, KDColor(cell.back));
            //ctx->drawString(cell.character, screenRect.topLeft(), KDFont::SmallFont, KDColor(cell.fore), KDColor(cell.back), 1);
            drawChar(ctx, cell.character[0], KDColor(cell.back), KDColor(cell.fore), screenRect.topLeft());
        }
    }
}

static void setColorsAt(int x, int y, KDColor back, KDColor fore) {
    Screen[y * CharWidth + x].back = back;
    Screen[y * CharWidth + x].fore = fore;
}

static void copy(KDRect from, KDPoint to) {
    for (int y = from.y(); y < from.y() + from.height(); y++) {
        for (int x = from.x(); x < from.x() + from.width(); x++) {
            int tx = x - from.x() + to.x();
            int ty = y - from.y() + to.y();

            if (tx < 0 || ty < 0 || tx >= CharWidth || ty >= CharHeight) continue;
            Screen[ty * CharWidth + tx] = Screen[y * CharWidth + x];
        }
    }
}

static void clearRect(KDRect rect) {
    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
            setColorsAt(x, y, KDColorBlack, KDColorWhite);
            Screen[y * CharWidth + x].character[0] = ' ';
        }
    }
}

static void clear(int count) {
    for (int i = 0; i < count; i++) {
        if (posY * CharWidth + posX + i >= CharWidth*CharHeight) continue;
        Screen[posY * CharWidth + posX + i].character[0] = ' ';
        Screen[posY * CharWidth + posX + i].fore = KDColorWhite;
        Screen[posY * CharWidth + posX + i].back = KDColorBlack;
    }
}

static KDColor getBackColorAt(int x, int y) {
    return Screen[y * CharWidth + x].back;
}

static void scrollDown() {
    copy(KDRect(0, 0, CharWidth, CharHeight), KDPoint(0, -1));
    clearRect(KDRect(0, CharHeight-1, CharWidth, 1));
}

static void newLine() {
    // Ensure that the last position is cleared out to avoid cursor ghosting
    // A bit hacky fix here
    if (getBackColorAt(posX, posY) == KDColorWhite) setColorsAt(posX, posY, KDColorBlack, KDColorWhite);
    posX--;
    posY++;
    posX = 0;

    if (posY >= CharHeight) {
        // Scroll the screen up to have more space, and set the cursor's y position to the last line
        // Warning : cells that are not shown on screen after the scroll were overwritten and lost

        scrollDown();
        posY = CharHeight-1;
    }
}

static void incrementPos() {
    // Ensure that the last position is cleared out to avoid cursor ghosting
    // A bit hacky fix here
    if (getBackColorAt(posX, posY) == KDColorWhite) setColorsAt(posX, posY, KDColorBlack, KDColorWhite);
    posX++;
    if (posX >= CharWidth) newLine();
}

static void decrementPos() {
    // Ensure that the last position is cleared out to avoid cursor ghosting
    // A bit hacky fix here
    if (getBackColorAt(posX, posY) == KDColorWhite) setColorsAt(posX, posY, KDColorBlack, KDColorWhite);
    posX--;
    if (posX < 0) {
        posY--;
        posX = CharWidth-1;
    }
}

static void writeAt(int x, int y, char text) {
    Screen[y * CharWidth + x].character[0] = text;
}

static void write(const char* text, KDColor color = KDColorWhite, int count = -1) {
    if (count == -1) count = strlen(text);
    for (int i = 0; i < count; i++) {
        if (text[i] == '\n') newLine();
        else Screen[posY * CharWidth + posX].character[0] = text[i];
        Screen[posY * CharWidth + posX].fore = color;
        incrementPos();
    }

    redraw();
}

// TODO : much better implementation of the SecuredString
static void write(SecuredString text, KDColor color = KDColorWhite) {
    write(text.c_str(), color, text.size());
}

static void writeB(const char* text, KDColor color = KDColorWhite, KDColor back = KDColorBlack, int count = -1) {
    if (count == -1) count = strlen(text);
    for (int i = 0; i < count; i++) {
        if (text[i] == '\n') newLine();
        else Screen[posY * CharWidth + posX].character[0] = text[i];
        Screen[posY * CharWidth + posX].fore = color;
        Screen[posY * CharWidth + posX].back = back;
        incrementPos();
    }

    redraw();
}

// TODO : much better implementation of the SecuredString
static void writeB(SecuredString text, KDColor color = KDColorWhite, KDColor back = KDColorBlack) {
    writeB(text.c_str(), color, back, text.size());
}

static void writeStringAt(int x, int y, SecuredString text, KDColor fore = KDColorWhite, KDColor back = KDColorBlack) {
    int lastX = posX;
    int lastY = posY;
    posX = x;
    posY = y;

    writeB(text.c_str(), fore, back, text.size());
}

static void writeStringAt(int x, int y, const char* text, KDColor fore = KDColorWhite, KDColor back = KDColorBlack) {
    int lastX = posX;
    int lastY = posY;
    posX = x;
    posY = y;

    writeB(text, fore, back);
}

static void writeChar(char text, KDColor color = KDColorWhite, int count = -1) {
    if (text == '\n') newLine();
    else Screen[posY * CharWidth + posX].character[0] = text;
    Screen[posY * CharWidth + posX].fore = color;
    incrementPos();
    redraw();
}

static void writeCharWithoutRedrawing(char text, KDColor color = KDColorWhite, int count = -1) {
    if (text == '\n') newLine();
    else Screen[posY * CharWidth + posX].character[0] = text;
    Screen[posY * CharWidth + posX].fore = color;
    incrementPos();
}

static void writeCharSilently(char text, KDColor color = KDColorWhite, int count = -1) {
    Screen[posY * CharWidth + posX].character[0] = text;
    Screen[posY * CharWidth + posX].fore = color;
    redraw();
}

static void writeCharSilentlyWithoutRedrawing(char text, KDColor color = KDColorWhite, int count = -1) {
    Screen[posY * CharWidth + posX].character[0] = text;
    Screen[posY * CharWidth + posX].fore = color;
}

static void writeLn(const char* text, KDColor color = KDColorWhite, int count = -1) {
    write(text, color, count);
    newLine();
}

// TODO : much better implementation of the SecuredString
static void writeLn(SecuredString text, KDColor color = KDColorWhite) {
    write(text.c_str(), color, text.size());
    newLine();
}

static void writeLnB(const char* text, KDColor color = KDColorWhite, KDColor back = KDColorBlack, int count = -1) {
    writeB(text, color, back, count);
    newLine();
}

// TODO : much better implementation of the SecuredString
static void writeLnB(SecuredString text, KDColor color, KDColor back) {
    writeB(text.c_str(), color, back, text.size());
    newLine();
}

static void updateTextStatus() {
    int bufX = posX;
    int bufY = posY;
    if (Terminal::Keyboard::isAlpha()) {
        posX = CharWidth - 8;
        posY = 0;
        writeB("-> alpha", KDColorBlack, KDColorWhite);
    } else if (Terminal::Keyboard::isAlphaCaps()) {
        posX = CharWidth - 8;
        posY = 0;
        writeB("-> ALPHA", KDColorBlack, KDColorWhite);
    } else if (Terminal::Keyboard::isShift()) {
        posX = CharWidth - 8;
        posY = 0;
        writeB("-> shift", KDColorBlack, KDColorWhite);
    } else {
        posX = CharWidth - 8;
        posY = 0;
        writeB("->      ", KDColorBlack, KDColorBlack);
    }
    posX = bufX;
    posY = bufY;
    redraw();
}

static void clear() {
    for (int y = 0; y < CharHeight; y++)
    {
        for (int x = 0; x < CharWidth; x++) {
            Screen[y * CharWidth + x].character[0] = ' ';
            Screen[y * CharWidth + x].back = KDColorBlack;
            Screen[y * CharWidth + x].fore = KDColorWhite;
        }
    }
    posY = 0;
    posX = 0;
    updateTextStatus();
}

static int readLn(char* buffer, int maxLength = 256, ReadLineSettings* settings = nullptr) {

    if (settings == nullptr) settings = ReadLineSettings::defaultSettings();
    if (history == 0) history = new StringPositionalList();
    int localHistoryPtr = 0;
    int ptr = 0;
    int cursor = 0; //0-50 : invisible - 50-100 : visible
    int originX = posX;
    int originY = posY;
    bool curPeriod = false; //Cursor's period : false=increasing, true=decreasing
    while (true) {
        auto scan = Ion::Keyboard::scan();
        if ((scan.keyDown(Ion::Keyboard::Key::OK) || scan.keyDown(Ion::Keyboard::Key::EXE))
        && (lastKey != (uint8_t)Ion::Keyboard::Key::OK)) {
            buffer[ptr] = '\0';
            lastKey = (uint8_t)Ion::Keyboard::Key::OK;
            history->copy(buffer, ptr);
            break;
        } else if (!scan.keyDown(Ion::Keyboard::Key::OK) && !scan.keyDown(Ion::Keyboard::Key::EXE) && (lastKey == (uint8_t)Ion::Keyboard::Key::OK)) {
            lastKey = 0;
        }

        if (scan.keyDown(Ion::Keyboard::Key::Alpha) && lastKey != (uint8_t)Ion::Keyboard::Key::Alpha) {
            Terminal::Keyboard::toggleAlpha();
            updateTextStatus();
            lastKey = (uint8_t)Ion::Keyboard::Key::Alpha;
        }
        else if (!scan.keyDown(Ion::Keyboard::Key::Alpha) && lastKey == (uint8_t)Ion::Keyboard::Key::Alpha) { 
            lastKey = 0;
        }
        
        if (scan.keyDown(Ion::Keyboard::Key::Shift) && lastKey != (uint8_t)Ion::Keyboard::Key::Shift) {
            Terminal::Keyboard::toggleShift();
            updateTextStatus();
            lastKey = (uint8_t)Ion::Keyboard::Key::Shift;
        } 
        else if (!scan.keyDown(Ion::Keyboard::Key::Shift) && lastKey == (uint8_t)Ion::Keyboard::Key::Shift) { 
            lastKey = 0;
        }

        if (scan.keyDown(Ion::Keyboard::Key::Down) && lastKey != (uint8_t)Ion::Keyboard::Key::Down) {
            if (!history->canDecrement() || !settings->isHistoryEnabled()) continue;
            memset(buffer, '\0', 256);
            memcpy(buffer, history->selected().c_str(), history->selected().size());
            posX = originX;
            clear(256);
            write(buffer);
            ptr = history->selected().size();
            history->decPointer();
            lastKey = (uint8_t)Ion::Keyboard::Key::Down;

            redraw();
        }
        else if (!scan.keyDown(Ion::Keyboard::Key::Down) && lastKey == (uint8_t)Ion::Keyboard::Key::Down) { 
            lastKey = 0;
        }

        if (scan.keyDown(Ion::Keyboard::Key::Up) && lastKey != (uint8_t)Ion::Keyboard::Key::Up) {
            if (!history->canIncrement() || !settings->isHistoryEnabled()) continue;
            memset(buffer, '\0', 256);
            memcpy(buffer, history->selected().c_str(), history->selected().size());
            posX = originX;
            clear(256);
            write(buffer);
            ptr = history->selected().size();
            history->incPointer();
            lastKey = (uint8_t)Ion::Keyboard::Key::Up;

            redraw();
        }
        else if (!scan.keyDown(Ion::Keyboard::Key::Up) && lastKey == (uint8_t)Ion::Keyboard::Key::Up) { 
            lastKey = 0;
        }

        if (scan.keyDown(Ion::Keyboard::Key::Left) && lastKey != (uint8_t)Ion::Keyboard::Key::Left) {
            if (ptr <= 0) continue;
            ptr--;
            decrementPos();
            lastKey = (uint8_t)Ion::Keyboard::Key::Left;
        }
        else if (!scan.keyDown(Ion::Keyboard::Key::Left) && lastKey == (uint8_t)Ion::Keyboard::Key::Left) { 
            lastKey = 0;
        }

        if (scan.keyDown(Ion::Keyboard::Key::Right) && lastKey != (uint8_t)Ion::Keyboard::Key::Right) {
            if (ptr >= strlen(buffer)) continue;
            ptr++;
            incrementPos();
            lastKey = (uint8_t)Ion::Keyboard::Key::Right;
        }
        else if (!scan.keyDown(Ion::Keyboard::Key::Right) && lastKey == (uint8_t)Ion::Keyboard::Key::Right) { 
            lastKey = 0;
        }

        //Letters
        LMAP(Ion::Keyboard::Key::One, '1', '1', 'w', 'W');
        LMAP(Ion::Keyboard::Key::Two, '2', '2', 'x', 'X');
        LMAP(Ion::Keyboard::Key::Three, '3', '3', 'y', 'Y');
        LMAP(Ion::Keyboard::Key::Four, '4', '4', 'r', 'R');
        LMAP(Ion::Keyboard::Key::Five, '5', '5', 's', 'S');
        LMAP(Ion::Keyboard::Key::Six, '6', '6', 't', 'T');
        LMAP(Ion::Keyboard::Key::Seven, '7', '7', 'm', 'M');
        LMAP(Ion::Keyboard::Key::Eight, '8', '8', 'n', 'N');
        LMAP(Ion::Keyboard::Key::Nine, '9', '9', 'o', 'O');
        LMAP(Ion::Keyboard::Key::Zero, '0', '0', '?', '?');
        LMAP(Ion::Keyboard::Key::Dot, '.', '.', '!', '!');
        LMAP(Ion::Keyboard::Key::EE, 'e', 'e', 'e', 'e');
        LMAP(Ion::Keyboard::Key::Plus, '+', '+', 'z', 'Z');
        LMAP(Ion::Keyboard::Key::Minus, '-', '-', ' ', ' ');
        LMAP(Ion::Keyboard::Key::Multiplication, '*', '*', 'u', 'U');
        LMAP(Ion::Keyboard::Key::Division, '/', '/', 'v', 'V');
        LMAP(Ion::Keyboard::Key::LeftParenthesis, '(', '(', 'p', 'P');
        LMAP(Ion::Keyboard::Key::RightParenthesis, ')', ')', 'q', 'Q');
        LMAPSTR(Ion::Keyboard::Key::Sine, "sin(", "asin(", "g", "G");
        LMAPSTR(Ion::Keyboard::Key::Cosine, "cos(", "acos(", "h", "H");
        LMAPSTR(Ion::Keyboard::Key::Tangent, "tan(", "atan(", "i", "I");
        LMAP(Ion::Keyboard::Key::Pi, 'π', '=', 'j', 'J');
        LMAPSTR(Ion::Keyboard::Key::Sqrt, "sqrt(", "<", "k", "K");
        LMAPSTR(Ion::Keyboard::Key::Square, "^2", ">", "l", "L");
        LMAPSTR(Ion::Keyboard::Key::Exp, "e^", "[", "a", "A");
        LMAPSTR(Ion::Keyboard::Key::Ln, "ln(", "]", "b", "B");
        LMAPSTR(Ion::Keyboard::Key::Log, "log(", "{", "c", "C");
        LMAP(Ion::Keyboard::Key::Imaginary, 'i', '}', 'd', 'D');
        LMAP(Ion::Keyboard::Key::Comma, ',', '_', 'e', 'E');
        LMAP(Ion::Keyboard::Key::Power, '^', '>', 'f', 'F');
        LMAP(Ion::Keyboard::Key::Toolbox, 't', 't', '"', '"');
        LMAP(Ion::Keyboard::Key::Var, 'v', 'v', ';', ';');
        LMAP(Ion::Keyboard::Key::XNT, 'x', 'x', ':', ':');

        if (scan.keyDown(Ion::Keyboard::Key::Backspace) && lastKey != (uint8_t)Ion::Keyboard::Key::Backspace) {
            if (Terminal::Keyboard::isAlpha()) {
                buffer[ptr] = '%'; 
                writeChar('%'); 
                ptr++;
                redraw();
                lastKey = (uint8_t)Ion::Keyboard::Key::Backspace;
                continue;
            } else if (Terminal::Keyboard::isShift()) {
                // Backspace + Shift = Clear key
                Terminal::Screen::clear();
                redraw();
                continue;
            }
            if (ptr <= 0) continue;
            buffer[ptr] = '\0';
            ptr--;
            decrementPos();
            writeAt(posX, posY, ' ');

            lastKey = (uint8_t)Ion::Keyboard::Key::Backspace;
            redraw();
        } else if (!scan.keyDown(Ion::Keyboard::Key::Backspace) && lastKey == (uint8_t)Ion::Keyboard::Key::Backspace) {
            lastKey = 0;
        }
        
        if (scan.keyDown(Ion::Keyboard::Key::OnOff) && lastKey != (uint8_t)Ion::Keyboard::Key::OnOff) {
            terminalSleep();
            Terminal::Screen::redraw();
            lastKey = (uint8_t)Ion::Keyboard::Key::OnOff;
        } else if (!scan.keyDown(Ion::Keyboard::Key::OnOff) && lastKey == (uint8_t)Ion::Keyboard::Key::OnOff) {
            lastKey = 0;
        }

        if (Terminal::isLaunchedFromFirmware) {
            if (scan.keyDown(Ion::Keyboard::Key::Home) || scan.keyDown(Ion::Keyboard::Key::Back)) { // Break if HOME or back is hit
                return -1;
            }
        }

        // Update the cursor

        if (curPeriod) {
            // Decrease cursor
            cursor--;
            if (cursor < 0) {
                // Switching to increase
                curPeriod = false;
                setColorsAt(posX, posY, curPeriod ? KDColorWhite : KDColorBlack, curPeriod ? KDColorBlack : KDColorWhite);
                redraw();
            }
        } else {
            // Increase cursor
            cursor++;
            if (cursor > 100) {
                // Switching to decrease
                curPeriod = true;
                setColorsAt(posX, posY, curPeriod ? KDColorWhite : KDColorBlack, curPeriod ? KDColorBlack : KDColorWhite);
                redraw();
            }
        }

    }

    newLine();
    return ptr;
}

}

}

#endif