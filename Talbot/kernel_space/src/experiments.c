#include <experiments.h>

#include <definitions.h>
#include <tlb.h>

#include <paging.h>
#include <memory_access.h>
#include <cpu_instructions.h>
#include <pointer_chasing.h>
#include <kernel_functions.h>
#include <replacement.h>

#include <linux/random.h>
#include <linux/slab.h>

int test_cache_existence(int level)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	volatile unsigned long p;

	int misses = 0;

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);
		
		write_data(target_address, prefix_aligned_address);

		fence();

		flush_mmu_caches();

		fence();

		p = read_data(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();

		fence();
	}

	release_cpu();
	return misses;
}

int test_split_layer_instructions(int level)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	volatile unsigned long p;
	int misses = 0;

	disable_smep();

	claim_cpu();

	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);

		build_pointer_chain(target_address, prefix_aligned_address, MAXIMUM_ENTRIES, level);

		fence();

		flush_mmu_caches();

		fence();

		for(int j = 0; j < 128; j++)
		{
			read_data(BASE_ADDRESS + ((get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]));
			fence();
		}

		fence();

		p = fetch_instruction(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		while(p != prefix_aligned_address)
		{
			p = read_data(p);
			fence();
		}

		if(fetch_instruction(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();
	}

	release_cpu();
	return misses;
}

int test_split_layer_data(int level)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	volatile unsigned long p;
	int misses = 0;

	disable_smep();

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);

		build_pointer_chain(target_address, prefix_aligned_address, MAXIMUM_ENTRIES, level);

		fence();

		flush_mmu_caches();

		fence();

		for(int j = 0; j < 128; j++)
		{
			read_data(BASE_ADDRESS + ((get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]));
			fence();
		}

		fence();

		p = read_data(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		while(p != prefix_aligned_address)
		{
			p = fetch_instruction(p);
			fence();
		}

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();

		fence();
	}

	release_cpu();
	return misses;
}

int test_shared_layer(int level)
{
	unsigned long target_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	volatile unsigned long p;

	int misses = 0;

	disable_smep();

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);
		
		write_data(target_address, target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]));

		fence();

		flush_mmu_caches();

		fence();

		p = fetch_instruction(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();
	}

	release_cpu();
	return misses;
}

int test_hash_function(int ways, int set_bits, HashFunction hash_function, int level)
{
	unsigned long start_address, target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	unsigned int misses = 0;

	unsigned long p;

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		// 2nd level: 16384/32 -> 512; 3rd level: 512/16 -> 32; 4th level: 32/8 -> 4;
		start_address = BASE_ADDRESS + ((get_random_long() % (PAGE_TABLE_ENTRIES[level] / (1 << (7 - level)))) << PAGE_TABLE_INDEX_SHIFT[level]);

		for(int j = 0; j < ways; j++)// due to this, it seems like we are getting conflicting results because ways = 0 will never produce hits, as this loop never runs.
		{
			target_address = hash_function(start_address, j, set_bits, level);
			if(target_address >= BASE_ADDRESS + ((uint64_t)PAGE_TABLE_ENTRIES[level] << PAGE_TABLE_INDEX_SHIFT[level]))
				continue;

			prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]);
			complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

			walk_page_table(target_address, &target_walk);
			walk_page_table(complementary_address, &complementary_walk);

			clear_nx(&target_walk);
			clear_nx(&complementary_walk);

			build_pointer_chain_from_hash(start_address, prefix_aligned_address, ways, hash_function, set_bits, level);

			fence();

			flush_mmu_caches();

			fence();

			p = read_data(start_address);

			fence();

			while(p != prefix_aligned_address)
			{
				p = read_data(p);
				fence();
			}

			fence();

			switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

			fence();

			if(read_data(p) == SIGNAL)
			{
				misses += 1;

				fence();

				switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
				flush_mmu_caches();

				fence();

				break;
			}

			fence();

			switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
			flush_mmu_caches();

			fence();
		}
	}
	release_cpu();

	return misses;
}

