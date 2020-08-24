#include <chrono>
#include <iostream>
#include <vector>

#include "TLSFAllocator.h"

/**
 @brief 乱数に使用するシード値
 */
static size_t RANDOM_SEED = 65536;

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

/**
 @brief C++標準のメモリアロケータの計測
 */
void StandardAllocatorTest(size_t random_seed, size_t alloc_size, size_t loop_count)
{
    // 乱数機能の初期化
    srand(random_seed);

    // 諸々定数の宣言・初期化
    std::vector<uint8_t*> vec;
    std::vector<size_t> size;

    // 配列初期化
    for (int i = 0; i < loop_count; ++i)
    {
        size.push_back(0);
        vec.push_back(nullptr);
    }

    // AllocとFreeを繰り返す
    for (int j = 0; j < loop_count; ++j)
    {
        // Alloc
        for (int i = 0; i < loop_count; ++i)
            if (vec[i] == nullptr)
            {
                size[i] = rand() % 128 + 1;
                vec[i] = new uint8_t[size[i]];
                fill(vec[i], size[i], i & 0x00ff);
            }

        // Free
        for (int i = 0; i < loop_count; ++i)
            if (rand() & 1)
            {
                delete[] vec[i];
                vec[i] = nullptr;
            }

        // メモリの中身をチェック
        for (int i = 0; i < loop_count; ++i)
            if (!check(vec[i], size[i], i & 0x00ff))
            {
                std::cout << "メモリ破壊を検出" << std::endl;
            }
    }

    // 領域の解放処理
    for (int i = 0; i < loop_count; ++i) delete[] vec[i];
}

/**
 @brief TLSFによるメモリアロケータの計測
 */
void TLSFAllocatorTest(size_t random_seed, size_t alloc_size, size_t loop_count)
{
    // 乱数機能の初期化
    srand(random_seed);

    // 諸々定数の宣言・初期化
    void* ptr = new char[alloc_size];

    // アロケータのインスタンスを作成
    TLSFAllocator* im = new TLSFAllocator(ptr, alloc_size);
    im->SetIsTrehadSafe(true);

    // 諸々定数の宣言・初期化
    std::vector<uint8_t*> vec;
    std::vector<size_t> size;

    // 配列初期化
    for (int i = 0; i < loop_count; ++i)
    {
        size.push_back(0);
        vec.push_back(nullptr);
    }

    // AllocとFreeを繰り返す
    for (int j = 0; j < loop_count; ++j)
    {
        // Alloc
        for (int i = 0; i < loop_count; ++i)
            if (vec[i] == nullptr)
            {
                size[i] = rand() % 128 + 1;
                vec[i] = (uint8_t*)im->Alloc(size[i]);
                fill(vec[i], size[i], i & 0x00ff);
            }

        // Free
        for (int i = 0; i < loop_count; ++i)
            if (rand() & 1)
            {
                im->Free(vec[i]);
                vec[i] = nullptr;
            }

        // メモリの中身をチェック
        for (int i = 0; i < loop_count; ++i)
            if (!check(vec[i], size[i], i & 0x00ff))
            {
                std::cout << "メモリ破壊を検出" << std::endl;
            }
    }

    // 領域の解放処理
    for (int i = 0; i < loop_count; ++i) im->Free(vec[i]);
    delete[] (char*)ptr;
}

/**
 @brief メインルーチン
 */
int main()
{
    Initialize();

    // 諸々変数の宣言
    std::chrono::system_clock::time_point start, end;
    uint64_t microsec;
    double time;

    //  C++標準のメモリアロケータの計測
    printf("[ C++ Standard Allocator ]\n");
    printf("START\n");
    start = std::chrono::system_clock::now();
    StandardAllocatorTest(RANDOM_SEED, 1 * 1024 * 1024, 10000);
    end = std::chrono::system_clock::now();
    microsec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    time = ((double)microsec) * 0.000001;
    printf("FINISH\n");
    printf("Result : %lf[sec]\n", time);

    //  TLSFによるのメモリアロケータの計測
    printf("[ TLSF Allocator ]\n");
    printf("START\n");
    start = std::chrono::system_clock::now();
    TLSFAllocatorTest(RANDOM_SEED, 1 * 1024 * 1024, 10000);
    end = std::chrono::system_clock::now();
    microsec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    time = ((double)microsec) * 0.000001;
    printf("FINISH\n");
    printf("Result : %lf[sec]\n", time);

    Terminate();

    return 0;
}
