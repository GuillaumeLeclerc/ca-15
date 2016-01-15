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

		inline word getIndex(volatile word* ptr) {
			uintptr_t addr = (uintptr_t)ptr;
			addr >>= this->log2StripSize;
			size_t index = addr % this->size;
			return index;
		}

		inline TSTMLock& getLockForAddress(volatile word* ptr) {
			return this->locks[this->getIndex(ptr)];
		}

		inline TSTMLock& getLockAtIndex(word index) {
			return this->locks[index];
		}

		inline word getSize() {
			return this->size;
		}

		inline TSTMLock& operator[](volatile word*  index) {
			return this->getLockForAddress(index);
		}

};
