#ifndef EXTERNAL_HEAP
#define EXTERNAL_HEAP
#include <stdio.h>
#include <stdlib.h>

#define HEAP_SIZE 5000

static char* external_heap;

static bool allocateHeap() {
    external_heap = (char*)malloc(HEAP_SIZE);
    if (external_heap == nullptr || external_heap == NULL) {
        // Not enough memory to allocate
        return false;
    }
    return true;
}

static void freeHeap() {
    memset(external_heap, 0, HEAP_SIZE);
    free(external_heap);
}

#endif