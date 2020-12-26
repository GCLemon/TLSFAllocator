#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <iostream>
#include <vector>

namespace TLSFAllocator
{
    bool Initialize(uint64_t bufferSize, bool isMonitorMode);

    bool Terminate();

    void* Malloc(size_t count, size_t size);

    void* Calloc(size_t count, size_t size);

    void* Realloc(void* address, size_t count, size_t size);

    void Free(void* address);

    void PrintDebugInfo();
}