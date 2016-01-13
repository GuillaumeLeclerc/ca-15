#include <TSTMLock.h>

#include <iostream>

TSTMLock::TSTMLock(word initialSequenceNumber) {
	this->owner = 0;
	this->version = initialSequenceNumber;
	__sync_synchronize();
}


word TSTMLock::getVersion() const {
	__sync_synchronize();
	word versionNumber = this->version;
	return versionNumber;
}

bool TSTMLock::own(word const ownerId) {
	assert(ownerId != 0);
	__sync_synchronize();
	word currentOwner = this->owner;
	return currentOwner == ownerId;
}

bool TSTMLock::lock(size_t ownerId) {
	assert(ownerId != 0);
	word oldValue = __sync_val_compare_and_swap(&this->owner, 0, ownerId);
	//TODO membar ? 
	
	const bool res = oldValue == 0 || oldValue == ownerId;
	if (res) {
		assert(this->own(ownerId) == true);
	}
	return res;
}

void TSTMLock::unlock(size_t const ownerId) {
	assert(ownerId != 0);
	word oldValue = __sync_val_compare_and_swap(&this->owner, ownerId, 0);
	assert(oldValue == ownerId || oldValue == 0);
	assert(this->own(ownerId) == false);
}

void TSTMLock::changeVersion(word const ownerId, word const newVersion) {
	__sync_synchronize();
	assert(ownerId != 0);
	assert(ownerId == this->owner);
	word currentVersion = this->version;
	__sync_synchronize();
	word gotVersion = __sync_val_compare_and_swap(&this->version, currentVersion, newVersion);
	assert(currentVersion == gotVersion);
	__sync_synchronize();
	assert(this->version == newVersion);
}

bool TSTMLock::locked() const {
	__sync_synchronize();
	std::cout << this->owner << std::endl;
	return this->owner != 0;
}

