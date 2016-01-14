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


#define PRINT(x) ;

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
		std::set<word*> allocated;
		std::set<word*> freed;
		size_t begin;
		size_t end;
		size_t backoff;

		inline void relaseLocks() { }
		inline void cleanLog() {
			this->writeLog.clear();
			this->readLog.clear();
			this->freed.clear();
			assert(this->allocated.size() == 0);
		}
		inline void extendValidity(word now) {
			this->end = now;

			for (auto& it: this->writeLog) {
				TSTMLock& lock = this->locks[it.first];
				if (!lock.locked()) {
					word version = lock.getVersion();
					PRINT("CUrrent VVV " << version);
					if (version == it.second.from) {
						it.second.to = now;
					}
				}
				this->end = this->end<it.second.to?this->end:it.second.to;
				PRINT("Extending end to " << this->end);
			}

			for (auto& it: this->readLog) {
				TSTMLock& lock = this->locks[it.first];
				if (!lock.locked()) {
					word version = lock.getVersion();
					PRINT("CUrrent VVV " << version);
					if (version == it.second.from) {
						it.second.to = now;
					}
				}
				this->end = this->end<it.second.to?this->end:it.second.to;
				PRINT("Extending end to " << this->end);
			}
		}
		inline void increaseBackoff() {
			PRINT("== Oold backoff" << this->backoff);
			if (this->backoff == 0) {
				this->backoff = 1;
			} else {
				this->backoff *= 2;
			}
			PRINT("==New Backoff " << this->backoff);
		}
		inline void wait() {
			PRINT("==Sleeping " << this->backoff);
			std::this_thread::sleep_for(std::chrono::microseconds(this->backoff));
		}

	public:
		TSTMMemory(word id, TSTMLockArray& locks, GlobalClock& clock);
		void start();

		inline word read(volatile word* addr) {
			TSTMLock& lock = this->locks[addr];

			auto writeEntry = this->writeLog.find(addr);
			// the value has already been written we must return the last value
			if (writeEntry != this->writeLog.end()) {
				return writeEntry->second.value;
			}

			auto readEntry = this->readLog.find(addr);
			// the value has been previously read, we can't provide a newer version;
			if (readEntry != this->readLog.end()) {
				return readEntry->second.value;
			}

			word latestVersion = lock.getVersion();
			if (latestVersion > this->end) {
				this->extendValidity(this->clock.getClock());
			}

			if (latestVersion <= this->end) {
				if (lock.locked()) {
					TX_ABORT(5);
				}
				word now = this->clock.getClock();
				__sync_synchronize();
				word value = *addr;
				PRINT("Reading " << value << " at " << now);
				this->begin = this->begin>latestVersion?this->begin:latestVersion;
				this->end = this->end<now?this->end:now;
				object o = {
					latestVersion,
					now,
					value
				};
				this->readLog[addr] = o;
				return value;
			}

			TX_ABORT(1);
		}


		inline void write(volatile word* addr, word value) {
			PRINT("Writing: " << value);
			TSTMLock& lock = this->locks[addr];



			if (!lock.lock(this->id)) {
				TX_ABORT(5);
			}

			PRINT("GETTING THE FUCKING LOCK");

			word currentVersion = lock.getVersion();
			word now = this->clock.getClock();

			lock.unlock(this->id);

			PRINT("Current Version" << currentVersion << "Now " << now);
			PRINT(this->begin << " - "  << this->end);

			if (currentVersion > this->end) {
				this->extendValidity(now);
			}
			PRINT(this->begin << " - "  << this->end);

			if (currentVersion <= this->end) {
				this->begin = this->begin > currentVersion?this->begin:currentVersion;
				this->end = this->end<now?this->end:now;
				object o = {
					currentVersion,
					now,
					value
				};
				this->writeLog[addr] = o;
				this->readLog.erase(addr);
				PRINT(this->begin << " - "  << this->end);
			} else {
				TX_ABORT(2);
			}
		}
		inline void save() {
			if (this->writeLog.size() > 0) {
				std::vector<word*> acquiredLocks(0);
				bool error = false;

				for (auto& it: this->writeLog) {
					if (this->locks[it.first].lock(this->id)) {
						PRINT("able to lock");
						acquiredLocks.push_back((word *)it.first);
					} else {
						PRINT("Unable to lock");
						error = true;
						goto err;
					}
				}

				if (!error) {
					word now = this->clock.increase(1);
					/*flockfile(stdout);
					  std::cout << "COMMIT STATUS at " << now << std::endl;
					  std::cout << "me -> [" << this->begin << ", " << this->end << "]" << std::endl;
					  for (auto& it : this->writeLog) {
					  std::cout << it.first << " = [" << it.second.from << ", " << it.second.to << "] -> " << it.second.value << std::endl;
					  }
					  std::cout << "------" << std::endl;
					  for (auto& it : this->readLog) {
					  std::cout << it.first << " = [" << it.second.from << ", " << it.second.to << "] -> " << it.second.value << std::endl;
					  }
					  std::cout << "------" << std::endl;
					  funlockfile(stdout);
					  */
					if (this->end < now - 1) {
						this->extendValidity(now - 1);
						//PRINT("me -> [" << this->begin << ", " << this->end << "]");
						if (this->end < now - 1) {
							error = true;
							goto err;
						}
					}
					for (auto& it: this->writeLog) {
						PRINT("WRITING ON MEMORY");
						TSTMLock& lock = this->locks[(volatile word*)it.first];
						lock.changeVersion(this->id, now);
						assert(lock.getVersion() == now);
						__sync_synchronize();
						PRINT("WRITING ON MEMORY -- " << now);
						*it.first = it.second.value;
					}
				}

err:

				for(auto& addr : acquiredLocks) {
					PRINT("Unlocking");
					__sync_synchronize();
					this->locks[addr].unlock(this->id);
					PRINT("UNLOCKED " << this->locks[addr].getVersion());
				} 
				if (error) {
					TX_ABORT(4);
				}
				// allocation were usefull
				this->allocated.clear();
				//free only if the transaction commit
				for(auto& it : this->freed) {
					free(it);
				}


				PRINT("== Reset backoff");
				this->backoff = 0;
				PRINT("end TX");
			}

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
