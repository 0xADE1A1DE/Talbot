#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <translation_cache.h>

int **calculate_permutation_vectors(struct translation_cache *translation_caches, int level);
unsigned long *determine_replacement_order(struct translation_cache *translation_caches, unsigned long set_start, int level);

#endif