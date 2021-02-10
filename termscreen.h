#ifndef TERMINAL_SCREEN
#define TERMINAL_SCREEN
#include <kandinsky.h>
#include <string.h>
#include <ion.h>
#include "list.h"
#include "stringx.h"
#include "font.h"
#include "system/power.h"
#include "events.h"
#include "palette.h"
#include "daemons/daemon.h"

namespace Terminal {

static bool isLaunchedFromFirmware = false;

// Special characters

#define CHAR_USB 128
#define CHAR_BATTERY_FILLED 129
#define CHAR_BATTERY_MIDDLE 130
#define CHAR_BATTERY_LOW 131
#define CHAR_ECO 132

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

static VolatileUInt8List* lastKeys;
static Ion::Keyboard::State keyScan;
static bool curPeriod = false; //Cursor's period : false=increasing, true=decreasing

enum class TerminalCellBackground : uint8_t {
    Fill,
    Stroke,
    StrokeUp,
    StrokeDown,
    StrokeLeft,
    StrokeRight,
    StrokeCornerUL,
    StrokeCornerUR,
    StrokeCornerDL,
    StrokeCornerDR
};

struct terminal_cell_t {
    uint16_t back;
    uint16_t fore;
    TerminalCellBackground backStyle;
    char character[1];
};

static bool operator==(terminal_cell_t left, terminal_cell_t right) {
    return left.character == right.character && left.back == right.back && left.fore == right.fore;
}

static terminal_cell_t Screen[CharWidth * CharHeight];

#define LMAP(key, nonshift, shift, alpha, alphacaps) if (keyPressed(key)) { \
if (Terminal::Keyboard::isShift()) { buffer[ptr] = shift; writeChar(settings->doHideText() ? '*' : shift); ptr++; redraw(); } \
else if (Terminal::Keyboard::isAlphaCaps()) { buffer[ptr] = alphacaps; writeChar(settings->doHideText() ? '*' : alphacaps); ptr++; redraw(); } \
else if (Terminal::Keyboard::isAlpha()) { buffer[ptr] = alpha; writeChar(settings->doHideText() ? '*' : alpha); ptr++; redraw(); } \
else { buffer[ptr] = nonshift; writeChar(settings->doHideText() ? '*' : nonshift); ptr++; redraw(); } \
}

#define LMAPSTR(key, nonshift, shift, alpha, alphacaps) if (keyPressed(key)) { \
if (Terminal::Keyboard::isShift()) { memcpy(buffer + ptr, shift, strlen(shift)); write(settings->doHideText() ? "*" : shift); ptr += strlen(shift); redraw(); } \
else if (Terminal::Keyboard::isAlphaCaps()) { memcpy(buffer + ptr, alphacaps, strlen(alphacaps)); write(settings->doHideText() ? "*" : alphacaps); ptr += strlen(alphacaps); redraw(); } \
else if (Terminal::Keyboard::isAlpha()) { memcpy(buffer + ptr, alpha, strlen(alpha)); write(settings->doHideText() ? "*" : alpha); ptr += strlen(alpha); redraw(); } \
else { memcpy(buffer + ptr, nonshift, strlen(nonshift)); write(settings->doHideText() ? "*" : nonshift); ptr += strlen(nonshift); redraw(); } \
}

static KDPoint screenCursorPosition() {
    return KDPoint(posX * 8, posY * 12);
}

static void init() {
    lastKeys = new VolatileUInt8List();
}

static bool keyPressed(Ion::Keyboard::Key key) {
    bool state = keyScan.keyDown(key) && !lastKeys->any((uint8_t)key);
    if (state) lastKeys->append((uint8_t)key);

    return state;
}

static bool anyKeyPressed() {
    return lastKeys->aliveCount() > 0;
}

static void keyRead() {
    keyScan = Ion::Keyboard::scan();
}

