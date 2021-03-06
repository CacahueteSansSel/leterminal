#include "vfs.h"
#include "vfs_node.h"

namespace Terminal {

namespace VFS {

SecuredStringList* pathList;
VFSNode* m_root;
VFSNode* m_current;

void mount(VFSNode* node) {
    node->setParent(m_current);
    m_current->write(node);
}

void mountVirtual() {
    mount(new IonStorageNode());
}

void init() {
    pathList = new SecuredStringList();
    m_root = new VFSNode("/");
    m_root->setParent(m_root); // Even worse than consanguinity !! Parent of itself !!
    m_current = m_root;

    mountVirtual();
}

VFSNode* fetch(SecuredString* path) {
    split(pathList, path->c_str(), path->size(), '/');
    
    VFSNode* node = path->at(0) == '/' ? m_root : m_current;
    int listPtr = 0;
    while (listPtr < pathList->count()) {
        SecuredString* entry = pathList->at(listPtr);
        if (entry->size() == 0) continue;
        VFSNode* tmp = node->provideChild(entry->c_str());
        if (tmp == nullptr) {
            // Error
            pathList->dispose();
            return nullptr;
        } else if (tmp->type() == VFSNodeType::Data) {
            node = tmp;
            break;
        }
        node = tmp;
    }
    pathList->dispose();
    return node;
}

VFSNode* current() {
    return m_current;
}

bool warp(const char* name) {
    VFSNode* rootChild = m_root->provideChild(name);
    VFSNode* tmp = check(name, "/") ? m_root : (rootChild == nullptr ? m_current->provideChild(name) : rootChild);
    if (tmp == nullptr || tmp->type() != VFSNodeType::NodeContainer) return false;

    m_current = tmp;
    return true;
}

void goBackwards() {
    m_current = m_current->parent();
}

}

}