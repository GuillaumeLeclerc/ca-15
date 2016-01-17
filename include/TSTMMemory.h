#pragma once
#include <unordered_map>
#include <set>

#include "GlobalClock.h"
#include "TSTMLock.h"
#include "TSTMLockArray.h"
#include "sstm.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <set>

#include <algorithm>


#if DEBUG
#define PRINT(x) flockfile(stdout);std::cout << "[" << this->id << "] " << x << std::endl;funlockfile(stdout)
#else 
#define PRINT(x) ;
#endif
typedef struct {
	volatile word* addr;
	word from;
	word to;
	word value;
} object;

#define MAXT ~((uint64_t) 0)


class TSTMMemory {
	private:

		static std::atomic_uint readers;
		TSTMLockArray& locks;
		word id;
		GlobalClock& clock;
		std::unordered_map<volatile word*, word> writeLog;
		std::set<word*> allocated;
		std::set<word*> freed;
		std::vector<word> acquiredLocks;
		size_t begin;
		size_t end;
		size_t backoff;
		bool readOnly;
    void* addr;
    std::set<void*> nroAddr;

		inline void relaseLocks() { 
			std::sort( this->acquiredLocks.begin(), this->acquiredLocks.end() );
			this->acquiredLocks.erase( std::unique( this->acquiredLocks.begin(), this->acquiredLocks.end() ), this->acquiredLocks.end() );
				for(auto& index : acquiredLocks) {
					PRINT("Unlocking " << index);
					this->locks.getLockAtIndex(index).unlock(this->id);
				} 
				acquiredLocks.clear();
		}
		inline void cleanLog() {
			this->writeLog.clear();
			this->freed.clear();
			assert(this->allocated.size() == 0);
		}


		inline void increaseBackoff() {
			PRINT("== Oold backoff" << this->backoff);
			if (this->backoff == 0) {
				this->backoff = 1;
			} else {
				this->backoff += 10;
			}
			PRINT("==New Backoff " << this->backoff);
		}

		inline void wait() {
			PRINT("==Sleeping " << this->backoff);
			std::this_thread::sleep_for(std::chrono::microseconds(this->backoff));
		}

		inline void getLock(volatile word* addr) {
			PRINT("Trying to ack " << this->locks.getIndex(addr));
			if(!this->locks[addr].lock(this->id)) {
				TX_ABORT(1);
			}
			this->acquiredLocks.push_back(this->locks.getIndex(addr));
			assert(this->locks.getLockAtIndex(this->locks.getIndex(addr)).own(this->id));
			PRINT("Locked " << this->locks.getIndex(addr));
		}

	public:
		TSTMMemory(word id, TSTMLockArray& locks, GlobalClock& clock);
		void start();

		inline word read(volatile word* addr) {
			//read only mode
			if (this->readOnly) {
				word version = this->locks[addr].getVersion();
				if (version < begin) { // must not read new values;
					TX_ABORT(2);
				}
			} else {
				this->getLock(addr);
			}
			word val = *addr;
			PRINT("Reading " << val);
			return val;
		}

		inline void newnew() {
      this->readOnly = true;
      return;
      this->addr = __builtin_return_address(0);
      PRINT("---addr" << this->addr);
      if (this->nroAddr.find(this->addr) != this->nroAddr.end()) {
        this->readOnly = false;
      } else {
        this->readOnly = true;
      }
		}


		inline void write(volatile word* addr, word value) {
			if (this->readOnly) {
        this->nroAddr.insert(this->addr);
				PRINT("FUCK YOU I'm not read only");
				this->readOnly = false;
				TX_ABORT(6);
			}

      word version = this->locks[addr].getVersion();
      if (version < begin) { // must not write new values;
        TX_ABORT(2);
      }
			this->getLock(addr);
      *addr = value;
		}
		inline void save() {
			// allocation were usefull
			this->allocated.clear();
			//free only if the transaction commit
			for(auto& it : this->freed) {
				free(it);
			}

			PRINT("== Reset backoff");
			this->backoff = 0;
			PRINT("end TX");
			this->relaseLocks();
			this->cleanLog();
		}
		inline void rollback() {
			//freeing useless memory
			for (auto& it : this->allocated) {
				free(it);
			}
			this->allocated.clear();
			this->cleanLog();
			this->relaseLocks();
			//this->increaseBackoff();
			//this->wait();
		}
		inline word* alloc(size_t t) {
			word* ptr = (word*) malloc(t);
			this->allocated.insert(ptr);
			return ptr;
		}
		inline void free(word* ptr) {
			this->freed.insert(ptr);
		}
};
