#pragma once

#include <stdio.h>
#include <stdlib.h> 

class GlobalClock {
	private:
		volatile size_t value;

	public:

		GlobalClock() {
			this->value = 0;
		}

		size_t increase(size_t inc) {
			return __sync_fetch_and_add(&value, inc) + inc;
		}

		size_t getClock() {
			return this->value;
		}


};