int test_nesting(int level, struct translation_cache *translation_caches)
{
	unsigned long target_address, prefix_aligned_address, complementary_address, isolated_upper_level_target;
	struct page_table_walk target_walk, complementary_walk;

	struct translation_cache *inclusive_translation_cache = &translation_caches[level + 1];

	volatile unsigned long p;

	int misses = 0;

	if(level > 3)
	{
		printk(KERN_DEBUG"[-] Level %d translation cache cannot be nested, as no upper level translation cache exists\n", level);
		return -1;
	}

	if(inclusive_translation_cache->hash_function == NULL)
	{
		printk(KERN_DEBUG"[-] Nesting: Hash function of level %d undefined but required\n", level + 1);
		return -1;
	}

	disable_smep();

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((get_random_long() % (PAGE_TABLE_ENTRIES[level] / (1 << (7 - (level))))) << PAGE_TABLE_INDEX_SHIFT[level]);
		isolated_upper_level_target = target_address & ~((1ULL << PAGE_TABLE_INDEX_SHIFT[level + 1]) - 1);
		prefix_aligned_address = target_address + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);
		
		build_pointer_chain_from_hash(isolated_upper_level_target, prefix_aligned_address, inclusive_translation_cache->ways, inclusive_translation_cache->hash_function, inclusive_translation_cache->set_bits, level + 1);
		write_data(target_address, read_data(isolated_upper_level_target));

		fence();

		flush_mmu_caches();

		fence();

		p = read_data(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		while(p != prefix_aligned_address)
		{
			p = read_data(p);
			fence();
		}

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();
	}

	release_cpu();
	return misses;
}

int test_huge_tlb(int level)
{
	unsigned long huge_base = (level == 2) ? HUGE_2MB_BASE_ADDRESS : HUGE_1GB_BASE_ADDRESS;
	unsigned int amount_hugepages = (level == 2) ? AMOUNT_2M_PAGES : AMOUNT_1G_PAGES;
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	//struct translation_cache *current_translation_cache = &translation_caches[level];

	int misses = 0;
	unsigned long p;

	if(level != 2 && level != 3)
	{
		printk("[-] Huge sTLB test not well defined for provided parameters!\n");
		return -1;
	}

	disable_smep();

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		prefix_aligned_address = target_address + (0x1ffULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);
		
		build_huge_chain(huge_base, prefix_aligned_address, amount_hugepages - 1, level);
		write_data(target_address, huge_base);

		fence();

		flush_mmu_caches();

		fence();

		p = read_data(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		while(p != prefix_aligned_address)
		{
			p = read_data(p);
			fence();
		}

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();

		fence();
	}

	flush_mmu_caches();

	release_cpu();
	return misses;
}

void test_huge_page_eviction(int level)
{
	unsigned long huge_base = (level == 2) ? HUGE_2MB_BASE_ADDRESS : HUGE_1GB_BASE_ADDRESS;
	unsigned int amount_hugepages = (level == 2) ? AMOUNT_2M_PAGES : AMOUNT_1G_PAGES;
	unsigned long target_page, complementary_address;

	struct page_table_walk target_walk, complementary_walk;

	int misses = 0;
	unsigned long p;

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_page = huge_base;
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_page - huge_base);

		if(walk_page_table(target_page, &target_walk) != -(level - 1))
			printk("[-] Target address 0x%lx does not map an appropriate huge page\n", target_page);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);

		build_huge_chain(target_page, target_page, amount_hugepages - 1, level);

		fence();

		flush_mmu_caches();

		fence();

		p = read_data(target_page);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		while(p != target_page)
		{
			p = read_data(p);
			fence();
		}

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		flush_mmu_caches();

		fence();
	}

	release_cpu();
	printk(KERN_INFO"[*] Huge page evictions: %d/%d\n", misses, ITERATIONS);
}

int test_supported_pcids(int level, int amount_pcids, int noflush)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	int misses = 0;
	unsigned long p;

	uint64_t cr3 = (get_cr3() >> 12 ) << 12;
	uint64_t noflush_bit = 0ULL;

	if(noflush)
		noflush_bit = CR3_NOFLUSH;

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		prefix_aligned_address = target_address + (0x1ffULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);

		write_data(target_address, prefix_aligned_address);

		fence();
		
		set_cr3(cr3);
		flush_mmu_caches();

		fence();

		p = read_data(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		for(int pcid = 1; pcid <= amount_pcids; pcid++)
		{
			set_cr3(cr3 | pcid | noflush_bit);
			fence();
		}

		set_cr3(cr3 | noflush_bit);

		fence();

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		set_cr3(cr3);
		flush_mmu_caches();

		fence();
	}

	release_cpu();
	return misses;
}

int test_pcid_respect(int level)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	int misses = 0;
	unsigned long p;
	uint16_t pcid = 0;

	uint64_t cr3 = (get_cr3() >> 12 ) << 12;

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]);
		prefix_aligned_address = target_address + (0x1ffULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		do {
			pcid = get_random_long() % 4096;
		} while(pcid == 0);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);

		write_data(target_address, prefix_aligned_address);

		fence();
		
		set_cr3(cr3);
		flush_mmu_caches();

		fence();

		p = read_data(target_address);

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);

		fence();

		set_cr3(cr3 | pcid | CR3_NOFLUSH);
		fence();

		if(read_data(p) == SIGNAL)
			misses += 1;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
		set_cr3(cr3);
		flush_mmu_caches();

		fence();
	}

	release_cpu();
	return misses;
}