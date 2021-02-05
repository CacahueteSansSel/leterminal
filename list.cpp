#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void StringList::add(char* element) {
    if (counter >= LIST_MAX_COUNT) return;

    list[counter] = element;
    counter++;
}

void StringList::copy(char* element) {
    if (counter >= LIST_MAX_COUNT) return;

    list[counter] = new char[256];
    memcpy((char*)list[counter], element, strlen(element));
    list[counter][strlen(element)] = '\0';
    counter++;
}

char* StringList::at(int index) {
    if (counter >= LIST_MAX_COUNT) return "\0";

    return list[index];
}

void SecuredStringList::add(char* element, int size) {
    if (counter >= LIST_MAX_COUNT) return;

    list[counter] = new SecuredString(size, element);
    counter++;
}

void SecuredStringList::dispose() {
    for (int i = 0; i < counter; i++) {
        free(list[i]);
    }
    counter = 0;
}

void SecuredStringList::clear() {
    counter = 0;
}

void SecuredStringList::copy(char* element, int size) {
    if (counter >= LIST_MAX_COUNT) return;
    
    list[counter] = SecuredString::copy(size, element);
    counter++;
}

SecuredString* SecuredStringList::at(int index) {
    if (index >= count()) return SecuredString::empty();

    return list[index];
}

void StringPositionalList::shift() {
    for (int i = 0; i < counter; i++) {
        int finalIndex = i-1;
        if (finalIndex >= 0) list[finalIndex] = list[i];
    }
}

void StringPositionalList::add(char* element, int size) {
    if (size <= 0) return;
    if (counter >= LIST_MAX_COUNT) {
        shift();
        m_pointer = LIST_MAX_COUNT-1;
        list[LIST_MAX_COUNT-1] = new SecuredString(size, element);
        return;
    }

    list[counter] = new SecuredString(size, element);
    if (counter < LIST_MAX_COUNT) counter++;
}

void StringPositionalList::copy(char* element, int size) {
    if (size <= 0) return;
    if (counter >= LIST_MAX_COUNT) {
        shift();
        m_pointer = LIST_MAX_COUNT-1;
        list[LIST_MAX_COUNT-1] = SecuredString::copy(size, element);
        return;
    }
    
    list[counter] = SecuredString::copy(size, element);
    m_pointer = counter;
    if (counter < LIST_MAX_COUNT) counter++;
}

SecuredString* StringPositionalList::at(int index) {
    if (index >= count()) return SecuredString::empty();

    return list[index];
}

bool VolatileUInt8List::any(uint8_t item) {
    for (int i = 0; i < m_counter; i++) {
        if (list[i] == item) return true;
    }

    return false;
}

void VolatileUInt8List::append(uint8_t item) {
    int emptyIndex = nextEmptySpaceIndex();
    if (emptyIndex != -1) {
        list[emptyIndex] = item;
        return;
    }
    if (m_counter >= LIST_MAX_COUNT) return;
    list[m_counter] = item;
    m_counter++;
}

void VolatileUInt8List::clear() {
    for (int i = 0; i < m_counter; i++) list[i] = 0;
    m_counter = 0;
}

int VolatileUInt8List::nextEmptySpaceIndex() {
    for (int i = 0; i < m_counter; i++) if (list[i] == 0) return i;
    
    return -1;
}

void VolatileUInt8List::clear(uint8_t item) {
    for (int i = 0; i < m_counter; i++) if (item == list[i]) list[i] = 0;
}