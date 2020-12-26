#pragma once

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <tuple>

#include "BlockHeader.h"
#include "Helper.h"


namespace TLSFAllocator
{
    /**
     @brief フリーブロックリスト
     */
    class FreeBlockList
    {
    private:

        /**
         @brief フリーブロックリスト
         */
        FreeBlockHeader* m_FreeBlockList[4096];

        /**
         @brief ブロックが各第1カテゴリに所属しているかのフラグ
         */
        uint64_t m_FLIFlagList;

        /**
         @brief ブロックが各第2カテゴリに所属しているかのフラグ
         */
        uint64_t m_SLIFlagList[64];

    public:

        /**
         @brief コンストラクタ
         */
        FreeBlockList();

        /**
         @brief フリーブロックリストに登録する
         @param header フリーブロックリストに登録するメモリブロックのヘッダ
         */
        void Append(FreeBlockHeader* header);

        /**
         @brief フリーブロックリストから削除する
        @param header フリーブロックリストから削除するメモリブロックのヘッダ
        */
        void Remove(FreeBlockHeader* header);

        /**
         @brief 確保する領域の大きさに最適なフリーブロックを取得する
         @param size 確保する領域の大きさ
         @return 確保する領域の大きさに最適なフリーブロック
         */
        FreeBlockHeader* Get(size_t size);

    };
}