#include "Wrapper.h"

extern "C"
{
	DECLSPEC void* STDCALL dll_TLSFAllocator_Constructor_0(void* ptr, size_t size, size_t split)
	{
		return new TLSFAllocator(ptr, size, split);
	}

	DECLSPEC void* STDCALL dll_TLSFAllocator_Constructor_1(size_t size, size_t split)
	{
		return new TLSFAllocator(size, split);
	}

	DECLSPEC void STDCALL dll_TLSFAllocator_Destructor(void* selfPtr, size_t size, size_t split)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		delete selfAllocator;
	}

	DECLSPEC void* STDCALL dll_TLSFAllocator_Alloc(void* selfPtr, const size_t size)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		return selfAllocator->Alloc(size);
	}

	DECLSPEC void STDCALL dll_TLSFAllocator_Free(void* selfPtr, void* ptr)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		selfAllocator->Free(ptr);
	}

	DECLSPEC void STDCALL dll_TLSFAllocator_PrintDebugInfo(void* selfPtr)
	{
		TLSFAllocator* selfAllocator = (TLSFAllocator*)selfPtr;
		selfAllocator->PrintDebugInfo();
	}
}