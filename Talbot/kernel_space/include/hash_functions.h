#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

// TODO: change parameter types as needed
typedef unsigned long (*HashFunction)(unsigned long, int, int, int);

unsigned long lin_hash(unsigned long target_address, int offset, int set_bits, int level);
unsigned long xor_hash(unsigned long target_address, int offset, int set_bits, int level);
unsigned long lin_shift_hash(unsigned long target_address, int offset, int set_bits, int level);

#endif
