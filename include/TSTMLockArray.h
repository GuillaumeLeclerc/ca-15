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
		TSTMLock& getLockForAddress(volatile word* ptr);
		TSTMLock& operator[](volatile word* index);
};
