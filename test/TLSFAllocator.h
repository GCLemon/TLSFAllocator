#pragma once

#include <memory>
#include <string>

#include "DynamicLinkLibrary.h"

/**
 @brief TLSF�������A���P�[�^�̃��C�u����
 */
static std::shared_ptr<DynamicLinkLibrary> dllTlsf = nullptr;

/**
 @brief �v���O�����N�����Ƀ��C�u������ǂݍ���
 */
void Initialize()
{
    dllTlsf = std::shared_ptr<DynamicLinkLibrary>(new DynamicLinkLibrary());

#ifdef _WIN32
    dllTlsf->Load("./TLSFAllocator.dll");
#elif defined(__APPLE__)
    dllTlsf->Load("./libTLSFAllocator.dylib");
#else
    dllTlsf->Load("./libTLSFAllocator.so");
#endif
}

/**
 @brief �v���O�����I�����Ƀ��C�u�������J������
 */
void Terminate()
{
    dllTlsf->Reset();
}

/**
 @brief �������A���P�[�^
 */
class TLSFAllocator
{
private:
    /**
     @brief ���g�̃|�C���^
     */
    void* selfPtr = nullptr;

    static void* dll_TLSFAllocator_Constructor_0(void* ptr, size_t size, size_t split)
    {
        typedef void* (*proc_t)(void*, size_t, size_t);
        static proc_t proc = dllTlsf->GetProc<proc_t>("dll_TLSFAllocator_Constructor_0");
        return proc(ptr, size, split);
    }

    static void* dll_TLSFAllocator_Constructor_1(size_t size, size_t split)
    {
        typedef void* (*proc_t)(size_t, size_t);
        static proc_t proc = dllTlsf->GetProc<proc_t>("dll_TLSFAllocator_Constructor_1");
        return proc(size, split);
    }

    static void  dll_TLSFAllocator_Destructor(void* selfPtr)
    {
        typedef void (*proc_t)(void*);
        static proc_t proc = dllTlsf->GetProc<proc_t>(" dll_TLSFAllocator_Destructor");
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
     @brief �R���X�g���N�^
     @param ptr �����������̈�̐擪�A�h���X
     @param size �����������̈�̃T�C�Y
     @param split ��2�J�e�S���̕�����
     */
    TLSFAllocator(void* ptr, size_t size, size_t split = 8)
    {
        selfPtr = dll_TLSFAllocator_Constructor_0(ptr, size, split);
    }

    /**
     @brief �R���X�g���N�^
     @param size �����������̈�̃T�C�Y
     @param split ��2�J�e�S���̕�����
     */
    TLSFAllocator(size_t size, size_t split = 8)
    {
        selfPtr = dll_TLSFAllocator_Constructor_1(size, split);
    }

    /**
     @brief �f�X�g���N�^
     */
    ~TLSFAllocator()
    {
        dll_TLSFAllocator_Destructor(selfPtr);
    }

    /**
     @brief �̈���m�ۂ���
     @param size �m�ۂ���̈�̃T�C�Y
     @return �m�ۂ����̈�̐擪�A�h���X(�u���b�N�S�̂̃T�C�Y�ł͂Ȃ�)
     */
    void* Alloc(const size_t size)
    {
        return dll_TLSFAllocator_Alloc(selfPtr, size);
    }

    /**
     @brief �m�ۂ����̈���������
     @param ptr �������̈�̐擪�A�h���X(�u���b�N�S�̂̃T�C�Y�ł͂Ȃ�)S
     */
    void Free(void* ptr)
    {
        dll_TLSFAllocator_Free(selfPtr, ptr);
    }

    /**
     @brief ���݂̃������̓��e���v�����g����
     */
    void PrintBufferInfo()
    {
        dll_TLSFAllocator_PrintDebugInfo(selfPtr);
    }
};