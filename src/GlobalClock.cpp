#include <GlobalClock.h>

#include <iostream>

GlobalClock::GlobalClock() {
	this->value = 0;
}

size_t GlobalClock::increase(size_t inc) {
 	return __sync_fetch_and_add(&value, inc) + inc;
}

size_t GlobalClock::getClock() {
	return this->value;
}

