#pragma once

#include <memory>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>

#include <atomic>
#include <iostream>

#if DEBUG
#define PRINT2(x) flockfile(stdout);std::cout << "[" << ownerId << "] " << x << std::endl;funlockfile(stdout)

#else 
#define PRINT2(x) ;
#endif
typedef uintptr_t word;


class TSTMLock {
	private:
		std::atomic_uint_least64_t owner;
		std::atomic_uint_least64_t version;
		std::atomic_uint_least64_t readCount;

	public:

		TSTMLock(word initialSequenceNumber = 1);
		TSTMLock(const TSTMLock& that) = delete;
		TSTMLock& operator=(const TSTMLock& that) = delete;

		word getVersion() const {
			return version.load();
		}

		bool own(word const ownerId) {
			assert(ownerId != 0);
			return ownerId == owner.load();
		}

		bool lock(size_t ownerId) {
			assert(ownerId != 0);
			word expected = 0;
			__sync_synchronize();
			const bool success = owner.compare_exchange_strong(expected, ownerId);
			__sync_synchronize();

			PRINT2("Succ Ack lock " << success);
			PRINT2("Old value was" << expected);

			const bool res = success || expected == ownerId;
			if (res) {
				//assert(this->own(ownerId) == true);
			} else {
				assert(expected != 0 && expected != ownerId);
			}
			return res;
		}

		void unlock(size_t const ownerId) {
			assert(ownerId != 0);
			word expected = ownerId;
			assert(ownerId == owner.load());
			bool success = owner.compare_exchange_strong(expected, 0);
			assert(success || expected == 0);
			PRINT2("LOCK SUCC " << success << " " << this->owner.load());
		}

		void changeVersion(word const ownerId, word const newVersion) {
			assert(ownerId != 0);
			assert(ownerId == this->owner);
			version.store(newVersion);
			assert(this->version == newVersion);
		}

		bool locked() const {
			return this->owner != 0;
		}

		void addReader() {
			this->readCount.fetch_add(1);
		}

		void decReader() {
			this->readCount.fetch_sub(1);
		}

		word getReaderCount() {
			return this->readCount.load();
		}
};
