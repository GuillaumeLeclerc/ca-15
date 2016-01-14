#include <TSTMLock.h>

#include <iostream>

TSTMLock::TSTMLock(word initialSequenceNumber) {
	this->owner = 0;
	this->version = initialSequenceNumber;
	__sync_synchronize();
}