static void keyEnd() {
    keyRead();
    for (int i = 0; i < lastKeys->count(); i++) {
        if (!keyScan.keyDown((Ion::Keyboard::Key)lastKeys->at(i))) lastKeys->clear(lastKeys->at(i));
    }
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

static void writeAt(int x, int y, char text) {
    Screen[y * CharWidth + x].character[0] = text;
}

static void redrawStatusBar() {
    // Draw the white bar
    for (int x = 0; x < CharWidth; x++) {
        Screen[(CharHeight-1) * CharWidth + x].back = KDColorWhite;
        Screen[(CharHeight-1) * CharWidth + x].fore = TerminalBackground;
    }

    // Battery icon
    auto powerLevel = Ion::Battery::level();
    if (powerLevel == Ion::Battery::Charge::FULL) {
        writeAt(1, CharHeight-1, CHAR_BATTERY_FILLED);
        Screen[(CharHeight-1) * CharWidth + 1].fore = TerminalBackground;
    } else if (powerLevel == Ion::Battery::Charge::SOMEWHERE_INBETWEEN) {
        writeAt(1, CharHeight-1, CHAR_BATTERY_MIDDLE);
        Screen[(CharHeight-1) * CharWidth + 1].fore = TerminalBackground;
    } else if (powerLevel <= Ion::Battery::Charge::LOW) {
        writeAt(1, CharHeight-1, CHAR_BATTERY_LOW);
        Screen[(CharHeight-1) * CharWidth + 1].fore = TerminalRed;
    }

    // Plugged icon
    if (Ion::USB::isPlugged()) {
        writeAt(2, CharHeight-1, CHAR_USB);
        Screen[(CharHeight-1) * CharWidth + 2].fore = TerminalBlue;
    } else {
        writeAt(2, CharHeight-1, ' ');
        Screen[(CharHeight-1) * CharWidth + 2].fore = TerminalBlue;
    }
}

static void redraw(bool complete = false) {
    KDContext* ctx = KDIonContext::sharedContext();

    redrawStatusBar();

    for (int y = 0; y < CharHeight; y++)
    {
        for (int x = 0; x < CharWidth; x++) {
            terminal_cell_t cell = Screen[y * CharWidth + x];
            KDRect screenRect = KDRect(x * 8, y * 12, 8, 12);
            switch (cell.backStyle) {
                case TerminalCellBackground::Fill:
                    ctx->fillRect(screenRect, KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::Stroke:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), screenRect.width(), 1), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x() + screenRect.width()-1, screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y() + screenRect.height()-1, screenRect.width(), 1), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeUp:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), screenRect.width(), 1), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeDown:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y() + screenRect.height()-1, screenRect.width(), 1), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeLeft:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeRight:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x() + screenRect.width()-1, screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeCornerUL:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), screenRect.width(), 1), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeCornerUR:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), screenRect.width(), 1), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x() + screenRect.width()-1, screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeCornerDL:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y() + screenRect.height()-1, screenRect.width(), 1), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
                case TerminalCellBackground::StrokeCornerDR:
                    ctx->fillRect(screenRect, TerminalBackground);
                    ctx->fillRect(KDRect(screenRect.x(), screenRect.y() + screenRect.height()-1, screenRect.width(), 1), KDColor::RGB16(cell.back));
                    ctx->fillRect(KDRect(screenRect.x() + screenRect.width()-1, screenRect.y(), 1, screenRect.height()), KDColor::RGB16(cell.back));
                    break;
            }
            drawChar(ctx, cell.character[0], KDColor::RGB16(cell.back), KDColor::RGB16(cell.fore), screenRect.topLeft());
        }
    }

    ctx->fillRect(KDRect(screenCursorPosition(), KDSize(8, 12)), curPeriod ? TerminalBackground : KDColorWhite);
}

static void setColorsAt(int x, int y, KDColor back, KDColor fore) {
    if (x < 0 || y < 0 || x >= CharWidth || y >= CharHeight) return;

    Screen[y * CharWidth + x].back = back;
    Screen[y * CharWidth + x].fore = fore;
}

static void setBackColorAt(int x, int y, KDColor back) {
    if (x < 0 || y < 0 || x >= CharWidth || y >= CharHeight) return;

    Screen[y * CharWidth + x].back = back;
}

