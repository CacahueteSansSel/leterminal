#ifndef TERMINAL_VFS_NODE
#define TERMINAL_VFS_NODE
#include "../stringx.h"
#include "../list.h"

#define VFS_CONTAINER_MAX 32

namespace Terminal {

namespace VFS {

enum class VFSNodeType {
    Data,
    NodeContainer
};

class VFSNode {
    public:

    VFSNode(const char* name) : m_name(SecuredString::fromBufferUnsafe((char*)name)), m_dataLength(0)
    {}
    VFSNode(const char* name, const void* data, int dataLength) : m_name(SecuredString::fromBufferUnsafe((char*)name)), m_data(data), m_dataLength(dataLength)
    {}

    virtual SecuredString name() { return *m_name; }

    virtual VFSNodeType type() { return m_dataLength > 0 ? VFSNodeType::Data : VFSNodeType::NodeContainer; }

    virtual int childCount() { return type() != VFSNodeType::NodeContainer ? 0 : childrenPtr; }

    virtual int dataLength() { return type() == VFSNodeType::Data ? m_dataLength : 0; }

    virtual const void* data() { return type() == VFSNodeType::Data ? m_data : nullptr; }

    virtual VFSNode* provideChild(const char* name) {
        if (type() != VFSNodeType::NodeContainer) return nullptr;
        for (int i = 0; i < childCount(); i++) {
            if (check(children[i]->name(), *SecuredString::fromBufferUnsafe((char*)name))) return children[i];
        }

        return nullptr;
    }

    virtual VFSNode* provideChild(int index) {
        if (type() != VFSNodeType::NodeContainer) return nullptr;
        return children[index];
    }

    virtual bool write(VFSNode* node) {
        children[childrenPtr] = node;
        childrenPtr++;
        return true;
    }

    void setParent(VFSNode* parent) {m_parent = parent;}

    VFSNode* withParent(VFSNode* parent) {
        setParent(parent);
        return this;
    }

    virtual VFSNode* parent() {return m_parent;}

    private:
    SecuredString* m_name;
    const void* m_data;
    int m_dataLength;
    VFSNode* children[VFS_CONTAINER_MAX];
    VFSNode* m_parent;
    int childrenPtr = 0;
};

}

}

#endif