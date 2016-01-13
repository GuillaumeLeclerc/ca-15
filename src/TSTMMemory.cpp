#include "TSTMMemory.h"
#include "sstm.h"

#include <iostream>

#include <vector>

#define PRINT(x) flockfile(stdout);std::cout << "[" << this->id << "] " << x << std::endl;funlockfile(stdout)


TSTMMemory::TSTMMemory(word id, TSTMLockArray& locks, GlobalClock& clock):
	locks(locks),
	id(id),
	clock(clock)
{}

void TSTMMemory::start() {
	this->begin = clock.getClock();
	this->end = MAXT;
	this->cleanLog();
	PRINT("new tx " << this->begin << ", " << this->end);
}

void TSTMMemory::write(volatile word* addr, word value) {
	PRINT("Writing: " << value);
	TSTMLock& lock = this->locks[addr];

	word currentVersion = lock.getVersion();
	PRINT("Current Version" << currentVersion);

	if (lock.locked()) {
		TX_ABORT(5);
	}
	word now = this->clock.getClock();

	if (currentVersion > this->end) {
		this->extendValidity(now);
	}

	if (currentVersion <= this->end) {
		this->begin = this->begin > currentVersion?this->begin:currentVersion;
		this->end = this->end<now?this->end:now;
		object o = {
			currentVersion,
			clock.getClock(),
			value
		};
		this->writeLog[addr] = o;
		this->readLog.erase(addr);
	} else {
		TX_ABORT(2);
	}

}

void TSTMMemory::extendValidity(word now) {
	return;
	this->end = now;

	for (auto& it: this->writeLog) {
		TSTMLock& lock = this->locks[it.first];
		word version = lock.getVersion();
		if (version == it.second.from) {
			it.second.to = now;
		}
		this->end = this->end<it.second.to?this->end:it.second.to;
	}
}

word TSTMMemory::read(volatile word* addr) {
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

void TSTMMemory::save() {
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
			flockfile(stdout);
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
			if (this->end < now - 1) {
				this->extendValidity(now - 1);
				if (this->end < now - 1) {
					error = true;
					goto err;
				}
			}
			for (auto& it: this->writeLog) {
				PRINT("WRITING ON MEMORY");
				TSTMLock& lock = this->locks[(volatile word*)it.first];
				lock.changeVersion(this->id, now);
				*it.first = it.second.value;
			}
		}

err:

		for(auto& addr : acquiredLocks) {
			PRINT("Unlocking");
			this->locks[addr].unlock(this->id);
		}

		if (error) {
			TX_ABORT(4);
		}
		PRINT("end TX");
	}

	this->relaseLocks();
	this->cleanLog();
}

void TSTMMemory::rollback() {
	this->cleanLog();
}


void TSTMMemory::relaseLocks() {
}

void TSTMMemory::cleanLog() {
	this->writeLog.clear();
	this->readLog.clear();
}