static void setForeColorAt(int x, int y, KDColor fore) {
    if (x < 0 || y < 0 || x >= CharWidth || y >= CharHeight) return;

    Screen[y * CharWidth + x].fore = fore;
}

static void setCellStyle(int x, int y, TerminalCellBackground style) {
    if (x < 0 || y < 0 || x >= CharWidth || y >= CharHeight) return;

    Screen[y * CharWidth + x].backStyle = style;
}

static void setCellStyle(KDPoint pos, TerminalCellBackground style) {
    if (pos.x() < 0 || pos.y() < 0 || pos.x() >= CharWidth || pos.y() >= CharHeight) return;

    Screen[pos.y() * CharWidth + pos.x()].backStyle = style;
}

static void drawRect(KDRect rect, KDColor color) {
    for (int x = rect.x(); x < rect.right(); x++) {
        setCellStyle(x, rect.y(), TerminalCellBackground::StrokeUp);
        setCellStyle(x, rect.bottom(), TerminalCellBackground::StrokeDown);
        setBackColorAt(x, rect.y(), color);
        setBackColorAt(x, rect.bottom(), color);
    }
    for (int y = rect.y(); y < rect.bottom(); y++) {
        setCellStyle(rect.x(), y, TerminalCellBackground::StrokeLeft);
        setCellStyle(rect.right(), y, TerminalCellBackground::StrokeRight);
        setBackColorAt(rect.x(), y, color);
        setBackColorAt(rect.right(), y, color);
    }
    setCellStyle(rect.topLeft(), TerminalCellBackground::StrokeCornerUL);
    setBackColorAt(rect.x(), rect.y(), color);
    setCellStyle(rect.topRight(), TerminalCellBackground::StrokeCornerUR);
    setBackColorAt(rect.topRight().x(), rect.topRight().y(), color);
    setCellStyle(rect.bottomLeft(), TerminalCellBackground::StrokeCornerDL);
    setBackColorAt(rect.bottomLeft().x(), rect.bottomLeft().y(), color);
    setCellStyle(rect.bottomRight(), TerminalCellBackground::StrokeCornerDR);
    setBackColorAt(rect.bottomRight().x(), rect.bottomRight().y(), color);
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
            setColorsAt(x, y, TerminalBackground, KDColorWhite);
            Screen[y * CharWidth + x].character[0] = ' ';
        }
    }
}

static void clear(int count) {
    for (int i = 0; i < count - CharWidth; i++) {
        if (posY * CharWidth + posX + i >= CharWidth*CharHeight) continue;
        Screen[posY * CharWidth + posX + i].character[0] = ' ';
        Screen[posY * CharWidth + posX + i].fore = KDColorWhite;
        Screen[posY * CharWidth + posX + i].back = TerminalBackground;
    }
}

static KDColor getBackColorAt(int x, int y) {
    return KDColor::RGB16(Screen[y * CharWidth + x].back);
}

static void scrollDown() {
    copy(KDRect(0, 0, CharWidth, CharHeight-1), KDPoint(0, -1));
    clearRect(KDRect(0, CharHeight-2, CharWidth, 1));
}

static void newLine() {
    posX--;
    posY++;
    posX = 0;

    if (posY >= CharHeight-1) {
        // Scroll the screen up to have more space, and set the cursor's y position to the last line
        // Warning : cells that are not shown on screen after the scroll were overwritten and lost

        scrollDown();
        posY = CharHeight-2;
    }
}

static void incrementPos() {
    posX++;
    if (posX >= CharWidth) newLine();
}

