#include <hash_functions.h>

#include <definitions.h>

#include <linux/kernel.h>

int xor_get_set(unsigned long address, int set_bits, int level);



unsigned long lin_hash(unsigned long target_address, int offset, int set_bits, int level)
{
	int sets = 1 << set_bits;

	return (target_address + ((uint64_t)(sets * offset) << PAGE_TABLE_INDEX_SHIFT[level]));
}

int xor_get_set(unsigned long address, int set_bits, int level)
{
	return ((address >> (PAGE_TABLE_INDEX_SHIFT[level] + set_bits)) ^ ((address >> (PAGE_TABLE_INDEX_SHIFT[level])))) & ((1 << set_bits) - 1);
}

unsigned long xor_hash(unsigned long target_address, int offset, int set_bits, int level)
{
	int target_set = xor_get_set(target_address, set_bits, level);

	unsigned long left_half = ((target_address >> (PAGE_TABLE_INDEX_SHIFT[level] + set_bits)) + offset);
	unsigned long lower_set_bits = (left_half & ((1 << set_bits) - 1)) ^ target_set;
	unsigned long right_half = target_address & (((uint64_t)1 << (PAGE_TABLE_INDEX_SHIFT[level])) - 1);

	unsigned long address = ((left_half << (PAGE_TABLE_INDEX_SHIFT[level] + set_bits)) | (lower_set_bits << (PAGE_TABLE_INDEX_SHIFT[level])) | right_half);

	return address;
}

unsigned long lin_shift_hash(unsigned long target_address, int offset, int set_bits, int level)
{
	int sets = 1 << set_bits;

	int dangling_bit = (target_address >> PAGE_TABLE_INDEX_SHIFT[level]) & 0x01;

	unsigned long page_table_index = ((sets * ((offset + dangling_bit) / 2)) << 1) + ((offset + dangling_bit) % 2);

	return ((target_address & ~(1ULL << (PAGE_TABLE_INDEX_SHIFT[level]))) + (page_table_index << PAGE_TABLE_INDEX_SHIFT[level]));
}
