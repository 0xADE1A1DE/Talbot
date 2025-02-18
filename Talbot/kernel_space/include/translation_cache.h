#ifndef TRANSLATION_CACHE_H
#define TRANSLATION_CACHE_H

#include <hash_functions.h>

struct translation_cache
{
	int set_bits;
	int ways;
	HashFunction hash_function;
};

#endif