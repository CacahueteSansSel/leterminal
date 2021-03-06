#ifndef TERMINAL_VFS
#define TERMINAL_VFS
#include "vfs_node.h"
#include "vfs_ionstorage_node.h"
#include "../stringx.h"
#include "../list.h"

namespace Terminal {

namespace VFS {

void init();
void mount(VFSNode* node);
void mountVirtual();
VFSNode* fetch(SecuredString* path);
VFSNode* current();
bool warp(const char* name);
void goBackwards();

}

}

#endif