#include <eviction_sets.h>

#include <definitions.h>
#include <paging.h>
#include <memory_access.h>
#include <cpu_instructions.h>
#include <pointer_chasing.h>

int chain_evicts(unsigned long target_address, unsigned long prefix_aligned_address, int level, unsigned long (*access_memory)(volatile unsigned long));

int chain_evicts(unsigned long target_address, unsigned long prefix_aligned_address, int level, unsigned long (*access_memory)(volatile unsigned long))
{
	unsigned long complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);
	struct page_table_walk target_walk, complementary_walk;

	uint misses = 0;

	volatile unsigned long p;

	walk_page_table(complementary_address, &complementary_walk);
	clear_nx(&complementary_walk);

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		walk_page_table(target_address, &target_walk);
		clear_nx(&target_walk);
		
		fence();

		flush_mmu_caches();

		fence();

		p = access_memory(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		while(p != prefix_aligned_address)
		{
			p = access_memory(p);
			fence();
		}

		if(access_memory(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();

		fence();
	}

	release_cpu();
	return !!(misses >= ITERATIONS * THRESHOLD);
}

int find_minimal_eviction_set(unsigned long target_address, int level, unsigned long (*access_memory)(volatile unsigned long))
{
	unsigned long prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]), last_mandatory_address;

	int length, size = 2;

	for(length = 2; length < 512; length++)
	{
		build_pointer_chain(target_address, prefix_aligned_address, length, level);
		if(chain_evicts(target_address, prefix_aligned_address, level, access_memory))
			break;
	}

	if(!chain_evicts(target_address, prefix_aligned_address, level, access_memory))
	{
		printk(KERN_DEBUG"[-] Did not find eviction set for 0x%lx!\n", target_address);
		return -1;
	}

	last_mandatory_address = target_address;

	for(uint64_t i = 1; i < length; i++)
	{
		write_data(last_mandatory_address, target_address + ((i + 1ULL) << PAGE_TABLE_INDEX_SHIFT[level]));
		if(!chain_evicts(target_address, prefix_aligned_address, level, access_memory))
		{
			write_data(last_mandatory_address, target_address + (i << PAGE_TABLE_INDEX_SHIFT[level]));
			last_mandatory_address = target_address + (i << PAGE_TABLE_INDEX_SHIFT[level]) + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
			size++;
		}
	}
	return size;
}


int eviction_set_aligns_with_hash_function(unsigned long target_address, int level, unsigned long (*access_memory)(volatile unsigned long), HashFunction hash, int set_bits, int ways)
{
	unsigned long prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]), last_mandatory_address;

	int length = (1 << set_bits) * ways, size = 2;

	build_pointer_chain(target_address, prefix_aligned_address, length, level);

	if(!chain_evicts(target_address, prefix_aligned_address, level, access_memory))
	{
		if(hash)
			return 0;
		return 1;
	}

	last_mandatory_address = target_address;

	for(uint64_t i = 1; i < length; i++)
	{
		write_data(last_mandatory_address, target_address + ((i + 1ULL) << PAGE_TABLE_INDEX_SHIFT[level]));
		if(!chain_evicts(target_address, prefix_aligned_address, level, access_memory))
		{
			if(hash(target_address, size - 1, set_bits, level) != (target_address + (i << PAGE_TABLE_INDEX_SHIFT[level])))
				return 0;
			write_data(last_mandatory_address, target_address + (i << PAGE_TABLE_INDEX_SHIFT[level]));
			last_mandatory_address = target_address + (i << PAGE_TABLE_INDEX_SHIFT[level]) + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
			size++;
		}
	}
	if(size == ways + 1)
		return 1;
	return 0;
}