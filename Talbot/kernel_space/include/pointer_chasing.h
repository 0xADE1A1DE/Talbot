#ifndef POINTER_CHASING_H
#define POINTER_CHASING_H

#include <hash_functions.h>

void build_pointer_chain(unsigned long start_address, unsigned long end_address, int length, int level);
void build_huge_chain(unsigned long start_address, unsigned long end_address, int length, int level);
void build_pointer_chain_from_hash(unsigned long start_address, unsigned long end_address, int length, HashFunction hash_function, int set_bits, int level);
void build_pointer_chain_from_hash_single(unsigned long start_address, unsigned long end_address, int length, HashFunction hash_function, int set_bits, int level);

#endif
