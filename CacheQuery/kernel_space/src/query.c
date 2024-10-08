#include <query.h>

#include <definitions.h>
#include <hash_functions.h>
#include <memory_access.h>
#include <paging.h>
#include <cpu_instructions.h>
#include <kernel_functions.h>

#include <linux/random.h>
#include <linux/slab.h>

#define LEVEL (2)
#define SET_BITS (3)
#define WAYS (4)

static HashFunction hash = lin_shift_hash;

struct address_map_entry
{
	char letter;
	unsigned long address;
};

typedef struct address_map_entry* address_map;



int address_map_contains_letter(char letter, address_map map, size_t map_size);
unsigned long get_next_address(char letter, address_map map, size_t map_size, size_t amount_unique_letters);
void build_chain_from_address_map(address_map map, unsigned long target, size_t map_size);



int address_map_contains_letter(char letter, address_map map, size_t map_size)
{
	for(int i = 0; i < map_size; i++)
		if(map[i].letter == letter)
			return 1;
	
	return 0;
}

unsigned long get_next_address(char letter, address_map map, size_t map_size, size_t amount_unique_letters)
{
	// First letter
	if(map_size == 0)
		return (BASE_ADDRESS + ((uint64_t)(get_random_long() % (PAGE_TABLE_ENTRIES[LEVEL] / (1 << (7 - LEVEL)))) << PAGE_TABLE_INDEX_SHIFT[LEVEL]));

	// Multiple accessed letter
	for(int i = map_size - 1; i >= 0; i--)
		if(map[i].letter == letter)
			return (map[i].address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[LEVEL - 1]));

	// New letter
	return hash(map[0].address, amount_unique_letters - 1, SET_BITS, LEVEL);
}

void build_chain_from_address_map(address_map map, unsigned long target, size_t map_size)
{
	unsigned long chain_link, last_chain_link = map[0].address;

	for(int i = 1; i < map_size; i++)
	{
		chain_link = map[i].address;
		write_data(last_chain_link, chain_link);
		last_chain_link = chain_link;
	}

	write_data(last_chain_link, target);
}

char *query_cache(const char *query, size_t query_size)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	char *unique_letters = kmalloc(sizeof(char) * query_size, GFP_KERNEL);
	size_t amount_unique_letters = 0;

	char *final_state = kmalloc(sizeof(char) * query_size, GFP_KERNEL);
	size_t final_state_length = 0;

	address_map map = kmalloc(sizeof(struct address_map_entry) * query_size, GFP_KERNEL);
	size_t map_size = 0;

	int misses = 0;
	unsigned long p;

	for(int i = 0; i < query_size; i++)
	{
		if(!address_map_contains_letter(query[i], map, map_size))
		{
			unique_letters[amount_unique_letters] = query[i];
			amount_unique_letters++;
		}

		map[map_size].letter = query[i];
		map[map_size].address = get_next_address(map[map_size].letter, map, map_size, amount_unique_letters);

		map_size++;
	}

	disable_smep();

	for(int i = 0; i < amount_unique_letters; i++)
	{
		for(int j = 0; j < map_size; j++)
		{
			if(map[j].letter == unique_letters[i])
			{
				target_address = map[j].address;
				break;
			}
		}

		prefix_aligned_address = target_address + (0x1ffULL << PAGE_TABLE_INDEX_SHIFT[LEVEL - 1]);
		complementary_address = COMPLEMENTARY_BASE_ADDRESS + (target_address - BASE_ADDRESS);

		walk_page_table(target_address, &target_walk);
		walk_page_table(complementary_address, &complementary_walk);

		clear_nx(&target_walk);
		clear_nx(&complementary_walk);

		build_chain_from_address_map(map, prefix_aligned_address, map_size);

		misses = 0;

		claim_cpu();
		for(int j = 0; j < ITERATIONS; j++)
		{
			flush_mmu_caches();

			fence();

			for(int k = 0; k < WAYS * (1 << SET_BITS) * 4; k++)
			{
				read_data(BASE_ADDRESS + ((get_random_long() % PAGE_TABLE_ENTRIES[LEVEL]) << PAGE_TABLE_INDEX_SHIFT[LEVEL]));
				fence();
			}

			fence();

			p = read_data(map[0].address);

			fence();

			while(p != prefix_aligned_address)
				p = read_data(p);

			fence();

			switch_page_table_entries(target_walk.page_table_entries[LEVEL], complementary_walk.page_table_entries[LEVEL]);

			fence();

			if(read_data(p) == SIGNAL)
				misses += 1;

			fence();

			switch_page_table_entries(target_walk.page_table_entries[LEVEL], complementary_walk.page_table_entries[LEVEL]);

			fence();
		}
		release_cpu();

//		printk("[*] Misses %c: %d\n", unique_letters[i], misses);

		if(misses < ITERATIONS * THRESHOLD)
		{
			final_state[final_state_length] = unique_letters[i];
			final_state_length++;
		}
	}

	kfree(unique_letters);
	kfree(map);

	final_state[final_state_length] = 0;
	return final_state;
}
