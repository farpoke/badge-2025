#include <stdlib.h>

#include <lib/oofatfs/ff.h>

void* ff_memalloc (size_t msize) { return malloc(msize); }
void ff_memfree (void* mblock) { free(mblock); }
