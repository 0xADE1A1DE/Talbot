#ifndef EVALUATION_H
#define EVALUATION_H

#include <translation_cache.h>

char *evaluate_cache_existence(int level);
char *evaluate_cache_hierarchy(int level);
char *evaluate_cache_structure(struct translation_cache *translation_caches, int level);
char *evaluate_replacement_policy(struct translation_cache *translation_caches, int level);
char *evaluate_nesting(struct translation_cache *translation_caches, int level);
char *evaluate_huge_TLB(int level);
char *evaluate_supported_pcids(int level);
char *evaluate_pcid_respect(int level);

#endif