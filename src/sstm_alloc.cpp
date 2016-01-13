#include "sstm_alloc.h"

/* allocate some memory within a transaction
*/
void*
sstm_tx_alloc(size_t size)
{
  void* m = malloc(size);

  /* 
     you need to keep track of allocations, so that if a TX
     aborts, you free that memory
   */

  return m;
}

/* free some memory within a transaction
*/
void
sstm_tx_free(void* mem)
{
  /* 
     in a more complex STM systems,
     you cannot immediately free(mem) as below because you might 
     have TX aborts. You need to keep track of mem frees
     and only make the actual free happen if the TX is commited
  */

  free(mem);
}

/* this function is executed when a transaction is aborted.
 * Purpose: (1) free any memory that was allocated during the
 * transaction that was just aborted, (2) clean-up any freed memory
 * references that were buffered during the transaction
*/
void
sstm_alloc_on_abort()
{

}

/* this function is executed when a transaction is committed.
 * Purpose: (1) free any memory that was freed during the
 * transaction, (2) clean-up any allocated memory
 * references that were buffered during the transaction
*/
void
sstm_alloc_on_commit()
{

}