static void decrementPos() {
    posX--;
    if (posX < 0) {
        posY--;
        posX = CharWidth-1;
    }
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

static void writeRectDecorate(const char* text, KDColor color = KDColorWhite, KDColor rectColor = KDColorRed, int count = -1) {
    int height = strlen(text) / CharWidth;
    int width = CharWidth - (strlen(text) % CharWidth);
    int x = posX;
    int y = posY;

    write(text, color);
    drawRect(KDRect(x, y, width, height), rectColor);
}

// TODO : much better implementation of the SecuredString
static void write(SecuredString* text, KDColor color = KDColorWhite) {
    write(text->c_str(), color, text->size());
}

static void writeB(const char* text, KDColor color = KDColorWhite, KDColor back = TerminalBackground, int count = -1) {
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
static void writeB(SecuredString* text, KDColor color = KDColorWhite, KDColor back = TerminalBackground) {
    writeB(text->c_str(), color, back, text->size());
}

static void writeStringAt(int x, int y, SecuredString* text, KDColor fore = KDColorWhite, KDColor back = TerminalBackground) {
    int lastX = posX;
    int lastY = posY;
    posX = x;
    posY = y;

    writeB(text->c_str(), fore, back, text->size());
}

static void writeStringAt(int x, int y, const char* text, KDColor fore = KDColorWhite, KDColor back = TerminalBackground) {
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
static void writeLn(SecuredString* text, KDColor color = KDColorWhite) {
    write(text->c_str(), color, text->size());
    newLine();
}

static void writeLnB(const char* text, KDColor color = KDColorWhite, KDColor back = TerminalBackground, int count = -1) {
    writeB(text, color, back, count);
    newLine();
}

// TODO : much better implementation of the SecuredString
static void writeLnB(SecuredString* text, KDColor color, KDColor back) {
    writeB(text->c_str(), color, back, text->size());
    newLine();
}

static void writeRectDecorateLn(const char* text, KDColor color = KDColorWhite, KDColor rectColor = KDColorRed, int count = -1) {
    newLine();
    int height = strlen(text) / CharWidth+2;
    int width = strlen(text) >= CharWidth ? CharWidth : strlen(text);
    int x = posX;
    int y = posY-1;

    writeLn(text, color);
    drawRect(KDRect(x, y, width, height), rectColor);
    newLine();
}

static void updateTextStatus() {
    int bufX = posX;
    int bufY = posY;
    if (Terminal::Keyboard::isAlpha()) {
        posX = CharWidth - 9;
        posY = CharHeight - 1;
        writeB("-> alpha", TerminalBackground, KDColorWhite);
    } else if (Terminal::Keyboard::isAlphaCaps()) {
        posX = CharWidth - 9;
        posY = CharHeight - 1;
        writeB("-> ALPHA", TerminalBackground, KDColorWhite);
    } else if (Terminal::Keyboard::isShift()) {
        posX = CharWidth - 9;
        posY = CharHeight - 1;
        writeB("-> shift", TerminalBackground, KDColorWhite);
    } else {
        posX = CharWidth - 9;
        posY = CharHeight - 1;
        writeB("        ", KDColorWhite, KDColorWhite);
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
            Screen[y * CharWidth + x].back = TerminalBackground;
            Screen[y * CharWidth + x].fore = KDColorWhite;
            Screen[y * CharWidth + x].backStyle = TerminalCellBackground::Fill;
        }
    }
    posY = 0;
    posX = 0;
    updateTextStatus();
}

static void writeBitmap(bool* bitmap, int width, int height, KDColor color) {
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++) {
            if (Terminal::Screen::posY + y >= CharHeight) {
                scrollDown();
            }
            if (bitmap[y * width + x]) setColorsAt(x, Terminal::Screen::posY + y, color, color);
            else setColorsAt(x, Terminal::Screen::posY + y, TerminalBackground, TerminalBackground);
        }
    }
    posX += width;
}

static bool safeLoop() {
    keyRead(); // Just to be sure ;)
    return !keyPressed(Ion::Keyboard::Key::Back);
}

