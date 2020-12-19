#pragma once

#include <climits>
#include <cmath>
#include <cstring>
#include <mutex>
#include <tuple>

#include "TLSFAlgoHelper.h"

namespace tlsf
{
    /**
     @brief TLSFアルゴリズムによるメモリアロケータ
    */
    class TLSFAllocator
    {
    private:

        /**
         @brief スレッド排他制御用のインスタンス
        */
        std::mutex m_mutex;

        /**
         @brief 管理中の領域の先頭アドレス
        */
        uint8_t* m_buffer;

        /**
         @brief 管理中の領域の大きさ
        */
        size_t m_bufferSize;

        /**
         @brief フリーブロックリスト
        */
        FreeBlockHeader* m_freeBlockList[MAX_FREELIST];

        /**
         @brief ブロックが各第1カテゴリに所属しているかのフラグ
        */
        size_t  m_fliFlagList;

        /**
         @brief ブロックが各第2カテゴリに所属しているかのフラグ
        */
        size_t m_sliFlagList[MAX_FLI_CATEGORY];

        /**
         @brief 第2カテゴリの分割数
        */
        size_t m_split;

        /**
         @brief 第2カテゴリの分割数(Logスケール)
        */
        size_t m_splitLog;

        /**
         @brief コンストラクタでメモリ確保が行われたか
        */
        bool m_isBufferInternal;

        /**
         @brief メモリ操作をスレッドセーフにするか
        */ 
        bool m_isThreadSafe;

        /**
         @brief 値が1である最上位ビットを取得する
        @param x ビット列
        */
        inline size_t MostSignificantBit(size_t x);

        /**
         @brief 値が1である最下位ビットを取得する
        @param x ビット列
        */
        inline size_t LeastSignificantBit(size_t x);

        /**
         @brief メモリブロックのカテゴリを取得する
        @param size 確保する領域のサイズ
        @param split 第2カテゴリの分割数
        @return カテゴリ情報を表すタプル
        */
        inline std::tuple<size_t, size_t> GetCategories(size_t size);

        /**
         @brief アロケータを初期化する
        @param ptr 初期化した領域の先頭アドレス
        @param size 初期化した領域のサイズ
        @param split 第2カテゴリの分割数
        @param isBufferInternal 
        */
        void Initialize(uint8_t* ptr, size_t size, size_t split);

        /**
         @brief メモリ領域を2つに分割する
        @param header 分割するメモリブロックのヘッダ
        @param total_size 切り出すメモリブロック全体のサイズ
        @return 分割したメモリ領域の先頭管理タグ
        */
        std::tuple<FreeBlockHeader*, FreeBlockHeader*> DevideMemory(FreeBlockHeader* header, size_t total_size);

        /**
         @brief 指定したメモリ領域の両隣を結合する
        @param header 結合するメモリブロックのヘッダ
        @param total_size メモリブロック全体のサイズ
        @return 結合したメモリ領域の先頭管理タグ
        */
        FreeBlockHeader* MergeMemory(FreeBlockHeader* header, size_t total_size);

        /**
         @brief フリーブロックリストに登録する
        @param header フリーブロックリストに登録するメモリブロックのヘッダ
        @param total_size メモリブロック全体のサイズ
        */
        void AddToFreeList(FreeBlockHeader* header, size_t total_size);

        /**
         @brief フリーブロックリストから削除する
        @param header フリーブロックリストから削除するメモリブロックのヘッダ
        @param total_size メモリブロック全体のサイズ
        */
        void DeleteFromFreeList(FreeBlockHeader* header, size_t total_sizee);

    public:

        /**
         @brief コンストラクタ
        @param ptr 初期化した領域の先頭アドレス
        @param size 初期化した領域のサイズ
        @param split 第2カテゴリの分割数
        */
        TLSFAllocator(void* ptr, size_t size, size_t split);

        /**
         @brief コンストラクタ
        @param size 初期化した領域のサイズ
        @param split 第2カテゴリの分割数
        */
        TLSFAllocator(size_t size, size_t split);

        /**
         @brief デストラクタ
        */
        ~TLSFAllocator();

        /**
         @brief このメモリアロケータがスレッドセーフであるかを取得する
        */
        bool GetIsThreadSafe() { return m_isThreadSafe; }

        /**
         @brief このメモリアロケータをスレッドセーフにするかを設定する
        */
        void SetIsTrehadSafe(bool isThreadSafe) { m_isThreadSafe = isThreadSafe; }

        /**
         @brief 領域を確保する
        @param size 確保する領域のサイズ
        @return 確保した領域の先頭アドレス(ブロック全体のサイズではない)
        */
        void* Alloc(const size_t size);

        /**
         @brief 確保した領域を解放する
        @param ptr 解放する領域の先頭アドレス(ブロック全体のサイズではない)
        */
        void Free(void* ptr);

        /**
         @brief デバッグをプリントする
        */
        void PrintDebugInfo();
    };
}