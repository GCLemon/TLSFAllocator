#include "TLSFAllocator.h"

namespace TLSFAllocator
{
    static size_t HEADER_SIZE = sizeof(BlockHeader);
    static size_t FOOTER_SIZE = sizeof(BlockFooter);
    static size_t TAG_SIZE = sizeof(BlockHeader) + sizeof(BlockFooter);

    static size_t FREE_HEADER_SIZE = sizeof(FreeBlockHeader);
    static size_t FREE_TAG_SIZE = sizeof(FreeBlockHeader) + sizeof(BlockFooter);

    TLSFAllocator* TLSFAllocator::m_Instance = nullptr;

    TLSFAllocator::TLSFAllocator(uint64_t bufferSize, bool isMonitorMode)
        : m_Buffer(new uint8_t[bufferSize]), m_BufferSize(bufferSize), m_IsMonitorMode(isMonitorMode)
    {
        /*** メモリ領域の初期化 ***/

        // 先頭管理ヘッダの設定
        FreeBlockHeader* header = new((void*)m_Buffer) FreeBlockHeader();
        header->SetIsOccupied(false);
        header->SetBlockSize(m_BufferSize - TAG_SIZE);
        header->m_PrevBlock = nullptr;
        header->m_NextBlock = nullptr;

        // 末端管理ヘッダの設定
        BlockFooter* footer = new((void*)(m_Buffer + m_BufferSize - FOOTER_SIZE)) BlockFooter();
        footer->SetRegionSize(m_BufferSize);

        // メモリブロックをフリーブロックリストに登録
        m_FreeBlockList.Append(header);

        /*** モニタリング用のスレッドの起動 ***/

        // モニタリング用のスレッドを起動する
        if(m_IsMonitorMode)
        {
            m_IsMonitoring = true;
            m_MonitorThread = std::thread(MonitorThread, this);
        }
    }

    TLSFAllocator::~TLSFAllocator()
    {
        /*** モニタリング用のスレッドの終了 ***/

        // モニタリング用のスレッドを終了する
        m_IsMonitoring = false;
        m_MonitorThread.join();

        /*** メモリ領域の終了処理 ***/

        // メモリ領域を解放する
        delete[] m_Buffer;
    }

    void TLSFAllocator::DevideMemory(FreeBlockHeader* header, size_t size)
    {
        // 指定されたメモリブロックは占有されていてはならない
        assert(!header->GetIsOccupied());

        // フリーブロックリストから削除する
        m_FreeBlockList.Remove(header);

        // 十分な領域がなかった場合は処理を終える
        if(header->GetBlockSize() < size + TAG_SIZE) return;

        // 余った管理領域のサイズを求める
        size_t restSize = header->GetBlockSize() - TAG_SIZE - size;

        // 1つ目の先頭管理タグ・末尾管理タグの書き換え
        header->SetBlockSize(size);
        BlockFooter* footer = (BlockFooter*)((uint8_t*)header + HEADER_SIZE + size);
        footer->SetRegionSize(size + TAG_SIZE);

        // 2つ目の先頭管理タグ・末尾管理タグの書き換え
        BlockHeader* nextHeader = (BlockHeader*)((uint8_t*)header + size + TAG_SIZE);
        nextHeader->SetBlockSize(restSize);
        nextHeader->SetIsOccupied(false);
        BlockFooter* nextFooter = (BlockFooter*)((uint8_t*)nextHeader + HEADER_SIZE + restSize);
        nextFooter->SetRegionSize(restSize + TAG_SIZE);

        // フリーブロックリストに追加する
        m_FreeBlockList.Append((FreeBlockHeader*)nextHeader);
    }

    void TLSFAllocator::MergeMemory(FreeBlockHeader* header1, FreeBlockHeader* header2)
    {
        // 指定されたメモリブロックは占有されていてはならない
        assert(!header1->GetIsOccupied());
        assert(!header2->GetIsOccupied());

        // 指定されたメモリブロックは隣り合っていなくてはならない
        assert((uint8_t*)header1 + header1->GetBlockSize() + TAG_SIZE == (uint8_t*)header2);

        // 新たな先頭・末尾管理タグを取得
        BlockHeader* header = header1;
        BlockFooter* footer = (BlockFooter*)((uint8_t*)header2 + HEADER_SIZE + header2->GetBlockSize());

        // ブロックサイズを計算
        size_t blockSize = header1->GetBlockSize() + header2->GetBlockSize() + TAG_SIZE;
        size_t regionSize = blockSize + TAG_SIZE;

        // 計算した値を設定
        header->SetBlockSize(blockSize);
        footer->SetRegionSize(regionSize);
    }

