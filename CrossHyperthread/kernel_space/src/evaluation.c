#include <evaluation.h>

#include <definitions.h>

#include <experiments.h>

#include <linux/kernel.h>
#include <linux/slab.h>

char *evaluate_hyperthreaded_pcid_support(int level)
{
	char *output = kzalloc(128, GFP_KERNEL);
	int misses = test_hyperthreaded_pcid_support(level);

	if(misses >= ITERATIONS * THRESHOLD)
		snprintf(output, 128, "[*] Level %d translation cache is affected by PCID switches in co-resident hyperthread (%d/%d).\n", level, misses, ITERATIONS);
	else
		snprintf(output, 128, "[*] Level %d translation cache entries are tagged with their hyperthread ID (%d/%d).\n", level, ITERATIONS - misses, ITERATIONS);

	return output;
}
