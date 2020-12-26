#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <tuple>

#include <assert.h>
#include <string.h>

#include "Helper.h"
#include "BlockHeader.h"
#include "BlockFooter.h"
#include "FreeBlockList.h"

namespace TLSFAllocator
{
class TLSFAllocator
{
    private:

        /**
         @brief アロケータの唯一インスタンス
         */
        static TLSFAllocator* m_Instance;

        /**
         @brief 排他処理用のロック
         */
        std::mutex m_Mutex;

        /**
         @brief アロケータによって提供される領域
         */
        const uint8_t* m_Buffer;

        /**
         @brief アロケータによって提供される領域の大きさ
         */
        const uint64_t m_BufferSize;

        /**
         @brief フリーブロックリスト
         */
        FreeBlockList m_FreeBlockList;

        /**
         @brief MonitorThread が有効か
         */
        const bool m_IsMonitorMode;

        /**
         @brief MonitorThread が有効か
         */
        bool m_IsMonitoring;

        /**
         @brief モニタリング用のスレッド
         */
        std::thread m_MonitorThread;

        /**
         @brief プライベートコンストラクタ
         */
        TLSFAllocator(uint64_t bufferSize, bool isMonitorMode);

        /**
         @brief プライベートデストラクタ
         */
        ~TLSFAllocator();

        /**
         @brief 指定子たメモリ領域から決められたサイズを切り出す
         @param header 分割するメモリブロックのヘッダ
         @param size 切り出す管理領域のサイズ
         */
        void DevideMemory(FreeBlockHeader* header, size_t size);

        /**
         @brief 指定したメモリ領域とその直後にあるメモリ領域を結合する
         @param header1 結合対象のメモリブロックのヘッダ
         @param header2 結合対象のメモリブロックのヘッダ
         */
        void MergeMemory(FreeBlockHeader* header1, FreeBlockHeader* header2);

        /**
         @brief メモリ領域を監視するスレッド
         */
        static void MonitorThread(TLSFAllocator* ptr);

    public:

        /**
         @brief メモリ領域を確保する
         @param count 格納するインスタンスの個数
         @param size 格納するインスタンスの大きさ
         @return 確保した領域の先頭アドレス
         */
        void* Malloc(size_t count, size_t size);

        /**
         @brief メモリ領域を確保し、0で初期化する
         @param count 格納するインスタンスの個数
         @param size 格納するインスタンスの大きさ
         @return 確保した領域の先頭アドレス
         */
        void* Calloc(size_t count, size_t size);

        /**
         @brief メモリ領域を再確保する
         @param address 再確保元のアドレス nullptrならばMallocに同じ
         @param count 格納するインスタンスの個数
         @param size 格納するインスタンスの大きさ
         @return 確保した領域の先頭アドレス
         */
        void* Realloc(void* address, size_t count, size_t size);

        /**
         @brief メモリ領域を解放する
         @param address 解放する領域のアドレス
         */
        void Free(void* address);

        /**
         @brief TLSFAllocatorクラスのインスタンスを取得する
         */
        static TLSFAllocator* GetInstance();

        /**
         @brief メモリアロケータの初期化を行う
         */
        static bool Initialize(uint64_t bufferSize, bool isMonitorMode);

        /**
         @brief メモリアロケータの終了処理を行う
         */
        static bool Terminate();

    };
}