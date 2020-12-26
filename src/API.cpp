#include "TLSFAllocator.h"

namespace TLSFAllocator
{
    bool Initialize(uint64_t bufferSize, bool isMonitorMode) { return TLSFAllocator::Initialize(bufferSize, isMonitorMode); }

    bool Terminate() { return TLSFAllocator::Terminate(); }

    void* Malloc(size_t count, size_t size)
    {
        static TLSFAllocator* instance = TLSFAllocator::GetInstance();
        return instance->Malloc(count, size);
    }

    void* Calloc(size_t count, size_t size)
    {
        static TLSFAllocator* instance = TLSFAllocator::GetInstance();
        return instance->Calloc(count, size);
    }

    void* Realloc(void* address, size_t count, size_t size)
    {
        static TLSFAllocator* instance = TLSFAllocator::GetInstance();
        return instance->Realloc(address, count, size);
    }

    void Free(void* address)
    {
        static TLSFAllocator* instance = TLSFAllocator::GetInstance();
        return instance->Free(address);
    }
}