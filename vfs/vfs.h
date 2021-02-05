#ifndef TERMINAL_VFS
#define TERMINAL_VFS
#include "vfs_node.h"
#include "vfs_ionstorage_node.h"
#include "vfs_externalapp_node.h"
#include "../stringx.h"
#include "../list.h"

namespace Terminal {

namespace VFS {

class VirtualFS {

    public:
    VirtualFS() {}

    static VirtualFS* sharedVFS() {
        static VirtualFS vfs;
        return &vfs;
    }

    void mount(VFSNode* node) {
        node->setParent(m_current);
        m_current->write(node);
    }

    void mountVirtual() {
        mount(new IonStorageNode());
        mount(new ExternalAppStorageNode());
    }

    void init() {
        m_root = new VFSNode("/");
        m_root->setParent(m_root); // Even worse than consanguinity !! Parent of itself !!
        m_current = m_root;

        mountVirtual();
    }

    VFSNode* fetch(SecuredString* path) {
        if (pathList == nullptr) pathList = new SecuredStringList();
        split(pathList, path->c_str(), path->size(), '/');
        
        VFSNode* node = path->at(0) == '/' ? m_root : m_current;
        int listPtr = 0;
        while (listPtr < pathList->count()) {
            SecuredString* entry = pathList->at(listPtr);
            if (entry->size() == 0) continue;
            VFSNode* tmp = node->provideChild(entry->c_str());
            if (tmp == nullptr) {
                // Error
                return nullptr;
            } else if (tmp->type() == VFSNodeType::Data) {
                node = tmp;
                break;
            }
            node = tmp;
        }
        return node;
    }

    VFSNode* current() {
        return m_current;
    }

    void warp(const char* name) {
        VFSNode* tmp = check(name, "/") ? m_root : m_root->provideChild(name);
        if (tmp == nullptr || tmp->type() != VFSNodeType::NodeContainer) return;

        m_current = tmp;
    }

    void goBackwards() {
        m_current = m_current->parent();
    }

    private:
    SecuredStringList* pathList;
    VFSNode* m_root;
    VFSNode* m_current;
};

}

}

#endif