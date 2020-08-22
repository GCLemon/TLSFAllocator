#include "Wrapper.h"

extern "C"
{
	DLL_EXPORT void* STDCALL dll_TLSFAllocator_Constructor_0(void* ptr, size_t size, size_t split)
	{
		return new TLSFAllocator(ptr, size, split);
	}

	DLL_EXPORT void* STDCALL dll_TLSFAllocator_Constructor_1(size_t size, size_t split)
	{
		return new TLSFAllocator(size, split);
	}

	DLL_EXPORT void STDCALL dll_TLSFAllocator_Destructor(void* selfPtr, size_t size, size_t split)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		delete selfAllocator;
	}

	DLL_EXPORT void* STDCALL dll_TLSFAllocator_Alloc(void* selfPtr, const size_t size)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		return selfAllocator->Alloc(size);
	}

	DLL_EXPORT void STDCALL dll_TLSFAllocator_Free(void* selfPtr, void* ptr)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		selfAllocator->Free(ptr);
	}

	DLL_EXPORT void STDCALL dll_TLSFAllocator_PrintDebugInfo(void* selfPtr)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		selfAllocator->PrintDebugInfo();
	}
}