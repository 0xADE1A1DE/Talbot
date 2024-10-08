#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include <translation_cache.h>
#include <hash_functions.h>

int test_cache_existence(int level);

int test_split_layer_instructions(int level);
int test_split_layer_data(int level);
int test_shared_layer(int level);

int test_hash_function(int ways, int set_bits, HashFunction hash_function, int level);
int test_nesting(int level, struct translation_cache *translation_caches);
int test_huge_tlb(int level);
void test_huge_page_eviction(int level);
int test_supported_pcids(int level, int amount_pcids, int noflush);
int test_pcid_respect(int level);
int test_hyperthreaded_pcid_support(int level);


#endif
