#pragma once

#include <memory>
#include <string>

#include "DynamicLinkLibrary.h"

/**
 @brief TLSFメモリアロケータのライブラリ
 */
static std::shared_ptr<DynamicLinkLibrary> dllTlsf = nullptr;

/**
 @brief プログラム起動時にライブラリを読み込む
 */
static void Initialize()
{
	dllTlsf = std::shared_ptr<DynamicLinkLibrary>(new DynamicLinkLibrary());

#ifdef _WIN32
	dllTlsf->Load("TLSFAllocator.dll");
#elif defined(__APPLE__)
	dllTlsf->Load("libTLSFAllocator.dylib");
#else
	dllTlsf->Load("libTLSFAllocator.so");
#endif
}

/**
 @brief プログラム終了時にライブラリをクリアする
 */
static void Terminate()
{
	dllTlsf->Reset();
}

/**
 @brief メモリアロケータ
 */
class TLSFAllocator
{
private:
	/**
	 @brief 自身のポインタ
	 */
	void* selfPtr = nullptr;

	static void* dll_TLSFAllocator_Constructor(void* ptr, size_t size, size_t split)
	{
		typedef void* (*proc_t)(void*, size_t, size_t);
		static proc_t proc = dllTlsf->GetProc<proc_t>("dll_TLSFAllocator_Constructor_0");
		return proc(ptr, size, split);
	}

	static void  dll_TLSFAllocator_Destructor(void* selfPtr)
	{
		typedef void (*proc_t)(void*);
		static proc_t proc = dllTlsf->GetProc<proc_t>(" dll_TLSFAllocator_Destructor_1");
		proc(selfPtr);
		selfPtr = nullptr;
	}

	static void* dll_TLSFAllocator_Alloc(void* selfPtr, const size_t size)
	{
		typedef void* (*proc_t)(void*, const size_t);
		static proc_t proc = dllTlsf->GetProc<proc_t>("dll_TLSFAllocator_Alloc");
		return proc(selfPtr, size);
	}

	static void dll_TLSFAllocator_Free(void* selfPtr, void* ptr)
	{
		typedef void (*proc_t)(void*, void*);
		static proc_t proc = dllTlsf->GetProc<proc_t>("dll_TLSFAllocator_Free");
		proc(selfPtr, ptr);
	}

	static void dll_TLSFAllocator_PrintDebugInfo(void* selfPtr)
	{
		typedef void (*proc_t)(void*);
		static proc_t proc = dllTlsf->GetProc<proc_t>("dll_TLSFAllocator_PrintDebugInfo");
		proc(selfPtr);
	}

public:
	/**
	 @brief コンストラクタ
	 @param ptr 初期化した領域の先頭アドレス
	 @param size 初期化した領域のサイズ
	 @param split 第2カテゴリの分割数
	 */
	TLSFAllocator(void* ptr, size_t size, size_t split = 8)
	{
		selfPtr = dll_TLSFAllocator_Constructor(ptr, size, split);
	}

	/**
	 @brief デストラクタ
	 */
	~TLSFAllocator()
	{
		dll_TLSFAllocator_Destructor(selfPtr);
	}

	/**
	 @brief 領域を確保する
	 @param size 確保する領域のサイズ
	 @return 確保した領域の先頭アドレス(ブロック全体のサイズではない)
	 */
	void* Alloc(const size_t size)
	{
		return dll_TLSFAllocator_Alloc(selfPtr, size);
	}

	/**
	 @brief 確保した領域を解放する
	 @param ptr 解放する領域の先頭アドレス(ブロック全体のサイズではない)S
	 */
	void Free(void* ptr)
	{
		dll_TLSFAllocator_Free(selfPtr, ptr);
	}

	/**
	 @brief 現在のメモリの内容をプリントする
	 */
	void PrintBufferInfo()
	{
		dll_TLSFAllocator_PrintDebugInfo(selfPtr);
	}
};