#include "TLSFAllocator.h"

TLSFAllocator::TLSFAllocator(void* ptr, size_t size, size_t split)
{
    Initialize((uint8_t*)ptr, size, split);
}

TLSFAllocator::TLSFAllocator(size_t size, size_t split)
{
    Initialize(nullptr, size, split);
}

TLSFAllocator::~TLSFAllocator()
{
    if (m_isBufferInternal) delete[] m_buffer;
}

inline size_t TLSFAllocator::MostSignificantBit(size_t x)
{
    size_t msb = 63;

    if (!(x & 0xffffffff00000000)) { x <<= 32; msb -= 32; }
    if (!(x & 0xffff000000000000)) { x <<= 16; msb -= 16; }
    if (!(x & 0xff00000000000000)) { x <<= 8; msb -= 8; }
    if (!(x & 0xf000000000000000)) { x <<= 4; msb -= 4; }
    if (!(x & 0xc000000000000000)) { x <<= 2; msb -= 2; }
    if (!(x & 0x8000000000000000)) { x <<= 1; msb -= 1; }

    return msb;
}

inline size_t TLSFAllocator::LeastSignificantBit(size_t x)
{
    size_t lsb = 0; x &= ~x + 1;

    if (x & 0xffffffff00000000) lsb += 32;
    if (x & 0xffff0000ffff0000) lsb += 16;
    if (x & 0xff00ff00ff00ff00) lsb += 8;
    if (x & 0xf0f0f0f0f0f0f0f0) lsb += 4;
    if (x & 0xcccccccccccccccc) lsb += 2;
    if (x & 0xaaaaaaaaaaaaaaaa) lsb += 1;

    return lsb;
}

void TLSFAllocator::Initialize(uint8_t* ptr, size_t size, size_t split)
{
    // 諸々初期化
    if (ptr != nullptr)
    {
        m_buffer = ptr;
        m_isBufferInternal = false;
    }
    else
    {
        m_buffer = new uint8_t[size];
        m_isBufferInternal = true;
    }
    m_bufferSize = size;
    m_split = split;
    m_splitLog = log2l(split);
    m_isThreadSafe = false;

    // フリーブロックリストを初期化
    memset(m_freeBlockList, 0, sizeof(FreeBlockHeader*) * MAX_FREELIST);

    // カテゴリリストを初期化
    m_fliFlagList = 0;
    memset(m_sliFlagList, 0, sizeof(size_t) * 64);

    // 先頭管理タグを初期化し、フリーブロックリストに代入する
    FreeBlockHeader* header = (FreeBlockHeader*)m_buffer;
    header->SetBlockSize(size - TAG_SIZE);
    header->SetIsOccupied(false);
    AddToFreeList(header, size);

    // 末端管理タグを初期化
    size_t* tailer = (size_t*)((uint8_t*)ptr + size - sizeof(size_t));
    *tailer = size;
}

std::tuple<size_t, size_t> TLSFAllocator::GetCategories(size_t size)
{
    // 第1カテゴリのインデックスを求める
    size_t fli = MostSignificantBit(size);

    // 第2カテゴリのインデックスを求める
    size_t mask = (1ULL << fli) - 1ULL;
    size_t shift = fli - m_splitLog;
    size_t sli = (size & mask) >> shift;

    // カテゴリを返す
    return std::make_tuple(fli, sli);
}

std::tuple<FreeBlockHeader*, FreeBlockHeader*>  TLSFAllocator::DevideMemory(FreeBlockHeader* header, size_t size)
{
    // メモリブロックが使用中ならば例外を投げる
    if (header->GetIsOccupied())
        throw "[Exception] This memory block is already occupied.";

    // 分割後のメモリブロックに管理情報を書き込むスペースがなかった場合、分割失敗
    if (header->GetBlockSize() <= size)
        throw "[Exception] There is no enough space to write block header.";

    // 余ったメモリブロックのサイズを求める
    size_t rest_size = header->GetBlockSize() + TAG_SIZE - size;

    // 1つ目の先頭管理タグ・末端管理タグの書き換え
    header->SetBlockSize(size - TAG_SIZE);
    size_t* tailer = (size_t*)((uint8_t*)header + size - sizeof(size_t));
    *tailer = size;

    // 2つ目の先頭管理タグ・末端管理タグの書き換え
    FreeBlockHeader* next_header = (FreeBlockHeader*)((uint8_t*)header + size);
    next_header->SetBlockSize(rest_size - TAG_SIZE);
    next_header->SetIsOccupied(false);
    size_t* next_tailer = (size_t*)((uint8_t*)next_header + rest_size - sizeof(size_t));
    *next_tailer = rest_size;

    // 分割したメモリ領域の先頭管理タグを返す
    return std::make_tuple(header, next_header);
}

