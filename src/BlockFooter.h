#pragma once

#include <stdint.h>

namespace TLSFAllocator
{
    /**
     @brief 末尾管理ヘッダ
     */
    class BlockFooter
    {
    private:

        /**
         @brief ブロック全体のサイズを管理するフラグ
         */
        uint64_t m_Flags;

    public:

        /**
         @brief このブロック全体のサイズを取得する
         */
        uint64_t GetRegionSize() { return m_Flags; }

        /**
         @brief このブロック全体のサイズを設定する
         */
        void SetRegionSize(uint64_t blockSize) { m_Flags = blockSize; }
        
    };
}