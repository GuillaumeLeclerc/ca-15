#include "TSTMMemory.h"
#include "sstm.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

//#define PRINT(x) flockfile(stdout);std::cout << "[" << this->id << "] " << x << std::endl;funlockfile(stdout)
//#define PRINT(x) flockfile(stdout);std::cout << "[" << this->id << "] " << x << std::endl;funlockfile(stdout)


TSTMMemory::TSTMMemory(word id, TSTMLockArray& locks, GlobalClock& clock):
	locks(locks),
	id(id),
	clock(clock)
{
	this->backoff = 0;
}

void TSTMMemory::start() {
	this->begin = clock.getClock();
	this->end = MAXT;
	this->cleanLog();
	PRINT("new tx " << this->begin << ", " << this->end);
}
