#ifndef TERMINAL_VFS_ION
#define TERMINAL_VFS_ION

#include "vfs_node.h"
#include <ion.h>

namespace Terminal {

namespace VFS {

class IonStorageNode : public VFSNode {
    public:
    IonStorageNode() : VFSNode("ram")
    {}

    int childCount() override {
        // TODO : shows every files, not only .py files
        return Ion::Storage::sharedStorage()->numberOfRecordsWithExtension("py");
    }

    VFSNode* provideChild(const char* name) override {
        auto record = Ion::Storage::sharedStorage()->recordNamed(name);

        if (record.isNull()) return nullptr;
        return (new VFSNode(record.fullName(), record.value().buffer, record.value().size))->withParent(this);
    }

    VFSNode* provideChild(int index) override {
        auto record = Ion::Storage::sharedStorage()->recordWithExtensionAtIndex("py", index);

        if (record.isNull()) return nullptr;
        return (new VFSNode(record.fullName(), record.value().buffer, record.value().size))->withParent(this);
    }

    bool write(VFSNode* node) override {
        auto error = Ion::Storage::sharedStorage()->createRecordWithFullName(node->name()->c_str(), node->data(), node->dataLength());
        return error == Ion::Storage::Record::ErrorStatus::None;
    }

    bool remove(const char* name) override {
        auto record = Ion::Storage::sharedStorage()->recordNamed(name);
        if (record.isNull()) return false;
        record.destroy();
        return true;
    }
};

}

}

#endif