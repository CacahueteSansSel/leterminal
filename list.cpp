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

void SecuredStringList::copy(char* element, int size) {
    if (counter >= LIST_MAX_COUNT) return;
    
    list[counter] = SecuredString::copy(size, element);
    counter++;
}

SecuredString SecuredStringList::at(int index) {
    if (index >= count()) return *SecuredString::empty();
    if (counter >= LIST_MAX_COUNT) return *SecuredString::empty();

    return *list[index];
}

void StringPositionalList::shift() {
    for (int i = 0; i < counter; i++) {
        int finalIndex = i-1;
        if (finalIndex >= 0) list[finalIndex] = list[i];
    }
}

void StringPositionalList::add(char* element, int size) {
    if (counter >= LIST_MAX_COUNT) {
        shift();
    }

    list[counter] = new SecuredString(size, element);
    counter++;
}

void StringPositionalList::copy(char* element, int size) {
    if (counter >= LIST_MAX_COUNT) {
        shift();
    }
    
    list[counter] = SecuredString::copy(size, element);
    m_pointer = counter;
    counter++;
}

SecuredString StringPositionalList::at(int index) {
    if (index >= count()) return *SecuredString::empty();

    return *list[index];
}