FreeBlockHeader* TLSFAllocator::MergeMemory(FreeBlockHeader* header, size_t size)
{
    // メモリブロックが使用中ならば例外を投げる
    if (header->GetIsOccupied())
        throw "[Exception] This memory block is already occupied.";

    // 末端管理タグを取得
    size_t* tailer = (size_t*)((uint8_t*)header + size - sizeof(size_t));

    if ((uint8_t*)header + size != m_buffer + m_bufferSize)
    {
        // 次の領域を結合
        FreeBlockHeader* next_header = (FreeBlockHeader*)((uint8_t*)header + size);
        size_t* next_tailer = (size_t*)((uint8_t*)next_header + next_header->GetBlockSize() + sizeof(BlockHeader));
        if (!next_header->GetIsOccupied())
        {
            // フリーブロックリストから削除する
            if(next_header->GetBlockSize() >= TAG_SIZE)
                DeleteFromFreeList(next_header, *next_tailer);

            // 先頭管理タグの情報を書き換える
            size_t new_block_size = header->GetBlockSize() + next_header->GetBlockSize() + TAG_SIZE;
            header->SetBlockSize(new_block_size);
            header->SetIsOccupied(false);

            // 末端管理タグの情報を書き換える
            *next_tailer = new_block_size + TAG_SIZE;

            // 結合後の末端管理タグのアドレスを代入
            tailer = next_tailer;
        }
    }

    if ((uint8_t*)header != m_buffer)
    {
        // 前の領域を結合
        size = *(size_t*)((uint8_t*)header - sizeof(size_t));
        FreeBlockHeader* prev_header = (FreeBlockHeader*)((uint8_t*)header - size);
        size_t* prev_tailer = (size_t*)((uint8_t*)header - sizeof(size_t));
        if (!prev_header->GetIsOccupied())
        {
            // フリーブロックリストから削除する
            if (prev_header->GetBlockSize() >= TAG_SIZE)
                DeleteFromFreeList(prev_header, *prev_tailer);

            // 先頭管理タグの情報を書き換える
            size_t new_block_size = prev_header->GetBlockSize() + header->GetBlockSize() + TAG_SIZE;
            prev_header->SetBlockSize(new_block_size);
            prev_header->SetIsOccupied(false);

            // 末端管理タグの情報を書き換える
            *tailer = new_block_size + TAG_SIZE;

            // 結合後の先頭管理タグのアドレスを代入
            header = prev_header;
        }
    }

    // 結合したメモリ領域の先頭管理タグを返す
    return header;
}

void TLSFAllocator::AddToFreeList(FreeBlockHeader* header, size_t size)
{
    // メモリブロックが使用中ならば例外を投げる
    if (header->GetIsOccupied())
        throw "[Exception] This memory block is already occupied.";

    // フリーブロックリストの接続状況の初期化
    header->prevBlock = nullptr;
    header->nextBlock = nullptr;

    // メモリブロックのカテゴリを取得する
    auto categories = GetCategories(size);
    size_t fli = std::get<0>(categories);
    size_t sli = std::get<1>(categories);

    // カテゴリを一つ下げる
    if (sli) --sli; else if (fli > m_splitLog) { sli = m_split - 1; --fli; }

    // フラグを立てる
    m_fliFlagList |= 1ULL << fli;
    m_sliFlagList[fli] |= 1ULL << sli;

    // 既にリストがあった場合はリストに挿入する
    if (m_freeBlockList[fli * m_split + sli])
    {
        FreeBlockHeader* next_header = m_freeBlockList[fli * m_split + sli];
        header->nextBlock = next_header;
        next_header->prevBlock = header;
    }

    // フリーブロックリストに追加
    m_freeBlockList[fli * m_split + sli] = header;
}

