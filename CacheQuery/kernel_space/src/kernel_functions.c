#include <kernel_functions.h>

#include <linux/mm.h>

void disable_smep(void)
{
	uint64_t cr4;
	asm volatile ("mov %%cr4, %0" : "=r" (cr4));
	asm volatile ("mov %0, %%cr4" :: "r" (cr4 & (~(1ULL << 20))));
}