static int readLn(char* buffer, int maxLength = 256, ReadLineSettings* settings = nullptr) {

    if (settings == nullptr) settings = ReadLineSettings::defaultSettings();
    if (history == 0) history = new StringPositionalList();
    int ptr = 0;
    int cursor = 0; //0-50 : invisible - 50-100 : visible
    int originX = posX;
    int originY = posY;
    while (true) {
        keyEnd();
        keyRead();
        if (!safeLoop()) {
            newLine();
            return EBREAK;
        }

        if (keyPressed(Ion::Keyboard::Key::OK) || keyPressed(Ion::Keyboard::Key::EXE)) {
            buffer[ptr] = '\0';
            if (settings->isHistoryEnabled()) history->copy(buffer, ptr);
            break;
        }

        if (keyPressed(Ion::Keyboard::Key::Alpha)) {
            Terminal::Keyboard::toggleAlpha();
            updateTextStatus();
        }
        
        if (keyPressed(Ion::Keyboard::Key::Shift)) {
            Terminal::Keyboard::toggleShift();
            updateTextStatus();
        }

        if (keyPressed(Ion::Keyboard::Key::Up)) {
            if (!history->canDecrement() || !settings->isHistoryEnabled()) continue;
            memset(buffer, '\0', 256);
            memcpy(buffer, history->selected()->c_str(), history->selected()->size());
            posX = originX;
            clear(256);
            write(buffer);
            ptr = history->selected()->size();
            history->decPointer();
            updateTextStatus();

            redraw();
        }

        if (keyPressed(Ion::Keyboard::Key::Down)) {
            if (!history->canIncrement() || !settings->isHistoryEnabled()) continue;
            memset(buffer, '\0', 256);
            memcpy(buffer, history->selected()->c_str(), history->selected()->size());
            posX = originX;
            clear(256);
            write(buffer);
            ptr = history->selected()->size();
            history->incPointer();
            updateTextStatus();

            redraw();
        }

        if (keyPressed(Ion::Keyboard::Key::Left)) {
            if (ptr <= 0) continue;
            ptr--;
            decrementPos();
        }

        if (keyPressed(Ion::Keyboard::Key::Right)) {
            if (ptr >= strlen(buffer)-1) continue;
            ptr++;
            incrementPos();
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
        LMAP(Ion::Keyboard::Key::Plus, '+', '+', 'z', 'Z');
        LMAP(Ion::Keyboard::Key::Minus, '-', '-', ' ', ' ');
        LMAP(Ion::Keyboard::Key::Multiplication, '*', '*', 'u', 'U');
        LMAP(Ion::Keyboard::Key::Division, '/', '/', 'v', 'V');
        LMAP(Ion::Keyboard::Key::LeftParenthesis, '(', '(', 'p', 'P');
        LMAP(Ion::Keyboard::Key::RightParenthesis, ')', ')', 'q', 'Q');
        LMAPSTR(Ion::Keyboard::Key::Sine, "sin(", "asin(", "g", "G");
        LMAPSTR(Ion::Keyboard::Key::Cosine, "cos(", "acos(", "h", "H");
        LMAPSTR(Ion::Keyboard::Key::Tangent, "tan(", "atan(", "i", "I");
        LMAPSTR(Ion::Keyboard::Key::Pi, "π", "=", "j", "J");
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
        LMAPSTR(Ion::Keyboard::Key::EE, "*10^", "*10^", "*10^", "*10^");
        LMAPSTR(Ion::Keyboard::Key::Ans, "ans", "ans", "ans", "ans");

        if (keyPressed(Ion::Keyboard::Key::Backspace)) {
            if (Terminal::Keyboard::isAlpha()) {
                buffer[ptr] = '%'; 
                writeChar(settings->doHideText() ? ':' : '%'); 
                ptr++;
                redraw();
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

            redraw();
        }

        // Update the daemons
        Terminal::Background::Hell::shared()->dispatchUpdate();

        // Update the cursor

        if (curPeriod) {
            // Decrease cursor
            cursor--;
            if (cursor < 0) {
                // Switching to increase
                curPeriod = false;
                redraw();
            }
        } else {
            // Increase cursor
            cursor++;
            if (cursor > 100) {
                // Switching to decrease
                curPeriod = true;
                redraw();
            }
        }
        
        keyEnd();
    }
    keyEnd();

    newLine();
    return ptr;
}

}

}

#endif