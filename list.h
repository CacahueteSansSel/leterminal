#ifndef TERMINAL_LIST
#define TERMINAL_LIST

#define LIST_MAX_COUNT 64
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ion.h>

class SecuredString {
    public:

    static SecuredString* empty() {
        return new SecuredString(0, "");
    }

    static SecuredString* fromBufferUnsafe(char* buffer) {
        return new SecuredString(strlen(buffer), buffer);
    }

    static SecuredString* copy(int _size, char* _chars) {
        char* newbuffer = (char*)malloc(_size+1);

        memcpy(newbuffer, _chars, _size);
        SecuredString* str = new SecuredString(_size, newbuffer);

        return str;
    }

    bool validate() const { return Ion::crc32Byte((const uint8_t*)m_chars, m_size) == m_safetyHash; }

    int size() const { return m_size; }
    char* c_str() const {
        if (!validate()) {
            return nullptr;
        }
        // We ensure that the string is null-terminated to avoid shit
        if (m_chars[m_size] != '\0') m_chars[m_size] = '\0';

        return m_chars;
    }
    char at(int index) const { return index >= size() ? '\0' : m_chars[index]; }
    bool matches(const char* text) {
        if (strlen(text) != size()) return false;

        for (int i = 0; i < size(); i++) {
            if (text[i] != at(i)) return false;
        }

        return true;
    }

    SecuredString(int _size, char* _chars) : m_size(_size), m_chars(_chars), m_safetyHash(Ion::crc32Byte((const uint8_t*)_chars, _size)) {}
    private:
    int m_size;
    uint32_t m_safetyHash;
    char* m_chars;
};

class StringList {
    private:
    char* list[LIST_MAX_COUNT];
    int counter = 0;

    public:
    int count() const { return counter; }

    void add(char* element);
    void copy(char* element);
    char* at(int index);
};

class SecuredStringList {
    private:
    SecuredString* list[LIST_MAX_COUNT];
    int counter = 0;

    public:
    SecuredStringList() {}
    int count() const { return counter; }

    void dispose();
    void clear();
    void add(char* element, int size);
    void copy(char* element, int size);
    SecuredString* at(int index);
};

class StringPositionalList {
    private:
    SecuredString* list[LIST_MAX_COUNT];
    int counter = 0;
    int m_pointer = 0;

    public:

    StringPositionalList() {}
    int count() const { return counter; }
    SecuredString* selected() const { return m_pointer >= count() ? SecuredString::empty() : list[m_pointer]; }
    int pointer() const { return m_pointer; }

    void shift();
    void add(char* element, int size);
    void copy(char* element, int size);
    SecuredString* at(int index);
    int setPointer(int ptr) { m_pointer = ptr; }
    bool canIncrement() { return m_pointer < count()+1 && count() > 0; }
    bool canDecrement() { return m_pointer > 0; }
    bool incPointer() {
        if (m_pointer < count()+1) {
            m_pointer++;
            return true;
        }

        return false;
    }
    bool decPointer() {
        if (m_pointer > 0) {
            m_pointer--;
            return true;
        }

        return false;
    }
    void clear() {
        m_pointer = 0;
    }
};

class VolatileUInt8List {
    private:
    uint8_t list[LIST_MAX_COUNT];
    int m_counter = 0;

    public:
    VolatileUInt8List() {}

    int count() {return m_counter;}
    int aliveCount();
    int nextEmptySpaceIndex();
    uint8_t at(int index) {return list[index];}
    bool any(uint8_t item);
    void append(uint8_t item);
    void clear();
    void clear(uint8_t item);
};

#endif