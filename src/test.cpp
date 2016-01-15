#include "sstm.h"
#include "iostream"

#include <thread>
#include <chrono>

#define T 10
#define I 100

int value = 0;
int* ptr = &value;


void read() {
	TX_LOAD(ptr);
	TX_LOAD(ptr);
}
void increase() {
	std::this_thread::yield();
	size_t cv = TX_LOAD(ptr);
	std::this_thread::yield();
	TX_STORE(ptr, cv + 1);
	std::this_thread::yield();
}

void work() {
	TM_THREAD_START();
	for (int i = 0 ; i < 2*I; ++i) {
		TX_START();
		if (i%2 == 0) {
			increase();
		} else {
			read();
		}
		TX_COMMIT();
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
