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
	// ���X������
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

	// �t���[�u���b�N���X�g��������
	memset(m_freeBlockList, 0, sizeof(FreeBlockHeader*) * MAX_FREELIST);

	// �J�e�S�����X�g��������
	m_fliFlagList = 0;
	memset(m_freeBlockList, 0, sizeof(size_t) * 64);

	// �擪�Ǘ��^�O�����������A�t���[�u���b�N���X�g�ɑ������
	FreeBlockHeader* header = (FreeBlockHeader*)m_buffer;
	header->SetBlockSize(size - TAG_SIZE);
	header->SetIsOccupied(false);
	AddToFreeList(header, size);

	// ���[�Ǘ��^�O��������
	size_t* tailer = (size_t*)((uint8_t*)ptr + size - sizeof(size_t));
	*tailer = size;
}

std::tuple<size_t, size_t> TLSFAllocator::GetCategories(size_t size)
{
	// ��1�J�e�S���̃C���f�b�N�X�����߂�
	size_t fli = MostSignificantBit(size);

	// ��2�J�e�S���̃C���f�b�N�X�����߂�
	size_t mask = (1ULL << fli) - 1ULL;
	size_t shift = fli - m_splitLog;
	size_t sli = (size & mask) >> shift;

	// �J�e�S����Ԃ�
	return std::make_tuple(fli, sli);
}

std::tuple<FreeBlockHeader*, FreeBlockHeader*>  TLSFAllocator::DevideMemory(FreeBlockHeader* header, size_t size)
{
	// �������u���b�N���g�p���Ȃ�Η�O�𓊂���
	if (header->GetIsOccupied())
		throw "[Exception] This memory block is already occupied.";

	// ������̃������u���b�N�ɊǗ������������ރX�y�[�X���Ȃ������ꍇ�A�������s
	if (header->GetBlockSize() <= size)
		throw "[Exception] There is no enough space to write block header.";

	// �]�����������u���b�N�̃T�C�Y�����߂�
	size_t rest_size = header->GetBlockSize() + TAG_SIZE - size;

	// 1�ڂ̐擪�Ǘ��^�O�E���[�Ǘ��^�O�̏�������
	header->SetBlockSize(size - TAG_SIZE);
	size_t* tailer = (size_t*)((uint8_t*)header + size - sizeof(size_t));
	*tailer = size;

	// 2�ڂ̐擪�Ǘ��^�O�E���[�Ǘ��^�O�̏�������
	FreeBlockHeader* next_header = (FreeBlockHeader*)((uint8_t*)header + size);
	next_header->SetBlockSize(rest_size - TAG_SIZE);
	next_header->SetIsOccupied(false);
	size_t* next_tailer = (size_t*)((uint8_t*)next_header + rest_size - sizeof(size_t));
	*next_tailer = rest_size;

	// ���������������̈�̐擪�Ǘ��^�O��Ԃ�
	return std::make_tuple(header, next_header);
}

FreeBlockHeader* TLSFAllocator::MergeMemory(FreeBlockHeader* header, size_t size)
{
	// �������u���b�N���g�p���Ȃ�Η�O�𓊂���
	if (header->GetIsOccupied())
		throw "[Exception] This memory block is already occupied.";

	// ���[�Ǘ��^�O���擾
	size_t* tailer = (size_t*)((uint8_t*)header + size - sizeof(size_t));

	if ((uint8_t*)header + size != m_buffer + m_bufferSize)
	{
		// ���̗̈������
		FreeBlockHeader* next_header = (FreeBlockHeader*)((uint8_t*)header + size);
		size_t* next_tailer = (size_t*)((uint8_t*)next_header + next_header->GetBlockSize() + sizeof(BlockHeader));
		if (!next_header->GetIsOccupied())
		{
			// �t���[�u���b�N���X�g����폜����
			if(next_header->GetBlockSize() >= TAG_SIZE)
				DeleteFromFreeList(next_header, *next_tailer);

			// �擪�Ǘ��^�O�̏�������������
			size_t new_block_size = header->GetBlockSize() + next_header->GetBlockSize() + TAG_SIZE;
			header->SetBlockSize(new_block_size);
			header->SetIsOccupied(false);

			// ���[�Ǘ��^�O�̏�������������
			*next_tailer = new_block_size + TAG_SIZE;

			// ������̖��[�Ǘ��^�O�̃A�h���X����
			tailer = next_tailer;
		}
	}

	if ((uint8_t*)header != m_buffer)
	{
		// �O�̗̈������
		size = *(size_t*)((uint8_t*)header - sizeof(size_t));
		FreeBlockHeader* prev_header = (FreeBlockHeader*)((uint8_t*)header - size);
		size_t* prev_tailer = (size_t*)((uint8_t*)header - sizeof(size_t));
		if (!prev_header->GetIsOccupied())
		{
			// �t���[�u���b�N���X�g����폜����
			if (prev_header->GetBlockSize() >= TAG_SIZE)
				DeleteFromFreeList(prev_header, *prev_tailer);

			// �擪�Ǘ��^�O�̏�������������
			size_t new_block_size = prev_header->GetBlockSize() + header->GetBlockSize() + TAG_SIZE;
			prev_header->SetBlockSize(new_block_size);
			prev_header->SetIsOccupied(false);

			// ���[�Ǘ��^�O�̏�������������
			*tailer = new_block_size + TAG_SIZE;

			// ������̐擪�Ǘ��^�O�̃A�h���X����
			header = prev_header;
		}
	}

	// ���������������̈�̐擪�Ǘ��^�O��Ԃ�
	return header;
}

