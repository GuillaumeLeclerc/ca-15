#pragma once

#include <stdio.h>
#include <stdlib.h> 

class GlobalClock {
	private:
		volatile size_t value;
	
	public:

		GlobalClock();
		size_t increase(size_t value = 1);
		size_t getClock();
};
