#ifndef TERMINAL_USERS
#define TERMINAL_USERS
#include "../list.h"
#include "../stringx.h"

#define MAX_USER_COUNT 16

typedef uint32_t useruid_t;
#define USERUID_INVALID (useruid_t)255

enum class ExecutionLevel {
    Low,
    Normal,
    High,
    Root
};

class User {
    private:
    SecuredString* m_name;
    ExecutionLevel m_level;
    useruid_t m_uid;

    public:
    User(SecuredString* name, ExecutionLevel level) : m_name(name), m_level(level), m_uid(USERUID_INVALID)
    {}

    void setUid(useruid_t uid);
    SecuredString* name() {return m_name;}
    ExecutionLevel level() {return m_level;}
    useruid_t uid() {return m_uid;}
};

namespace Terminal {

namespace Users {
    
void init();
User* at(useruid_t uid);
uint32_t count();
useruid_t getUserUidFromUsername(SecuredString* name);
useruid_t currentUid();
User* currentUser();
bool add(User* user);
bool remove(useruid_t uid);
bool switchUser(useruid_t uid);

}

}

#endif