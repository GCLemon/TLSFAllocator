#pragma once

#include "TLSFAllocator.h"

#ifndef DECLSPEC
#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC
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
	DECLSPEC void* STDCALL dll_TLSFAllocator_Constructor_0(void* ptr, size_t size, size_t split);

	DECLSPEC void* STDCALL dll_TLSFAllocator_Constructor_1(size_t size, size_t split);

	DECLSPEC void STDCALL dll_TLSFAllocator_Destructor(void* selfPtr, size_t size, size_t split);

	DECLSPEC void* STDCALL dll_TLSFAllocator_Alloc(void* selfPtr, const size_t size);

	DECLSPEC void STDCALL dll_TLSFAllocator_Free(void* selfPtr, void* ptr);

	DECLSPEC void STDCALL dll_TLSFAllocator_PrintDebugInfo(void* selfPtr);
}