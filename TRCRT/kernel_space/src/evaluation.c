#include <evaluation.h>

#include <definitions.h>

#include <experiments.h>
#include <hash_functions.h>
#include <eviction_sets.h>
#include <memory_access.h>
#include <replacement.h>

#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/slab.h>

char *evaluate_cache_existence(int level)
{
	int misses = test_cache_existence(level);
	char *output = kzalloc(128, GFP_KERNEL);

	if(misses >= ITERATIONS * THRESHOLD)
		snprintf(output, 128, "[*] Level %d translation cache does not exist (%d/%d)!\n", level, misses, ITERATIONS);
	else
		snprintf(output, 128, "[*] Level %d translation cache does exist (%d/%d)!\n", level, ITERATIONS - misses, ITERATIONS);

	return output;
}

char *evaluate_cache_hierarchy(int level)
{
	int misses = 0;
	char *output = kzalloc(256, GFP_KERNEL);
	char *tmp = kzalloc(64, GFP_KERNEL);

	snprintf(tmp, 64, "[*] Level %d translation cache:\n", level);

	strcat(output, tmp);

	misses = test_split_layer_instructions(level);
	snprintf(tmp, 64, "[*]	Level %d split L1i (%d/%d)\n", level, ITERATIONS - misses, ITERATIONS);

	strcat(output, tmp);

	misses = test_split_layer_data(level);
	snprintf(tmp, 64, "[*]	Level %d split L1d (%d/%d)\n", level, ITERATIONS - misses, ITERATIONS);

	strcat(output, tmp);

	misses = test_shared_layer(level);
	snprintf(tmp, 64, "[*]	Level %d shared L2 (%d/%d)\n", level, ITERATIONS - misses, ITERATIONS);

	strcat(output, tmp);

	kfree(tmp);
	return output;
}

