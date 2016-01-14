#include <TSTMLockArray.h>
/**
#include <pthread.h>
#include <GlobalClock.h>
#include <iostream>
#include <chrono>
#include <thread>

#define T 3
#define PRINT(x) do{flockfile(stdout); std::cout << x << std::endl; funlockfile(stdout);}while(false)
#define D 14

**/


TSTMLockArray::TSTMLockArray(size_t pow2Size, size_t stripSize) {
	this->log2Size = pow2Size;
	this->log2StripSize = stripSize + 3;
	this->size = 1 << this->log2Size;
	this->locks = new TSTMLock[this->size];
}

TSTMLockArray::~TSTMLockArray() {
	delete this->locks;
}


/**
GlobalClock versioning;
word* data;
TSTMLockArray locks(D - 4, 1);

void increase(word* ptr, sstm_metadata* owner) {
	PRINT("Increasing " << ptr << " By " << owner);
	TSTMLock& lock = locks[ptr];
	while(!lock.lock(owner)) { }
	*ptr *= 3;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	*ptr += 1;
	PRINT("Finished Increasing " << ptr << " By " << owner);
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	lock.unlock(owner, versioning.increase(2));
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void* work(void* input) {
	uintptr_t myId = (uintptr_t) input;
	myId <<= 1;
	size_t localSize = 1 << D;
	sstm_metadata* me = (sstm_metadata*) myId;
	PRINT("Hello I'm " << myId);
	for (size_t i = myId / 2 ; i < localSize; ++i) {
		for (size_t j = 0 ; j < 1 ; ++j) {
			increase(data + i, me);
		}
	}
	return NULL;
}

int main(void) {
	pthread_t threads[T];
	size_t localSize = 1 << D;
	data = new word[localSize];
	for (size_t i = 0; i < localSize; ++i) {
		data[i] = 0;
	}

	for (size_t i = 0 ; i < T; ++i) {
		pthread_create(threads + i, NULL, work, (void*) i);
	}
	for (size_t i = 0 ; i < T; ++i) {
		pthread_join(threads[i], NULL);
	}

	for (size_t i = 0; i < localSize; ++i) {
		PRINT(data[i]);
	}

	delete data;
}
**/
