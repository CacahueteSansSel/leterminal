#include "users.h"

void User::setUid(useruid_t uid)
{
    if (m_uid != USERUID_INVALID) return;

    m_uid = uid;
}

namespace Terminal {

namespace Users {

useruid_t currentUserUid = 0;
useruid_t userPtr = 0;
User* userList[MAX_USER_COUNT];  
bool initied = false;

User* at(useruid_t index)
{
    if (index >= userPtr) return nullptr;
    return userList[index];
}

useruid_t getUserUidFromUsername(SecuredString* name) 
{
    for (useruid_t i = 0; i < userPtr; i++) {
        if (*name == *userList[i]->name()) return i;
    }

    return USERUID_INVALID;
}

bool switchUser(useruid_t uid) {
    if (uid == USERUID_INVALID) return false;
    
    currentUserUid = uid;
    return true;
}

bool add(User* user) {
    for (useruid_t i = 0; i < userPtr; i++) {
        if (*user->name() == *userList[i]->name()) return false;
    }

    if (userPtr >= MAX_USER_COUNT) return false;
    userList[userPtr] = user;
    user->setUid(userPtr);
    userPtr++;

    return true;
}

void init() {
    if (initied) return;
    add(new User(SecuredString::fromBufferUnsafe("cacahuete"), ExecutionLevel::Normal));
    add(new User(SecuredString::fromBufferUnsafe("boat"), ExecutionLevel::Low));
    add(new User(SecuredString::fromBufferUnsafe("coconut"), ExecutionLevel::High));
    add(new User(SecuredString::fromBufferUnsafe("root"), ExecutionLevel::Root));

    initied = true;
}

useruid_t currentUid() {
    return currentUserUid;
}

User* currentUser() {
    return userList[currentUserUid];
}

uint32_t count() {
    return userPtr;
}

}

}