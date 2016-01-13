#pragma once
#include <unordered_map>

#include "GlobalClock.h"
#include "TSTMLock.h"
#include "TSTMLockArray.h"

typedef struct {
	word from;
	word to;
	word value;
} object;

#define MAXT ~((uint64_t) 0)


class TSTMMemory {
	private:
		TSTMLockArray& locks;
		word id;
		GlobalClock& clock;
		std::unordered_map<volatile word*, object> writeLog;
		std::unordered_map<volatile word*, object> readLog;
		size_t begin;
		size_t end;

		void relaseLocks();
		void cleanLog();
		void extendValidity(word now);
	
	public:
		TSTMMemory(word id, TSTMLockArray& locks, GlobalClock& clock);
		void start();
		word read(volatile word* addr);
		void write(volatile word* addr, word value);
		void save();
		void rollback();
};
