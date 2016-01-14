#pragma once
#include <vector>

#include <TSTMLock.h>

class TSTMLockArray {
	private:
		TSTMLock* locks;
		size_t log2Size;
		size_t log2StripSize;
		size_t size;

	public:
		TSTMLockArray(size_t log2Size, size_t log2Strip);
		~TSTMLockArray();
		inline TSTMLock& getLockForAddress(volatile word* ptr) {
			uintptr_t addr = (uintptr_t)ptr;
			addr >>= this->log2StripSize;
			size_t index = addr % this->size;
			return this->locks[index];
		}

		inline TSTMLock& operator[](volatile word*  index) {
			return this->getLockForAddress(index);
		}

};
