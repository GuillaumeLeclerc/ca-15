#include "sstm.h"
#include "iostream"

#include <thread>
#include <chrono>

#define T 4
#define I 2

int value = 0;
int* ptr = &value;

void increase() {
	TX_START();
	std::this_thread::yield();
	size_t cv = TX_LOAD(ptr);
	std::this_thread::yield();
	TX_STORE(ptr, cv * 3);
	std::this_thread::yield();
	cv = TX_LOAD(ptr);
	size_t* ptr2 = (size_t*) TX_MALLOC(sizeof(size_t));
	TX_STORE(ptr, cv + 1);
	std::this_thread::yield();
	TX_STORE(ptr2, cv);
	std::this_thread::yield();
	size_t v = TX_LOAD(ptr2);
	std::this_thread::yield();
	assert(v == cv);
	TX_FREE(ptr2);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	TX_COMMIT();
}

void work() {
	TM_THREAD_START();
	for (int i = 0 ; i < I; ++i) {
		increase();
	}
	TM_THREAD_STOP();
}

int main(void) {

	TM_START();

	std::thread threads[T];

	for (int i = 0 ; i < T; ++i) {
		threads[i] = std::thread(work);
	}

	for (int i = 0 ; i < T; ++i) {
		threads[i].join();
	}
	TM_STOP();
	std::cout << "--" << value << "--" << std::endl;
}
