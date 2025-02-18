#include <pointer_chasing.h>

#include <definitions.h>
#include <memory_access.h>
#include <eviction_sets.h>

void build_pointer_chain(unsigned long start_address, unsigned long end_address, int length, int level)
{
	unsigned long last_chain_link = start_address, chain_link;

//	write_data(start_address, start_address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]));
//	last_chain_link = start_address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);

	for(uint64_t i = 0; i < length; i++)
	{
		chain_link = start_address + ((i + 1) << PAGE_TABLE_INDEX_SHIFT[level]);

		if(!(chain_link > BASE_ADDRESS && chain_link < BASE_ADDRESS + ((uint64_t)PAGE_TABLE_ENTRIES[level] << PAGE_TABLE_INDEX_SHIFT[level])))
		{
			//printk("[-] Not enough memory mapped. Consider mapping more entries on level %d!\n", level);
			//printk("[*] Subsequent experiments may not work as intended!");
			break;
		}

		write_data(last_chain_link, chain_link);

		last_chain_link = chain_link;

/*		chain_link = last_chain_link + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);
		write_data(last_chain_link, chain_link);

		last_chain_link = chain_link;*/
	}

	write_data(last_chain_link, end_address);
}

void build_huge_chain(unsigned long start_address, unsigned long end_address, int length, int level)
{
	unsigned long page_offset = 0;
	unsigned long last_chain_link = start_address, chain_link;
	for(uint64_t i = 0; i < length; i++)
	{
		chain_link = start_address + ((i + 1) << PAGE_TABLE_INDEX_SHIFT[level]) + page_offset;
		write_data(last_chain_link, chain_link);

		last_chain_link = chain_link;
		page_offset += 16;
	}

	write_data(last_chain_link, end_address);
}

// touches each entry twice because??? needed
// TODO: function needs renaming as it is inaccurate due to touching each entry twice
void build_pointer_chain_from_hash(unsigned long start_address, unsigned long end_address, int length, HashFunction hash_function, int set_bits, int level)
{
	int offset = 0;
	unsigned long last_chain_link = start_address, chain_link;

//	write_data(start_address, start_address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]));
//	last_chain_link = start_address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);

	for(int i = 0; i < length; i++)
	{
		offset++;
		chain_link = hash_function(start_address, offset, set_bits, level);

		if(!(chain_link > BASE_ADDRESS && chain_link < BASE_ADDRESS + ((uint64_t)PAGE_TABLE_ENTRIES[level] << PAGE_TABLE_INDEX_SHIFT[level])))
		{
			//printk("[-] Not enough memory mapped. Consider mapping more entries on level %d!\n", level);
			//printk("[*] Subsequent experiments may not work as intended!");
			break;
		}

		write_data(last_chain_link, chain_link);

		last_chain_link = chain_link;

/*		chain_link = last_chain_link + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);

		write_data(last_chain_link, chain_link);
		last_chain_link = chain_link;*/
	}

	write_data(last_chain_link, end_address);
}

void build_pointer_chain_from_hash_single(unsigned long start_address, unsigned long end_address, int length, HashFunction hash_function, int set_bits, int level)
{
	int offset = 0;
	unsigned long last_chain_link = start_address, chain_link;

	write_data(start_address, start_address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]));
	last_chain_link = start_address + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);

	for(int i = 0; i < length; i++)
	{
		offset++;
		chain_link = hash_function(start_address, offset, set_bits, level);

		if(!(chain_link > BASE_ADDRESS && chain_link < BASE_ADDRESS + ((uint64_t)PAGE_TABLE_ENTRIES[level] << PAGE_TABLE_INDEX_SHIFT[level])))
		{
			//printk("[-] Not enough memory mapped. Consider mapping more entries on level %d!\n", level);
			//printk("[*] Subsequent experiments may not work as intended!");
			break;
		}

		write_data(last_chain_link, chain_link);
		last_chain_link = chain_link;

/*		chain_link = last_chain_link + (0x1ULL << PAGE_TABLE_INDEX_SHIFT[level - 1]);

		write_data(last_chain_link, chain_link);
		last_chain_link = chain_link;*/
	}

	write_data(last_chain_link, end_address);
}

/*void build_eviction_chain(unsigned long start_address, unsigned long end_address, memory_access_function access_memory)
{

}*/