void TLSFAllocator::DeleteFromFreeList(FreeBlockHeader* header, size_t size)
{
    // メモリブロックが使用中ならば例外を投げる
    if (header->GetIsOccupied())
        throw "[Exception] This memory block is already occupied.";

    if (header->GetBlockSize() < TAG_SIZE)
        throw "[Exception] This memory block is not expected to be in list.";

    // メモリブロックのカテゴリを取得する
    auto categories = GetCategories(size);
    size_t fli = std::get<0>(categories);
    size_t sli = std::get<1>(categories);

    // カテゴリを一つ下げる
    if (sli) --sli; else if (fli > m_splitLog) { sli = m_split - 1; --fli; }

    // ブロック間の接続を切る
    if (header->nextBlock != nullptr)
        header->nextBlock->prevBlock = header->prevBlock;
    if (header->prevBlock != nullptr)
        header->prevBlock->nextBlock = header->nextBlock;

    // 処理中のメモリブロックがフリーブロックリストの先頭だった場合
    // または直前のリストがnullptrだった場合
    if (header == m_freeBlockList[fli * m_split + sli] || header->prevBlock == nullptr)
    {
        // 次のメモリブロックをフリーブロックリストとする
        m_freeBlockList[fli * m_split + sli] = header->nextBlock;

        // その結果nullptrとなった場合、フラグを下す
        if (m_freeBlockList[fli * m_split + sli] == nullptr)
        {
            m_sliFlagList[fli] &= ~(1 << sli);
            if (!m_sliFlagList[fli])
                m_fliFlagList &= ~(1 << fli);
        }
    }
}

void* TLSFAllocator::Alloc(const size_t size)
{
    // ロックをとる
    if (m_isThreadSafe) std::lock_guard<std::mutex> lock(m_mutex);

    // 0byteの確保では何もしない
    if (!size) return nullptr;

    // メモリブロックのカテゴリを取得する
    auto categories = GetCategories(size + TAG_SIZE);
    size_t fli = std::get<0>(categories);
    size_t sli = std::get<1>(categories);

    // フリーブロックリストに登録があった場合
    if (m_freeBlockList[fli * m_split + sli] != nullptr)
    {
        // メモリブロックのヘッダを取得する
        FreeBlockHeader* header = m_freeBlockList[fli * m_split + sli];

        // フリーブロックリストに登録されている可能性がある場合、削除する
        if (header->GetBlockSize() >= TAG_SIZE)
            DeleteFromFreeList(header, header->GetBlockSize() + TAG_SIZE);

        // 占有フラグを立てる
        header->SetIsOccupied(true);

        // メモリブロックのヘッダを返す
        return (uint8_t*)header + sizeof(BlockHeader);
    }

    // そうでない場合
    else
    {
        // より大きな第2カテゴリを探す
        size_t sli_flag = m_sliFlagList[fli];
        size_t enable_list = sli_flag & (ULLONG_MAX << sli);

        // 存在した場合
        if (enable_list)
        {
            // メモリブロックのヘッダを取得する
            sli = LeastSignificantBit(enable_list);
            FreeBlockHeader* header = m_freeBlockList[fli * m_split + sli];

            // フリーブロックリストに登録されている可能性がある場合、削除する
            if (header->GetBlockSize() >= TAG_SIZE)
                DeleteFromFreeList(header, header->GetBlockSize() + TAG_SIZE);

            // 分割するための領域が十分にある場合、メモリ領域を分割する
            if(header->GetBlockSize() > size + TAG_SIZE)
            {
                auto headers = DevideMemory((FreeBlockHeader*)header, size + TAG_SIZE);
                if (header != std::get<0>(headers))
                    throw "[Exception] Unexpected return value.";
                FreeBlockHeader* new_header = std::get<1>(headers);

                // 分割後の領域に十分なスペースがあった場合、フリーブロックリストに登録する
                if (new_header->GetBlockSize() + TAG_SIZE >= FREE_TAG_SIZE)
                    AddToFreeList(new_header, new_header->GetBlockSize() + TAG_SIZE);
            }

            // 占有フラグを立てる
            header->SetIsOccupied(true);

            // メモリブロックのヘッダを返す
            return (uint8_t*)header + sizeof(BlockHeader);
        }

        // 存在しなかった場合
        else
        {
            // より大きな第1カテゴリを探す
            enable_list = m_fliFlagList & (ULLONG_MAX << (fli + 1));

            // 存在した場合
            if (enable_list)
            {
                // メモリブロックのヘッダを取得する
                fli = LeastSignificantBit(enable_list);
                sli = LeastSignificantBit(m_sliFlagList[fli]);
                FreeBlockHeader* header = m_freeBlockList[fli * m_split + sli];

                // フリーブロックリストに登録されている可能性がある場合、削除する
                if (header->GetBlockSize() >= TAG_SIZE)
                    DeleteFromFreeList(header, header->GetBlockSize() + TAG_SIZE);

                // 分割するための領域が十分にある場合、メモリ領域を分割する
                if (header->GetBlockSize() > size + TAG_SIZE)
                {
                    auto headers = DevideMemory((FreeBlockHeader*)header, size + TAG_SIZE);
                    if (header != std::get<0>(headers))
                        throw "[Exception] Unexpected return value.";

                    // 分割後の領域に十分なスペースがあった場合、フリーブロックリストに登録する
                    FreeBlockHeader* new_header = std::get<1>(headers);
                    if (new_header->GetBlockSize() + TAG_SIZE >= FREE_TAG_SIZE)
                        AddToFreeList(new_header, new_header->GetBlockSize() + TAG_SIZE);
                }

                // 占有フラグを立てる
                header->SetIsOccupied(true);

                // メモリブロックのヘッダを返す
                return (uint8_t*)header + sizeof(BlockHeader);
            }

            // それでも存在しなかった場合、確保失敗
            else
            {
                return nullptr;
            }
        }
    }
}

