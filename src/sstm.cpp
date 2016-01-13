#include "GlobalClock.h"
#include "TSTMLock.h"
#include "TSTMLockArray.h"
#include "TSTMMemory.h"
#include "sstm.h"

#include <iostream>

__thread sstm_metadata_t sstm_meta;	 /* per-thread metadata */
__thread TSTMMemory* memory;

word tid = 1;
TSTMLockArray* locks;
GlobalClock gClock;
sstm_metadata_global_t sstm_meta_global; /* global metadata */

/* initializes the TM runtime 
   (e.g., allocates the locks that the system uses ) 
*/
void
sstm_start()
{
	locks = new TSTMLockArray(11, 2);
}

/* terminates the TM runtime
   (e.g., deallocates the locks that the system uses ) 
*/
void
sstm_stop()
{
	delete locks;
}


/* initializes thread local data
   (e.g., allocate a thread local counter)
 */
void
sstm_thread_start()
{
	sstm_meta.id = __sync_fetch_and_add(&tid, 1);
	std::cout << sstm_meta.id <<std::endl;

	memory = new TSTMMemory(sstm_meta.id, *locks, gClock);
}

/* terminates thread local data
   (e.g., deallocate a thread local counter)
****** DO NOT CHANGE THE EXISTING CODE*********
****** Feel free to add more code *************
 */
void
sstm_thread_stop()
{
	delete memory;
  __sync_fetch_and_add(&sstm_meta_global.n_commits, sstm_meta.n_commits);
  __sync_fetch_and_add(&sstm_meta_global.n_aborts, sstm_meta.n_aborts);
}

extern void sstm_tx_init() {
	memory->start();
}

/* transactionally reads the value of addr
 * On a more complex than GL-STM algorithm,
 * you need to do more work than simply reading the value.
*/
uintptr_t sstm_tx_load(volatile uintptr_t* addr)
{
	return memory->read(addr);
}

/* transactionally writes val in addr
 * On a more complex than GL-STM algorithm,
 * you need to do more work than simply reading the value.
*/
void sstm_tx_store(volatile uintptr_t* addr, uintptr_t val)
{
	return memory->write(addr, val);
}

/* cleaning up in case of an abort 
   (e.g., flush the read or write logs)
*/
void
sstm_tx_cleanup()
{
	memory->rollback();
	sstm_alloc_on_abort();
	sstm_meta.n_aborts++;
}

/* tries to commit a transaction
   (e.g., validates some version number, and/or
   acquires a couple of locks)
 */
void
sstm_tx_commit()
{
	memory->save();
	sstm_alloc_on_commit();
	sstm_meta.n_commits++;		
}


/* prints the TM system stats
****** DO NOT TOUCH *********
*/
void
sstm_print_stats(double dur_s)
{
  printf("# Commits: %-10zu - %.0f /s\n",
	 sstm_meta_global.n_commits,
	 sstm_meta_global.n_commits / dur_s);
  printf("# Aborts : %-10zu - %.0f /s\n",
	 sstm_meta_global.n_aborts,
	 sstm_meta_global.n_aborts / dur_s);
}
