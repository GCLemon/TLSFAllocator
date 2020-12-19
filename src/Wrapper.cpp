#include <stdio.h>
#include <stdint.h>

#include "TLSFAllocator.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#include <Windows.h>
#endif

#ifndef DLL_EXPORT
#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif
#endif

#ifndef STDCALL
#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

extern "C"
{
    DLL_EXPORT void* STDCALL dll_TLSFAllocator_Constructor_0(void* ptr, size_t size, size_t split)
    {
        return new tlsf::TLSFAllocator(ptr, size, split);
    }

    DLL_EXPORT void* STDCALL dll_TLSFAllocator_Constructor_1(size_t size, size_t split)
    {
        return new tlsf::TLSFAllocator(size, split);
    }

    DLL_EXPORT void STDCALL dll_TLSFAllocator_Destructor(void* selfPtr, size_t size, size_t split)
    {
        tlsf::TLSFAllocator* selfAllocator = (tlsf::TLSFAllocator*)selfPtr;
        delete selfAllocator;
    }

    DLL_EXPORT bool STDCALL dll_TLSFAllocator_GetIsThreadSafe(void* selfPtr)
    {
        tlsf::TLSFAllocator* selfAllocator = (tlsf::TLSFAllocator*)selfPtr;
        return selfAllocator->GetIsThreadSafe();
    }

    DLL_EXPORT void STDCALL dll_TLSFAllocator_SetIsThreadSafe(void* selfPtr, bool isThreadSafe)
    {
        tlsf::TLSFAllocator* selfAllocator = (tlsf::TLSFAllocator*)selfPtr;
        selfAllocator->SetIsTrehadSafe(isThreadSafe);
    }

    DLL_EXPORT void* STDCALL dll_TLSFAllocator_Alloc(void* selfPtr, const size_t size)
    {
        tlsf::TLSFAllocator* selfAllocator = (tlsf::TLSFAllocator*)selfPtr;
        return selfAllocator->Alloc(size);
    }

    DLL_EXPORT void STDCALL dll_TLSFAllocator_Free(void* selfPtr, void* ptr)
    {
        tlsf::TLSFAllocator* selfAllocator = (tlsf::TLSFAllocator*)selfPtr;
        selfAllocator->Free(ptr);
    }

    DLL_EXPORT void STDCALL dll_TLSFAllocator_PrintDebugInfo(void* selfPtr)
    {
        tlsf::TLSFAllocator* selfAllocator = (tlsf::TLSFAllocator*)selfPtr;
        selfAllocator->PrintDebugInfo();
    }
}