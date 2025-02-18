#ifndef CPU_INSTRUCTIONS_H
#define CPU_INSTRUCTIONS_H

#include <linux/kernel.h>
#include <asm/tlbflush.h>

static inline void fence(void)
{
	asm volatile("lfence;\nmfence;" : : : "memory");
}

static inline void set_cr3(uint64_t cr3)
{
	asm volatile("mov %0, %%cr3" : : "r" (cr3));
}

static inline uint64_t get_cr3(void)
{
	uint64_t cr3;
	asm volatile("mov %%cr3, %0" : "=r" (cr3));
	return cr3;
}

static inline void invlpg(unsigned long addr)
{
	asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

static inline void flush_mmu_caches(void)
{
	uint64_t cr3 = get_cr3();
	set_cr3(cr3);
}


// The following is from TLB;dr
static volatile unsigned long flags;

void claim_cpu(void);
void release_cpu(void);

#endif
