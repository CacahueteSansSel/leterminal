#ifndef STRINGX
#define STRINGX
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

static char currentBuffer[100];
static char itemBuffer[64];

static bool startsWith(const char* target, const char* text) {
    int count = strlen(text);
    for (int i = 0; i < count; i++) {
        if (target[i] != text[i]) return false;
    }

    return true;
}

static bool startsWith(SecuredString target, SecuredString text) {
    int count = text.size();
    for (int i = 0; i < count; i++) {
        if (target.at(i) != text.at(i)) return false;
    }

    return true;
}

static bool check(const char* target, const char* text) {
    if (strlen(target) != strlen(text)) return false;
    return startsWith(target, text);
}

static bool check(SecuredString target, SecuredString text) {
    return startsWith(target, text);
}

static bool check(SecuredString target, char* text) {
    return startsWith(target, *SecuredString::fromBufferUnsafe(text));
}

static SecuredStringList* split(SecuredStringList* sourceList, char* text, int textLength, char separator) {
    int itemPtr = 0;
    int textPtr = 0;
    bool inQuotes = false;
    SecuredStringList* list = sourceList == nullptr ? new SecuredStringList() : sourceList;

    for (int i = 0; i < textLength; i++) {
        if (text[i] == separator && !inQuotes) {
            currentBuffer[textPtr] = '\0';
            list->copy(currentBuffer, textPtr);
            memset(currentBuffer, '\0', 100);
            textPtr = 0;
            continue;
        }
        
        if (text[i] == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        
        currentBuffer[textPtr] = text[i];
        textPtr++;
    }
    currentBuffer[textPtr] = '\0';
    list->copy(currentBuffer, textPtr);
    memset(currentBuffer, '\0', 100);

    return list;
}

static char intToString(int value) {
    return '0' + value;
}

static bool operator==(SecuredString left, SecuredString right) {
    return check(left, right);
}

#endif