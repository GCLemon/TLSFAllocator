#include <iostream>
#include <vector>
#include <cstdint>
#include <stdlib.h>
#include <time.h>

#include "TLSFAllocator.h"

// 指定されたメモリ領域をvalでフィル memset相当
void fill(uint8_t* ptr, size_t size, uint8_t val)
{
	for (size_t i = 0; i < size; ++i) {
		ptr[i] = val;
	}
}

// 指定されたメモリ領域がvalで埋められているか確認
bool check(uint8_t* ptr, size_t size, uint8_t val)
{
	if (nullptr == ptr) return true;

	for (size_t i = 0; i < size; ++i) {
		if (ptr[i] != val) return false;
	}
	return true;
}

// test
int main()
{
	Initialize();

	srand(static_cast<unsigned int>(time(NULL)));

	void* ptr = new char[1 * 1024 * 1024];

	TLSFAllocator* im = new TLSFAllocator(ptr, 1 * 1024 * 1024);
	std::cout << "test 開始" << std::endl;

	const int n = 100;
	std::vector<uint8_t*> vec;
	std::vector<size_t> size;


	// 配列初期化
	for (int i = 0; i < n; ++i)
	{
		size.push_back(0);
		vec.push_back(nullptr);
	}

	for (int j = 0; j < n; ++j) {

		for (int i = 0; i < n; ++i)
		{
			if (vec[i] == nullptr)
			{
				size[i] = rand() % 128 + 1;
				vec[i] = reinterpret_cast<uint8_t*>(im->Alloc(size[i]));
				fill(vec[i], size[i], i & 0x00ff);
			}
		}

		for (int i = 0; i < n; ++i)
		{
			if (rand() & 1) {
				im->Free(vec[i]);
				vec[i] = nullptr;
			}
		}

		for (int i = 0; i < n; ++i)
		{
			if (!check(vec[i], size[i], i & 0x00ff)) {
				std::cout << "メモリ破壊を検出" << std::endl;
			}
		}
	}

	for (int i = 0; i < n; ++i)
	{
		im->Free(vec[i]);
	}

	std::cout << "test 終了" << std::endl;

	delete[] ptr;

	Terminate();

	return 0;
}