char *evaluate_cache_structure(struct translation_cache *translation_caches, int level)
{
	char *output = kzalloc(1024*64, GFP_KERNEL);
	char *tmp = kzalloc(128, GFP_KERNEL);

	struct translation_cache *current_translation_cache = &translation_caches[level];

	unsigned long target_address, p;

	int minimal_eviction_set_size;

	int minimum_set_bits = current_translation_cache->set_bits;
	int minimum_ways = current_translation_cache->ways;

	int misses, certainty;
	int eviction_set_aligning = 0;

	HashFunction hash_functions[] = {lin_hash, lin_shift_hash/*, xor_hash*/};
	HashFunction hash_function;
	int n_hash_functions = sizeof(hash_functions) / sizeof(hash_functions[0]);

	for(int set_bits = 0; set_bits < 9; set_bits++)
	{
		for(int ways = 1; ways <= 32; ways++)
		{
			if(ways > minimum_ways)
				break;
		
			for(int i = 0; i < n_hash_functions; i++)
			{
				misses = test_hash_function(ways, set_bits, hash_functions[i], level);

				/*if(hash_functions[i] == lin_hash)
					snprintf(tmp, 128, "[*] Level %d translation cache structure: %d Sets, %d Ways, LIN hash (%d/%d).\n", level, 1 << set_bits, ways, misses, ITERATIONS);
				else if(hash_functions[i] == lin_shift_hash)
					snprintf(tmp, 128, "[*] Level %d translation cache structure: %d Sets, %d Ways, LIN>>1 hash (%d/%d).\n", level, 1 << set_bits, ways, misses, ITERATIONS);
				else if(hash_functions[i] == xor_hash)
					snprintf(tmp, 128, "[*] Level %d translation cache structure: %d Sets, %d Ways, XOR hash (%d/%d).\n", level, 1 << set_bits, ways, misses, ITERATIONS);
				else
					snprintf(tmp, 128, "[-] Level %d translation cache uses an unknown hash function!\n", level);

				strcat(output, tmp);*/

				if(misses >= ITERATIONS * THRESHOLD && (ways < minimum_ways || (ways <= minimum_ways && set_bits < minimum_set_bits)))
				{
					certainty = misses;

					minimum_ways = ways;
					minimum_set_bits = set_bits;
					hash_function = hash_functions[i];

					i = n_hash_functions;
					break;
				}
			}
		}
	}

	if(minimum_ways != current_translation_cache->ways || minimum_set_bits != current_translation_cache->set_bits)
	{
		current_translation_cache->set_bits = minimum_set_bits;
		current_translation_cache->ways = minimum_ways;
		current_translation_cache->hash_function = hash_function;
	}

	// Calculating eviction sets is expensive, so we only do this for 1/10th of ITERATIONS
	for(int i = 0; i < ITERATIONS/10; i++)
	{
		target_address = BASE_ADDRESS + ((get_random_long() % (PAGE_TABLE_ENTRIES[level] / (1 << (7 - level)))) << PAGE_TABLE_INDEX_SHIFT[level]);
		eviction_set_aligning += eviction_set_aligns_with_hash_function(target_address, level, read_data, hash_function, minimum_set_bits, minimum_ways);
	}

	if(hash_function == lin_hash)
		snprintf(tmp, 128, "[*] Level %d translation cache structure: %d Sets, %d Ways, LIN hash (%d/%d).\n", level, 1 << minimum_set_bits, minimum_ways, certainty, ITERATIONS);
	else if(hash_function == lin_shift_hash)
		snprintf(tmp, 128, "[*] Level %d translation cache structure: %d Sets, %d Ways, LIN>>1 hash (%d/%d).\n", level, 1 << minimum_set_bits, minimum_ways, certainty, ITERATIONS);
	else if(hash_function == xor_hash)
		snprintf(tmp, 128, "[*] Level %d translation cache structure: %d Sets, %d Ways, XOR hash (%d/%d).\n", level, 1 << minimum_set_bits, minimum_ways, certainty, ITERATIONS);
	else
		snprintf(tmp, 128, "[-] Level %d translation cache uses an unknown hash function!\n", level);

	strcat(output, tmp);

	if(eviction_set_aligning < (ITERATIONS / 10) * THRESHOLD)
	{
		snprintf(tmp, 128, "[-] Level %d hash function does not align with minimal eviction set! (%d/%d)\n", level, (ITERATIONS/10) - eviction_set_aligning, ITERATIONS / 10);
		strcat(output, tmp);
	}
	else
	{
		snprintf(tmp, 128, "[-] Level %d hash function and minimal eviction set align! (%d/%d)\n", level, eviction_set_aligning, ITERATIONS / 10);
		strcat(output, tmp);
	}

	if(PRINT_MINIMAL_EVICTION_SET)
	{
		for(uint64_t i = 0; i < 2; i++)
		{
			target_address = BASE_ADDRESS + ((get_random_long() % (PAGE_TABLE_ENTRIES[level] / (1 << (7 - level)))) << PAGE_TABLE_INDEX_SHIFT[level]);
			p = target_address;

			minimal_eviction_set_size = find_minimal_eviction_set(target_address, level, read_data);

			snprintf(tmp, 128, "[*] Minimal eviction set for 0x%lx:\n", target_address);
			strcat(output, tmp);

			for(int j = 0; j < minimal_eviction_set_size - 1; j++)
			{
				snprintf(tmp, 128, "[*] 0x%lx\n", p);
				strcat(output, tmp);
				p = read_data(p);

				if((p >> PAGE_TABLE_INDEX_SHIFT[level - 1]) & 0x1ff)
					p = read_data(p);
			}

			snprintf(tmp, 128, "[*] 0x%lx\n", p);
			strcat(output, tmp);

			target_address = p;
			minimal_eviction_set_size = find_minimal_eviction_set(target_address, level, read_data);

			snprintf(tmp, 128, "[*] Minimal eviction set for 0x%lx:\n", target_address);
			strcat(output, tmp);

			for(int j = 0; j < minimal_eviction_set_size - 1; j++)
			{
				snprintf(tmp, 128, "[*] 0x%lx\n", p);
				strcat(output, tmp);
				p = read_data(p);

				if((p >> PAGE_TABLE_INDEX_SHIFT[level - 1]) & 0x1ff)
					p = read_data(p);
			}

			snprintf(tmp, 128, "[*] 0x%lx\n", p);
			strcat(output, tmp);
		}
	}

	kfree(tmp);
	return output;
}

