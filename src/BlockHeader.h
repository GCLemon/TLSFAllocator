#pragma once

#include <stdint.h>

namespace TLSFAllocator
{
    /**
     @brief 先頭管理ヘッダ
     */
    class BlockHeader
    {
    private:

        /**
         @brief ブロックの管理領域サイズと占有フラグを管理するフラグ
         */
        int64_t m_Flags;

    public:

        /**
         @brief このブロックが使用中であるかどうかを取得する
         */
        bool GetIsOccupied() { return m_Flags & 0x8000000000000000; }

        /**
         @brief このブロックが使用中であるかどうかを設定する
         */
        void SetIsOccupied(bool isOccupied)
        {
            if (isOccupied) m_Flags |= 0x8000000000000000;
            else m_Flags &= 0x7fffffffffffffff;
        };

        /**
         @brief このブロックの管理領域のサイズを取得する
         */
        uint64_t GetBlockSize() { return m_Flags & 0x7fffffffffffffff; }

        /**
         @brief このブロックの管理領域のサイズを設定する
         */
        void SetBlockSize(uint64_t blockSize)
        {
            uint64_t size = blockSize & 0x7fffffffffffffff;
            uint64_t isOccupied = m_Flags & 0x8000000000000000;
            m_Flags = size | isOccupied;
        };
        
    };

    /**
     @brief 未割当ブロックに使用する先頭管理ヘッダ
     */
    class FreeBlockHeader : public BlockHeader
    {
    public:

        /**
         @brief 自身が所属しているフリーブロックリストについて直後に登録されているブロックの先頭管理ヘッダ
         */
        FreeBlockHeader* m_PrevBlock;

        /**
         @brief 自身が所属しているフリーブロックリストについて直後に登録されているブロックの先頭管理ヘッダ
         */
        FreeBlockHeader* m_NextBlock;

    };
}