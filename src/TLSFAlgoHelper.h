#pragma once

#include <cstddef>

/**
 @brief 先頭管理ヘッダ
 */
class BlockHeader
{
private:
	size_t flags;

public:
	/**
	 @brief このブロックが使用中であるかどうかを取得する
	 */
	bool GetIsOccupied()
	{
		return flags & 0x8000000000000000;
	}
	/**
	 @brief このブロックが使用中であるかどうかを設定する
	 */
	void SetIsOccupied(bool isOccupied)
	{
		size_t mask = 0x8000000000000000;
		if (isOccupied) flags |= mask; else flags &= ~mask;
	};

	/**
	 @brief このブロックの管理領域のサイズを取得する
	 */
	size_t GetBlockSize()
	{
		return flags & 0x7fffffffffffffff;
	}
	/**
	 @brief このブロックの管理領域のサイズを設定する
	 */
	void SetBlockSize(size_t blockSize)
	{
		size_t mask = 0x7fffffffffffffff;
		flags = (blockSize & mask) | (flags & ~mask);
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
	FreeBlockHeader* prevBlock;

	/**
	 @brief 自身が所属しているフリーブロックリストについて直後に登録されているブロックの先頭管理ヘッダ
	 */
	FreeBlockHeader* nextBlock;
};

/**
 @brief フリーブロックリストの最大サイズ
 */
static const int MAX_FREELIST = 4096;

/**
 @brief 第1カテゴリの最大数
 */
static const int MAX_FLI_CATEGORY = 64;

/**
 @brief 第2カテゴリの最大数
 */
static const int MAX_SLI_CATEGORY = 64;

/**
 @brief 先頭菅タグ・末端管理タグの総合サイズ
 */
static const size_t TAG_SIZE = sizeof(BlockHeader) + sizeof(size_t);

/**
 @brief 未割当ブロックの先頭菅タグ・末端管理タグの総合サイズ
 */
static const size_t FREE_TAG_SIZE = sizeof(FreeBlockHeader) + sizeof(size_t);