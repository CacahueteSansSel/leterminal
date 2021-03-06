#ifndef STRINGX
#define STRINGX
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

bool startsWith(const char* target, const char* text);
bool startsWith(SecuredString* target, SecuredString* text);
bool check(const char* target, const char* text);
bool check(SecuredString* target, SecuredString* text);
bool check(SecuredString* target, char* text);
SecuredStringList* split(SecuredStringList* sourceList, char* text, int textLength, char separator);
char intToString(int value);
char uintToString(uint32_t value);
bool operator==(SecuredString left, SecuredString right);

#endif