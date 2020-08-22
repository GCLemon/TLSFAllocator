#pragma once

#include <atomic>
#include <cassert>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#else
#include <cstddef>
#include <dlfcn.h>
#endif

class DynamicLinkLibrary
{
private:
	mutable std::atomic<int32_t> m_reference;

#if _WIN32
	HMODULE m_dll;
#else
	void* m_dll = nullptr;
#endif

public:
	DynamicLinkLibrary()
	{
		m_dll = nullptr;
	}

	~DynamicLinkLibrary()
	{
		Reset();
	}

	void Reset()
	{
		if (m_dll != nullptr)
		{
#if _WIN32
			::FreeLibrary(m_dll);
#else
			dlclose(m_dll);
#endif
			m_dll = nullptr;
		}
	}

	bool Load(const char* path)
	{
#if _WIN32
		m_dll = ::LoadLibraryA(path);
#else
		m_dll = dlopen(path, RTLD_LAZY);
#endif
		return m_dll != nullptr;
	}

	template <typename T> T GetProc(const char* name)
	{
#if _WIN32
		void* pProc = ::GetProcAddress(m_dll, name);
#else
		void* pProc = dlsym(m_dll, name);
#endif
		return (T)(pProc);
	}

	int AddRef()
	{
		std::atomic_fetch_add_explicit(&m_reference, 1, std::memory_order_consume);
		return m_reference;
	}

	int GetRef()
	{
		return m_reference;
	}

	int Release()
	{
		assert(m_reference > 0);
		bool destroy = std::atomic_fetch_sub_explicit(&m_reference, 1, std::memory_order_consume) == 1;
		if (destroy)
		{
			delete this;
			return 0;
		}
		return m_reference;
	}
};
