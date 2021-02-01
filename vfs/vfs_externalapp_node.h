#ifndef TERMINAL_VFS_EXTAPP
#define TERMINAL_VFS_EXTAPP

#include "vfs_node.h"
#include <ion.h>
#include "../external/archive.h"
#include "../events.h"
#include "../external/heap.h"

namespace Terminal {

namespace VFS {

class ExternalAppNode : public VFSNode {
    public:
    ExternalAppNode(struct External::Archive::File file) : VFSNode(file.name), m_file(file)
    {}

    VFSNode* provideChild(const char* name) override {
        return nullptr;
    }

    VFSNode* provideChild(int index) override {
        return nullptr;
    }

    VFSNodeType type() override {return VFSNodeType::Data;}
    int dataLength() override {return (int)m_file.dataLength;}
    const void* data() override {return m_file.data;}
    bool isExecutable() override {return m_file.isExecutable;}
    int execute() override {
        if (!isExecutable()) return EEXTNOTEXECUTABLE;

        return External::Archive::executeFile(m_file.name, external_heap, HEAP_SIZE);
    }

    bool write(VFSNode* node) override {
        return false;   
    }

    bool remove(const char* name) override {
        return false;
    }
    private:
    struct External::Archive::File m_file;
};

class ExternalAppStorageNode : public VFSNode {
    public:
    ExternalAppStorageNode() : VFSNode("ext")
    {}

    int childCount() override {
        return (int)External::Archive::numberOfFiles();
    }

    VFSNodeType type() override {return VFSNodeType::NodeContainer;}
    VFSNode* provideChild(const char* name) override {
        int index = External::Archive::indexFromName(name);
        struct External::Archive::File file;

        if (!External::Archive::fileAtIndex(index, file)) {
            return nullptr;
        }
        return new ExternalAppNode(file);
    }

    VFSNode* provideChild(int index) override {
        struct External::Archive::File file;
        if (!External::Archive::fileAtIndex(index, file)) {
            return nullptr;
        }
        return new ExternalAppNode(file);
    }

    bool write(VFSNode* node) override {
        return false;   
    }

    bool remove(const char* name) override {
        return false;
    }
};

}

}

#endif