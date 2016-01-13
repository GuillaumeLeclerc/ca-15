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
	/**
	 * Writer part
	 */
	// I want the lock
	bool lock(word const ownerId);

	//do I own the lock ?
	bool own(word const ownerId);

	// I don't want the lock anymore
	void unlock(word const ownerId);

	void changeVersion(word const ownerId, word const newVersion);

	/**
	 * Reader part
	 */
	word getVersion() const;

	bool locked() const;
};
