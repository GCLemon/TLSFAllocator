#include "FreeBlockList.h"

namespace TLSFAllocator
{
    FreeBlockList::FreeBlockList()
    {
        // フリーブロックリストの初期化
        memset(m_FreeBlockList, 0, sizeof(FreeBlockHeader*) * 4096);

        // フラグリストの初期化
        m_FLIFlagList = 0;
        memset(m_SLIFlagList, 0, sizeof(uint64_t) * 64);
    }

    void FreeBlockList::Append(FreeBlockHeader* header)
    {
        // 指定されたメモリブロックは占有されていてはならない
        assert(!header->GetIsOccupied());

        // 連結リストを構成するために必要な領域がなければ処理を終える
        if(header->GetBlockSize() + sizeof(BlockHeader) < sizeof(FreeBlockHeader)) return;

        // 前後のアドレスにnullを代入
        header->m_NextBlock = nullptr;
        header->m_PrevBlock = nullptr;

        // カテゴリを取得
        auto category = GetCategory(header->GetBlockSize());
        uint8_t fli = std::get<0>(category);
        uint8_t sli = std::get<1>(category);

        // カテゴリを一つ下げる
        if (sli) --sli; else { sli = 63; --fli; }

        // フラグを立てる
        m_FLIFlagList |= 1ULL << fli;
        m_SLIFlagList[fli] |= 1ULL << sli;

        // フリーブロックリストに追加
        header->m_NextBlock = m_FreeBlockList[fli * 64 + sli];

        if (m_FreeBlockList[fli * 64 + sli])
            m_FreeBlockList[fli * 64 + sli]->m_PrevBlock = header;

        m_FreeBlockList[fli * 64 + sli] = header;
    }

    void FreeBlockList::Remove(FreeBlockHeader* header)
    {
        // 指定されたメモリブロックは占有されていてはならない
        assert(!header->GetIsOccupied());

        // カテゴリを取得
        auto category = GetCategory(header->GetBlockSize());
        uint8_t fli = std::get<0>(category);
        uint8_t sli = std::get<1>(category);

        // カテゴリを一つ下げる
        if (sli) --sli; else { sli = 63; --fli; }

        // ブロック間の接続を切る
        if (header->m_NextBlock != nullptr) header->m_NextBlock->m_PrevBlock = header->m_PrevBlock;
        if (header->m_PrevBlock != nullptr) header->m_PrevBlock->m_NextBlock = header->m_NextBlock;

        // 処理中のメモリブロックがフリーブロックリストの先頭だった場合
        if (header == m_FreeBlockList[fli * 64 + sli])
        {
            // 次のメモリブロックをフリーブロックリストとする
            m_FreeBlockList[fli * 64 + sli] = header->m_NextBlock;

            // その結果nullptrとなった場合、フラグを下す
            if (m_FreeBlockList[fli * 64 + sli] == nullptr)
            {
                m_SLIFlagList[fli] &= ~(1ULL << sli);
                if (!m_SLIFlagList[fli]) m_FLIFlagList &= ~(1ULL << fli);
            }
        }

        // 前後のアドレスにnullを代入
        header->m_NextBlock = nullptr;
        header->m_PrevBlock = nullptr;
    }

    FreeBlockHeader* FreeBlockList::Get(size_t size)
    {
        // カテゴリを取得
        auto categories = GetCategory(size);
        size_t fli = std::get<0>(categories);
        size_t sli = std::get<1>(categories);

        // フリーブロックリストに登録があった場合、登録されているブロックを返す
        if (m_FreeBlockList[fli * 64 + sli] != nullptr)
        {
            return m_FreeBlockList[fli * 64 + sli];
        }
        
        // より大きな第2カテゴリを探す
        size_t sliFlag = m_SLIFlagList[fli];
        size_t enableList = sliFlag & (ULLONG_MAX << sli);

        // 存在した場合、登録されているブロックを返す
        if (enableList)
        {
            sli = LeastSignificantBit(enableList);
            return m_FreeBlockList[fli * 64 + sli];
        }

        // より大きな第1カテゴリを探す
        enableList = m_FLIFlagList & (ULLONG_MAX << (fli + 1));

        // 存在した場合、登録されているブロックを返す
        if (enableList)
        {
            fli = LeastSignificantBit(enableList);
            sli = LeastSignificantBit(m_SLIFlagList[fli]);
            return m_FreeBlockList[fli * 64 + sli];
        }

        // それでも存在しなかった場合、確保失敗
        return nullptr;
    }
}