void TLSFAllocator::Free(void* ptr)
{
    // ロックをとる
    if (m_isThreadSafe) std::lock_guard<std::mutex> lock(m_mutex);

    // nullptrを渡されたら何もしない
    if (ptr == nullptr) return;

    // ヘッダ位置にポインタを移動
    BlockHeader* header = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));

    // 占有フラグを下す
    header->SetIsOccupied(false);

    // メモリ領域を結合する
    header = MergeMemory((FreeBlockHeader*)header, header->GetBlockSize() + TAG_SIZE);

    // フリーブロックリストに追加するために十分な領域があるならば、追加する
    if(header->GetBlockSize() + TAG_SIZE >= FREE_TAG_SIZE)
        AddToFreeList((FreeBlockHeader*)header, header->GetBlockSize() + TAG_SIZE);
}

void TLSFAllocator::PrintDebugInfo()
{
    // ロックを取る
    if (m_isThreadSafe) std::lock_guard<std::mutex> lock(m_mutex);

    // メモリプールの先頭のアドレス
    BlockHeader* block_address = (BlockHeader*)m_buffer;

    // メモリのブロック情報をプリント
    printf("\n");
    printf("[ MEMORY TABLE ]\n");
    printf("---------------------------------------------------------\n");
    printf("      Address       :         Block Size, Vacancy        \n");
    printf("---------------------------------------------------------\n");
    while((uint8_t*)block_address < m_buffer + m_bufferSize)
    {
        // メモリブロックの情報を出力
        bool is_occupied = block_address->GetIsOccupied();
        size_t block_size = block_address->GetBlockSize();
        if (is_occupied) printf(" %018p : %18llu[byte], occupied\n", block_address, block_size + TAG_SIZE);
        else printf(" %018p : %18llu[byte], vacant\n", block_address, block_size + TAG_SIZE);
        block_address = (BlockHeader*)((uint8_t*)block_address + block_address->GetBlockSize() + TAG_SIZE);
    }
    printf("---------------------------------------------------------\n");

    // フリーブロックリストの情報をプリント
    printf("[ FREE BLOCK LIST ]\n");
    for (int i = 0; i < 64; ++i)
        for (int j = 0; j < m_split; ++j)
        {
            int count = 0;
            FreeBlockHeader* header = m_freeBlockList[i * m_split + j];
            while (header != nullptr)
            {
                printf("%018p : %llu[byte]\n", header, header->GetBlockSize() + TAG_SIZE);
                header = header->nextBlock;
                ++count;
            }
            if (count) printf("%2d - %2d : %d blocks.\n", i, j, count);
        } 
    printf("\n");
}