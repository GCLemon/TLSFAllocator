#include "TLSFAllocator.h"

/**
 @brief 指定されたメモリ領域をvalでフィル memset相当
 */
void fill(uint8_t* ptr, size_t size, uint8_t val)
{
    for (size_t i = 0; i < size; ++i) ptr[i] = val;
}

/**
 @brief 指定されたメモリ領域がvalで埋められているか確認
 */
bool check(uint8_t* ptr, size_t size, uint8_t val)
{
    if (nullptr == ptr) return true;

    for (size_t i = 0; i < size; ++i)
        if (ptr[i] != val) return false;

    return true;
}

int main()
{
    // 乱数機能の初期化
    srand(65536);

    // アロケータの初期化
    TLSFAllocator::Initialize(1 * 1024 * 1024, true);

    // 諸々定数の宣言・初期化
    std::vector<uint8_t*> vec;
    std::vector<size_t> size;

    // 配列初期化
    for (int i = 0; i < 10000; ++i)
    {
        size.push_back(0);
        vec.push_back(nullptr);
    }

    // AllocとFreeを繰り返す
    for (int j = 0; j < 10000; ++j)
    {
        // Alloc
        for (int i = 0; i < 10000; ++i)
        {
            if (vec[i] == nullptr)
            {
                size[i] = rand() % 128 + 1;
                vec[i] = (uint8_t*)TLSFAllocator::Malloc(size[i], 1);
                fill(vec[i], size[i], i & 0x00ff);
            }
        }

        // Free
        for (int i = 0; i < 10000; ++i)
        {
            if (rand() & 1)
            {
                TLSFAllocator::Free(vec[i]);
                vec[i] = nullptr;
            }
        }

        // メモリの中身をチェック
        for (int i = 0; i < 10000; ++i)
        {
            if (!check(vec[i], size[i], i & 0x00ff))
            {
                std::cout << "メモリ破壊を検出" << std::endl;
            }
        }

        printf("Loop%d Done.\n", j + 1);
    }

    // 領域の解放処理
    for (int i = 0; i < 10000; ++i) TLSFAllocator::Free(vec[i]);

    TLSFAllocator::Terminate();
}