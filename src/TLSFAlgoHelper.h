#pragma once

#include <cstddef>

/**
 @brief �擪�Ǘ��w�b�_
 */
class BlockHeader
{
private:
	size_t flags;

public:
	/**
	 @brief ���̃u���b�N���g�p���ł��邩�ǂ������擾����
	 */
	bool GetIsOccupied()
	{
		return flags & 0x8000000000000000;
	}
	/**
	 @brief ���̃u���b�N���g�p���ł��邩�ǂ�����ݒ肷��
	 */
	void SetIsOccupied(bool isOccupied)
	{
		size_t mask = 0x8000000000000000;
		if (isOccupied) flags |= mask; else flags &= ~mask;
	};

	/**
	 @brief ���̃u���b�N�̊Ǘ��̈�̃T�C�Y���擾����
	 */
	size_t GetBlockSize()
	{
		return flags & 0x7fffffffffffffff;
	}
	/**
	 @brief ���̃u���b�N�̊Ǘ��̈�̃T�C�Y��ݒ肷��
	 */
	void SetBlockSize(size_t blockSize)
	{
		size_t mask = 0x7fffffffffffffff;
		flags = (blockSize & mask) | (flags & ~mask);
	};
};

/**
 @brief �������u���b�N�Ɏg�p����擪�Ǘ��w�b�_
 */
class FreeBlockHeader : public BlockHeader
{
public:
	/**
	 @brief ���g���������Ă���t���[�u���b�N���X�g�ɂ��Ē���ɓo�^����Ă���u���b�N�̐擪�Ǘ��w�b�_
	 */
	FreeBlockHeader* prevBlock;

	/**
	 @brief ���g���������Ă���t���[�u���b�N���X�g�ɂ��Ē���ɓo�^����Ă���u���b�N�̐擪�Ǘ��w�b�_
	 */
	FreeBlockHeader* nextBlock;
};

/**
 @brief �t���[�u���b�N���X�g�̍ő�T�C�Y
 */
static const int MAX_FREELIST = 4096;

/**
 @brief ��1�J�e�S���̍ő吔
 */
static const int MAX_FLI_CATEGORY = 64;

/**
 @brief ��2�J�e�S���̍ő吔
 */
static const int MAX_SLI_CATEGORY = 64;

/**
 @brief �擪���^�O�E���[�Ǘ��^�O�̑����T�C�Y
 */
static const size_t TAG_SIZE = sizeof(BlockHeader) + sizeof(size_t);

/**
 @brief �������u���b�N�̐擪���^�O�E���[�Ǘ��^�O�̑����T�C�Y
 */
static const size_t FREE_TAG_SIZE = sizeof(FreeBlockHeader) + sizeof(size_t);