void TLSFAllocator::AddToFreeList(FreeBlockHeader* header, size_t size)
{
	// �������u���b�N���g�p���Ȃ�Η�O�𓊂���
	if (header->GetIsOccupied())
		throw "[Exception] This memory block is already occupied.";

	// �t���[�u���b�N���X�g�̐ڑ��󋵂̏�����
	header->prevBlock = nullptr;
	header->nextBlock = nullptr;

	// �������u���b�N�̃J�e�S�����擾����
	auto categories = GetCategories(size);
	size_t fli = std::get<0>(categories);
	size_t sli = std::get<1>(categories);

	// �J�e�S�����������
	if (sli) --sli; else if (fli > m_splitLog) { sli = m_split - 1; --fli; }

	// �t���O�𗧂Ă�
	m_fliFlagList |= 1ULL << fli;
	m_sliFlagList[fli] |= 1ULL << sli;

	// ���Ƀ��X�g���������ꍇ�̓��X�g�ɑ}������
	if (m_freeBlockList[fli * m_split + sli])
	{
		FreeBlockHeader* next_header = m_freeBlockList[fli * m_split + sli];
		header->nextBlock = next_header;
		next_header->prevBlock = header;
	}

	// �t���[�u���b�N���X�g�ɒǉ�
	m_freeBlockList[fli * m_split + sli] = header;
}

