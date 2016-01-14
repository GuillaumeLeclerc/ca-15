#pragma once

#include <memory>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>

typedef uintptr_t word;


class TSTMLock {
	private:
		volatile word owner __attribute__((aligned(32)));
		volatile word version __attribute__ ((aligned(32)));

	public:

		TSTMLock(word initialSequenceNumber = 0);
		TSTMLock(const TSTMLock& that) = delete;
		TSTMLock& operator=(const TSTMLock& that) = delete;
		word getVersion() const {
			__sync_synchronize();
			word versionNumber = this->version;
			return versionNumber;
		}

		bool own(word const ownerId) {
			assert(ownerId != 0);
			__sync_synchronize();
			word currentOwner = this->owner;
			return currentOwner == ownerId;
		}

		bool lock(size_t ownerId) {
			assert(ownerId != 0);
			word oldValue = __sync_val_compare_and_swap(&this->owner, 0, ownerId);
			//TODO membar ? 

			const bool res = oldValue == 0 || oldValue == ownerId;
			if (res) {
				assert(this->own(ownerId) == true);
			}
			return res;
		}

		void unlock(size_t const ownerId) {
			assert(ownerId != 0);
			word oldValue = __sync_val_compare_and_swap(&this->owner, ownerId, 0);
			assert(oldValue == ownerId || oldValue == 0);
			assert(this->own(ownerId) == false);
		}

		void changeVersion(word const ownerId, word const newVersion) {
			__sync_synchronize();
			assert(ownerId != 0);
			assert(ownerId == this->owner);
			word currentVersion = this->version;
			__sync_synchronize();
			word gotVersion = __sync_val_compare_and_swap(&this->version, currentVersion, newVersion);
			assert(currentVersion == gotVersion);
			assert(this->version == newVersion);
		}

		bool locked() const {
			__sync_synchronize();
			return this->owner != 0;
		}

};