    void TLSFAllocator::MonitorThread(TLSFAllocator* ptr)
    {
        while(ptr->m_IsMonitoring)
        {
            // モニタリング情報を出力する
            {
                std::lock_guard<std::mutex> lock(ptr->m_Mutex);
            }

            // 1秒待つ
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void* TLSFAllocator::Malloc(size_t count, size_t size)
    {
        // 確保領域のバイト数を算出
        size *= count;

        // ロックを取る
        std::lock_guard<std::mutex> lock(m_Mutex);

        // フリーブロックを取得
        FreeBlockHeader* header = m_FreeBlockList.Get(size);

        // メモリを分割する
        DevideMemory(header, size);

        // 分割したメモリブロックを占有状態にする
        header->SetIsOccupied(true);

        // 確保領域の先頭を返す
        return (uint8_t*)header + HEADER_SIZE;
    }

    void* TLSFAllocator::Calloc(size_t count, size_t size)
    {
        // 確保領域のバイト数を算出
        size *= count;

        // ロックを取る
        std::lock_guard<std::mutex> lock(m_Mutex);

        // フリーブロックを取得
        FreeBlockHeader* header = m_FreeBlockList.Get(size);

        // メモリを分割する
        DevideMemory(header, size);

        // 分割したメモリブロックを占有状態にする
        header->SetIsOccupied(true);

        // 確保領域をゼロ初期化する
        memset((uint8_t*)header + HEADER_SIZE, 0, header->GetBlockSize());

        // 確保領域の先頭を返す
        return (uint8_t*)header + HEADER_SIZE;
    }

    void* TLSFAllocator::Realloc(void* address, size_t count, size_t size)
    {
        // ロックを取る
        std::lock_guard<std::mutex> lock(m_Mutex);
        return nullptr;
    }

    void TLSFAllocator::Free(void* address)
    {
        // ロックを取る
        std::lock_guard<std::mutex> lock(m_Mutex);

        // nullptrを渡されたら何もしない
        if (address == nullptr) return;

        // ヘッダ位置にポインタを移動
        BlockHeader* header = (BlockHeader*)((uint8_t*)address - HEADER_SIZE);

        // 占有フラグを下す
        header->SetIsOccupied(false);

        // 直後のメモリ領域と結合する
        if((uint8_t*)header + header->GetBlockSize() + TAG_SIZE < m_Buffer + m_BufferSize)
        {
            BlockHeader* nextHeader = (BlockHeader*)((uint8_t*)header + header->GetBlockSize() + TAG_SIZE);
            if(!nextHeader->GetIsOccupied())
            {
                if(nextHeader->GetBlockSize() + HEADER_SIZE >= FREE_HEADER_SIZE)
                {
                    m_FreeBlockList.Remove((FreeBlockHeader*)nextHeader);
                }
                MergeMemory((FreeBlockHeader*)header, (FreeBlockHeader*)nextHeader);
            }
        }

        // 直前のメモリ領域と結合する
        if((uint8_t*)header - FOOTER_SIZE >= m_Buffer)
        {
            BlockFooter* prevFooter = (BlockFooter*)((uint8_t*)header - FOOTER_SIZE);
            BlockHeader* prevHeader = (BlockHeader*)((uint8_t*)header - prevFooter->GetRegionSize());
            if(!prevHeader->GetIsOccupied())
            {
                if(prevHeader->GetBlockSize() + HEADER_SIZE >= FREE_HEADER_SIZE)
                {
                    m_FreeBlockList.Remove((FreeBlockHeader*)prevHeader);
                }
                MergeMemory((FreeBlockHeader*)prevHeader, (FreeBlockHeader*)header);
                header = prevHeader;
            }
        }

        // フリーブロックリストに追加する
        m_FreeBlockList.Append((FreeBlockHeader*)header);
    }

    TLSFAllocator* TLSFAllocator::GetInstance()
    {
        return m_Instance;
    }

    bool TLSFAllocator::Initialize(uint64_t bufferSize, bool isMonitorMode)
    {
        m_Instance = new TLSFAllocator(bufferSize, isMonitorMode);
        return m_Instance != nullptr;
    }

    bool TLSFAllocator::Terminate()
    {
        delete m_Instance;
        return true;
    }
}