void TLSFAllocator::DeleteFromFreeList(FreeBlockHeader* header, size_t size)
{
	// �������u���b�N���g�p���Ȃ�Η�O�𓊂���
	if (header->GetIsOccupied())
		throw "[Exception] This memory block is already occupied.";

	if (header->GetBlockSize() < TAG_SIZE)
		throw "[Exception] This memory block is not expected to be in list.";

	// �������u���b�N�̃J�e�S�����擾����
	auto categories = GetCategories(size);
	size_t fli = std::get<0>(categories);
	size_t sli = std::get<1>(categories);

	// �J�e�S�����������
	if (sli) --sli; else if (fli > m_splitLog) { sli = m_split - 1; --fli; }

	// �u���b�N�Ԃ̐ڑ���؂�
	if (header->nextBlock != nullptr)
		header->nextBlock->prevBlock = header->prevBlock;
	if (header->prevBlock != nullptr)
		header->prevBlock->nextBlock = header->nextBlock;

	// �������̃������u���b�N���t���[�u���b�N���X�g�̐擪�������ꍇ
	// �܂��͒��O�̃��X�g��nullptr�������ꍇ
	if (header == m_freeBlockList[fli * m_split + sli] || header->prevBlock == nullptr)
	{
		// ���̃������u���b�N���t���[�u���b�N���X�g�Ƃ���
		m_freeBlockList[fli * m_split + sli] = header->nextBlock;

		// ���̌���nullptr�ƂȂ����ꍇ�A�t���O������
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
	// ���b�N���Ƃ�
	std::lock_guard<std::mutex> lock(m_mutex);

	// 0byte�̊m�ۂł͉������Ȃ�
	if (!size) return nullptr;

	// �������u���b�N�̃J�e�S�����擾����
	auto categories = GetCategories(size + TAG_SIZE);
	size_t fli = std::get<0>(categories);
	size_t sli = std::get<1>(categories);

	// �t���[�u���b�N���X�g�ɓo�^���������ꍇ
	if (m_freeBlockList[fli * m_split + sli] != nullptr)
	{
		// �������u���b�N�̃w�b�_���擾����
		FreeBlockHeader* header = m_freeBlockList[fli * m_split + sli];

		// �t���[�u���b�N���X�g�ɓo�^����Ă���\��������ꍇ�A�폜����
		if (header->GetBlockSize() >= TAG_SIZE)
			DeleteFromFreeList(header, header->GetBlockSize() + TAG_SIZE);

		// ��L�t���O�𗧂Ă�
		header->SetIsOccupied(true);

		// �������u���b�N�̃w�b�_��Ԃ�
		return (uint8_t*)header + sizeof(BlockHeader);
	}

	// �����łȂ��ꍇ
	else
	{
		// ���傫�ȑ�2�J�e�S����T��
		size_t sli_flag = m_sliFlagList[fli];
		size_t enable_list = sli_flag & (ULLONG_MAX << sli);

		// ���݂����ꍇ
		if (enable_list)
		{
			// �������u���b�N�̃w�b�_���擾����
			sli = LeastSignificantBit(enable_list);
			FreeBlockHeader* header = m_freeBlockList[fli * m_split + sli];

			// �t���[�u���b�N���X�g�ɓo�^����Ă���\��������ꍇ�A�폜����
			if (header->GetBlockSize() >= TAG_SIZE)
				DeleteFromFreeList(header, header->GetBlockSize() + TAG_SIZE);

			// �������邽�߂̗̈悪�\���ɂ���ꍇ�A�������̈�𕪊�����
			if(header->GetBlockSize() > size + TAG_SIZE)
			{
				auto headers = DevideMemory((FreeBlockHeader*)header, size + TAG_SIZE);
				if (header != std::get<0>(headers))
					throw "[Exception] Unexpected return value.";
				FreeBlockHeader* new_header = std::get<1>(headers);

				// ������̗̈�ɏ\���ȃX�y�[�X���������ꍇ�A�t���[�u���b�N���X�g�ɓo�^����
				if (new_header->GetBlockSize() + TAG_SIZE >= FREE_TAG_SIZE)
					AddToFreeList(new_header, new_header->GetBlockSize() + TAG_SIZE);
			}

			// ��L�t���O�𗧂Ă�
			header->SetIsOccupied(true);

			// �������u���b�N�̃w�b�_��Ԃ�
			return (uint8_t*)header + sizeof(BlockHeader);
		}

		// ���݂��Ȃ������ꍇ
		else
		{
			// ���傫�ȑ�1�J�e�S����T��
			enable_list = m_fliFlagList & (ULLONG_MAX << (fli + 1));

			// ���݂����ꍇ
			if (enable_list)
			{
				// �������u���b�N�̃w�b�_���擾����
				fli = LeastSignificantBit(enable_list);
				sli = LeastSignificantBit(m_sliFlagList[fli]);
				FreeBlockHeader* header = m_freeBlockList[fli * m_split + sli];

				// �t���[�u���b�N���X�g�ɓo�^����Ă���\��������ꍇ�A�폜����
				if (header->GetBlockSize() >= TAG_SIZE)
					DeleteFromFreeList(header, header->GetBlockSize() + TAG_SIZE);

				// �������邽�߂̗̈悪�\���ɂ���ꍇ�A�������̈�𕪊�����
				if (header->GetBlockSize() > size + TAG_SIZE)
				{
					auto headers = DevideMemory((FreeBlockHeader*)header, size + TAG_SIZE);
					if (header != std::get<0>(headers))
						throw "[Exception] Unexpected return value.";

					// ������̗̈�ɏ\���ȃX�y�[�X���������ꍇ�A�t���[�u���b�N���X�g�ɓo�^����
					FreeBlockHeader* new_header = std::get<1>(headers);
					if (new_header->GetBlockSize() + TAG_SIZE >= FREE_TAG_SIZE)
						AddToFreeList(new_header, new_header->GetBlockSize() + TAG_SIZE);
				}

				// ��L�t���O�𗧂Ă�
				header->SetIsOccupied(true);

				// �������u���b�N�̃w�b�_��Ԃ�
				return (uint8_t*)header + sizeof(BlockHeader);
			}

			// ����ł����݂��Ȃ������ꍇ�A�m�ێ��s
			else
			{
				return nullptr;
			}
		}
	}
}

void TLSFAllocator::Free(void* ptr)
{
	// ���b�N���Ƃ�
	std::lock_guard<std::mutex> lock(m_mutex);

	// nullptr��n���ꂽ�牽�����Ȃ�
	if (ptr == nullptr) return;

	// �w�b�_�ʒu�Ƀ|�C���^���ړ�
	BlockHeader* header = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));

	// ��L�t���O������
	header->SetIsOccupied(false);

	// �������̈����������
	header = MergeMemory((FreeBlockHeader*)header, header->GetBlockSize() + TAG_SIZE);

	// �t���[�u���b�N���X�g�ɒǉ����邽�߂ɏ\���ȗ̈悪����Ȃ�΁A�ǉ�����
	if(header->GetBlockSize() + TAG_SIZE >= FREE_TAG_SIZE)
		AddToFreeList((FreeBlockHeader*)header, header->GetBlockSize() + TAG_SIZE);
}

void TLSFAllocator::PrintDebugInfo()
{
	// �������v�[���̐擪�̃A�h���X
	BlockHeader* block_address = (BlockHeader*)m_buffer;

	// �������̃u���b�N�����v�����g
	printf("\n");
	printf("[ MEMORY TABLE ]\n");
	printf("---------------------------------------------------------\n");
	printf("      Address       :         Block Size, Vacancy        \n");
	printf("---------------------------------------------------------\n");
	while((uint8_t*)block_address < m_buffer + m_bufferSize)
	{
		// �������u���b�N�̏����o��
		bool is_occupied = block_address->GetIsOccupied();
		size_t block_size = block_address->GetBlockSize();
		if (is_occupied) printf(" %018p : %18llu[byte], occupied\n", block_address, block_size + TAG_SIZE);
		else printf(" %018p : %18llu[byte], vacant\n", block_address, block_size + TAG_SIZE);
		block_address = (BlockHeader*)((uint8_t*)block_address + block_address->GetBlockSize() + TAG_SIZE);
	}
	printf("---------------------------------------------------------\n");

	// �t���[�u���b�N���X�g�̏����v�����g
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