char *evaluate_replacement_policy(struct translation_cache *translation_caches, int level)
{
	char *output = kzalloc(1024*64, GFP_KERNEL);
	char *tmp = kzalloc(64, GFP_KERNEL);

	int **permutation_vectors = calculate_permutation_vectors(translation_caches, level);

	if(permutation_vectors == NULL)
	{
		snprintf(output, 64, "[-] Could not determine permutation vectors for level %d\n", level);
		kfree(tmp);
		return "";
	}

	snprintf(tmp, 64, "[*] Level %d permutation vectors:\n", level);
	strcat(output, tmp);

	for(int i = 0; i < translation_caches[level].ways; i++)
	{

		snprintf(tmp, 64, "[*]	Level %d - Pi_%d: (%d", level, i, permutation_vectors[i][0]);
		strcat(output, tmp);

		for(int j = 1; j < translation_caches[level].ways; j++)
		{
			snprintf(tmp, 64, ",%d", permutation_vectors[i][j]);
			strcat(output, tmp);
		}

		strcat(output, ")\n");
	}

	kfree(permutation_vectors);
	kfree(tmp);
	return output;
}

char *evaluate_nesting(struct translation_cache *translation_caches, int level)
{
	char *output = kzalloc(128, GFP_KERNEL);

	int misses;

	misses = test_nesting(level, translation_caches);
	if(misses < 0)
	{
		snprintf(output, 128, "[*] Cannot evaluate nesting for level %d translation cache\n", level);
		return output;
	}

	if(misses >= ITERATIONS * THRESHOLD)
		snprintf(output, 128, "[*] Level %d translation cache is nested in level %d translation cache (%d/%d).\n", level, level + 1, misses, ITERATIONS);
	else
		snprintf(output, 128, "[*] Level %d translation cache is NOT nested in level %d translation cache (%d/%d).\n", level, level + 1, ITERATIONS - misses, ITERATIONS);

	return output;
}

char *evaluate_huge_TLB(int level)
{
	char *output = kzalloc(128, GFP_KERNEL);

	int misses = test_huge_tlb(level);

	if(misses >= ITERATIONS * THRESHOLD)
		snprintf(output, 128, "[*] Level %d translation cache and huge TLB are unified (%d/%d)\n", level, misses, ITERATIONS);
	else
		snprintf(output, 128, "[*] Separate level %d translation cache / huge TLB (%d/%d)\n", level, ITERATIONS - misses, ITERATIONS);

	return output;
}

char *evaluate_supported_pcids(int level)
{
	char *output = kzalloc(256, GFP_KERNEL);
	char *tmp = kzalloc(128, GFP_KERNEL);

	int misses;

	for(int amount_pcids = 0; amount_pcids < 4096; amount_pcids++)
	{
		misses = test_supported_pcids(level, amount_pcids, 0);

		if(misses >= ITERATIONS * THRESHOLD)
		{
			snprintf(tmp, 128, "[*] Level %d translation cache supports %d PCIDs (NOFLUSH = 0) (%d/%d)\n", level, amount_pcids, misses, ITERATIONS);
			strcat(output, tmp);
			goto noflush_set;
		}
	}

	snprintf(tmp, 128, "[*] Level %d translation cache supports %d PCIDs (NOFLUSH = 0) (%d/%d)\n", level, 4096, misses, ITERATIONS);
	strcat(output, tmp);

noflush_set:
	for(int amount_pcids = 0; amount_pcids < 4096; amount_pcids++)
	{
		misses = test_supported_pcids(level, amount_pcids, 1);

		if(misses >= ITERATIONS * THRESHOLD)
		{
			snprintf(tmp, 128, "[*] Level %d translation cache supports %d PCIDs (NOFLUSH = 1) (%d/%d)\n", level, amount_pcids, misses, ITERATIONS);
			strcat(output, tmp);
			return output;
		}
	}

	snprintf(tmp, 128, "[*] Level %d translation cache supports %d PCIDs (NOFLUSH = 1) (%d/%d)\n", level, 4096, misses, ITERATIONS);
	strcat(output, tmp);

	return output;
}

char *evaluate_pcid_respect(int level)
{
	char *output = kzalloc(128, GFP_KERNEL);

	int misses = test_pcid_respect(level);

	if(misses >= ITERATIONS * THRESHOLD)
		snprintf(output, 128, "[*] Level %d translation cache respects PCIDs (%d/%d)\n", level, misses, ITERATIONS);
	else
		snprintf(output, 128, "[*] Level %d translation cache does NOT respect PCIDs (%d/%d)\n", level, ITERATIONS - misses, ITERATIONS);

	return output;
}
