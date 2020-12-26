#pragma once

#include <algorithm>
#include <tuple>

#include <stdint.h>

namespace TLSFAllocator
{
    /**
     @brief 最上位ビットを取得
     */
    inline uint8_t MostSignificantBit(uint64_t x)
    {
        uint8_t msb = 63;

        if (!(x & 0xffffffff00000000)) { x <<= 32; msb -= 32; }
        if (!(x & 0xffff000000000000)) { x <<= 16; msb -= 16; }
        if (!(x & 0xff00000000000000)) { x <<= 8; msb -= 8; }
        if (!(x & 0xf000000000000000)) { x <<= 4; msb -= 4; }
        if (!(x & 0xc000000000000000)) { x <<= 2; msb -= 2; }
        if (!(x & 0x8000000000000000)) { x <<= 1; msb -= 1; }

        return msb;
    }

    /**
     @brief 最下位ビットを取得
     */
    inline uint8_t LeastSignificantBit(uint64_t x)
    {
        uint8_t lsb = 0; x &= ~x + 1;

        if (x & 0xffffffff00000000) lsb += 32;
        if (x & 0xffff0000ffff0000) lsb += 16;
        if (x & 0xff00ff00ff00ff00) lsb += 8;
        if (x & 0xf0f0f0f0f0f0f0f0) lsb += 4;
        if (x & 0xcccccccccccccccc) lsb += 2;
        if (x & 0xaaaaaaaaaaaaaaaa) lsb += 1;

        return lsb;
    }

    /**
     @brief 確保領域のサイズからカテゴリを算出する
     @param size 確保領域のサイズ
     @return 第一・第二カテゴリのサイズ
     */
    inline std::tuple<uint8_t, uint8_t> GetCategory(size_t size)
    {
        // 第1カテゴリのインデックスを求める
        int8_t fli = MostSignificantBit(size);

        // 第2カテゴリのインデックスを求める
        size_t mask = (1ULL << fli) - 1ULL;
        size_t shift = std::max(fli - 6, 0);
        int8_t sli = (size & mask) >> shift;

        // カテゴリを返す
        return std::make_tuple(fli, sli);
    }
}