#include <replacement.h>

#include <definitions.h>

#include <pointer_chasing.h>
#include <memory_access.h>
#include <cpu_instructions.h>
#include <paging.h>

#include <linux/slab.h>
#include <linux/random.h>

int **calculate_permutation_vectors(struct translation_cache *translation_caches, int level)
{
	unsigned long target_address;
	struct translation_cache *current_translation_cache = &translation_caches[level];
	unsigned long *initial_order, *current_order;

	int **permutation_vectors = kmalloc(current_translation_cache->ways * sizeof(int *), GFP_KERNEL);

	for(int i = 0; i < current_translation_cache->ways; i++)
	{
		permutation_vectors[i] = kmalloc(current_translation_cache->ways * sizeof(int), GFP_KERNEL);
		for(int j = 0; j < current_translation_cache->ways; j++)
			permutation_vectors[i][j] = -1;
	}

	target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % (PAGE_TABLE_ENTRIES[level] / (1 << (7 - level)))) << PAGE_TABLE_INDEX_SHIFT[level]);

	build_pointer_chain_from_hash_single(target_address, 0, current_translation_cache->ways - 1, current_translation_cache->hash_function, current_translation_cache->set_bits, level);

	initial_order = determine_replacement_order(translation_caches, target_address, level);

	if(!initial_order)
	{
		printk(KERN_DEBUG"[-] Could not determine initial order\n");
		return NULL;
	}

	for(int i = 0; i < current_translation_cache->ways; i++)
	{
		if(initial_order[i] == 0x0)
		{
			printk(KERN_DEBUG"[-] There was an error when determining the initial_order of entries in the level %d translation cache\n", level);
			printk(KERN_DEBUG"[*] Skipping permutation vector %d\n", i);
			continue;
		}

		// trigger permutation i - actually we need to access initial_order[i] + some_offset, because initial_order[i] and initial_order[i] + (0x1 << X) were already accessed before.
		build_pointer_chain_from_hash_single(target_address, initial_order[i] + (0x2ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]), current_translation_cache->ways - 1, current_translation_cache->hash_function, current_translation_cache->set_bits, level);
		write_data(initial_order[i] + (0x2ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]), 0);

		current_order = determine_replacement_order(translation_caches, target_address, level);

		for(int j = 0; j < current_translation_cache->ways; j++)
		{
			for(int k = 0; k < current_translation_cache-> ways; k++)
			{
				if(initial_order[k] == current_order[j])
				{
					permutation_vectors[i][j] = k;
					break;
				}
			}
		}
		kfree(current_order);
	}
	kfree(initial_order);

	return permutation_vectors;
}

unsigned long *determine_replacement_order(struct translation_cache *translation_caches, unsigned long set_start, int level)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	unsigned long last_address, next_address;

	unsigned long *address_set, *replacement_order;
	unsigned int *misses;

	int skip;
	unsigned long p;

	struct translation_cache *current_translation_cache = &translation_caches[level];

	address_set = kmalloc(current_translation_cache->ways * sizeof(unsigned long), GFP_KERNEL);
	replacement_order = kmalloc(current_translation_cache->ways * sizeof(unsigned long), GFP_KERNEL);
	misses = kmalloc(current_translation_cache->ways * sizeof(unsigned int), GFP_KERNEL);

	p = set_start;

	while(p != 0)
	{
		last_address = p;
		p = read_data(p);
	}

	p = set_start;

	for(int i = 0; i < current_translation_cache->ways; i++)
		address_set[i] = current_translation_cache->hash_function(set_start, i, current_translation_cache->set_bits, level);


	next_address = current_translation_cache->hash_function(set_start, current_translation_cache->ways, current_translation_cache->set_bits, level);
	write_data(last_address, next_address);


	for(int position = current_translation_cache->ways - 1; position >= 0; position--)
	{
		for(int i = 0; i < current_translation_cache->ways; i++)
			misses[i] = 0;

		for(int target = 0; target < current_translation_cache->ways; target++)
		{
			skip = 0;
			for(int i = 0; i < current_translation_cache->ways; i++)
				if(address_set[target] == replacement_order[i])
					skip = 1;

			if(skip)
				continue;

			target_address = address_set[target];
			prefix_aligned_address = target_address + (0x1ffULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
			complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

			walk_page_table(target_address, &target_walk);
			walk_page_table(complementary_address, &complementary_walk);

			clear_nx(&target_walk);
			clear_nx(&complementary_walk);
		
			build_pointer_chain_from_hash_single(next_address, prefix_aligned_address, current_translation_cache->ways - position - 1, current_translation_cache->hash_function, current_translation_cache->set_bits, level);

			claim_cpu();
			for(int i = 0; i < ITERATIONS; i++)
			{
				fence();

				flush_mmu_caches();

				fence();

				for(int j = 0; j < current_translation_cache->ways * (1 << current_translation_cache->set_bits) * 4; j++)
				{
					read_data(BASE_ADDRESS + ((get_random_long() % PAGE_TABLE_ENTRIES[level]) << PAGE_TABLE_INDEX_SHIFT[level]));
					fence();
				}

				p = set_start;

				while(p != last_address)
				{
					p = read_data(p);
					fence();
				}

				p = read_data(p);
				fence();

				switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
				fence();

				while(p != prefix_aligned_address)
				{
					p = read_data(p);
					fence();
				}

				if(read_data(p) == SIGNAL)
					misses[target]++;

				fence();

				switch_page_table_entries(target_walk.page_table_entries[level], complementary_walk.page_table_entries[level]);
			}
			flush_mmu_caches();
			release_cpu();
		}

//		printk("[*] Position %d:\n", position);
		for(int i = 0; i < current_translation_cache->ways; i++)
		{
//			printk("[*] 	%d: %d\n", i, misses[i]);
			if(misses[i] >= ITERATIONS * THRESHOLD)
			{
				replacement_order[position] = address_set[i];
				break;
			}
		}
	}
	kfree(address_set);
	kfree(misses);

	return replacement_order;
}
