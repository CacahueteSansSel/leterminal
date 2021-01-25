#ifndef SIGMA_TERMINAL_USERS
#define SIGMA_TERMINAL_USERS
#include "../stringx.h"

#define MAX_USER_COUNT 32

enum class ExecutionLevel {
    Low,
    Normal,
    High,
    Root
};

class User {
    private:
    SecuredString m_name;
    ExecutionLevel m_level;
    int m_uid;

    public:
    User(SecuredString name, ExecutionLevel level) : m_name(name), m_level(level), m_uid(-1)
    {}

    void setUid(int uid) {
        if (m_uid >= 0) return;

        m_uid = uid;
    }
    SecuredString name() {return m_name;}
    ExecutionLevel level() {return m_level;}
    int uid() {return m_uid;}
};

class UsersRepository {
    public:
    UsersRepository() : m_userPointer(0), m_currentUserIndex(1), m_alreadyAddedDefaultUsers(false) {}

    static UsersRepository* sharedRepository() {
        static UsersRepository rep;
        rep.setupDefaultUsers();
        return &rep;
    }

    User* current() {return m_users[m_currentUserIndex];}
    User* at(int index) {
        if (index >= m_userPointer) return nullptr;
        return m_users[index];
    }
    int count() {return m_userPointer;}

    int indexOfUsername(SecuredString name) {
        for (int i = 0; i < m_userPointer; i++) {
            if (name == m_users[i]->name()) return i;
        }

        return -1;
    }

    bool switchUser(SecuredString name) {
        int index = indexOfUsername(name);
        if (index < 0) return false;

        m_currentUserIndex = index;
        return true;
    }

    bool addUser(User* user) {
        for (int i = 0; i < m_userPointer; i++) {
            if (user->name() == m_users[i]->name()) return false;
        }

        if (m_userPointer >= MAX_USER_COUNT) return false;
        m_users[m_userPointer] = user;
        user->setUid(m_userPointer);
        m_userPointer++;

        return true;
    }

    private:
    bool m_alreadyAddedDefaultUsers;
    void setupDefaultUsers() {
        if (m_alreadyAddedDefaultUsers) return;
        addUser(new User(*SecuredString::fromBufferUnsafe("root"), ExecutionLevel::Root));
        addUser(new User(*SecuredString::fromBufferUnsafe("cacahuete"), ExecutionLevel::Normal));
        addUser(new User(*SecuredString::fromBufferUnsafe("sandy"), ExecutionLevel::Low));
        addUser(new User(*SecuredString::fromBufferUnsafe("kevin"), ExecutionLevel::High));
        m_alreadyAddedDefaultUsers = true;
    }
    int m_currentUserIndex;
    int m_userPointer;
    User* m_users[MAX_USER_COUNT];
};

#endif