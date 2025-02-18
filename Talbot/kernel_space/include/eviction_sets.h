#ifndef EVICTION_SETS_H
#define EVICTION_SETS_H

#include <hash_functions.h>

int find_minimal_eviction_set(unsigned long target_address, int level, unsigned long (*access_memory)(volatile unsigned long));
int eviction_set_aligns_with_hash_function(unsigned long target_address, int level, unsigned long (*access_memory)(volatile unsigned long), HashFunction hash, int set_bits, int ways);

#endif
