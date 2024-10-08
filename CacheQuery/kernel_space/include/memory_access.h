#ifndef MEMORY_ACCESS_H
#define MEMORY_ACCESS_H

#include <linux/uaccess.h>

typedef u64 (*retf_t)(void);
typedef unsigned long (*memory_access_function)(volatile unsigned long);

static inline __attribute__((always_inline)) void write_data(volatile unsigned long addr, unsigned long data){
	__uaccess_begin_nospec();
	asm volatile("mfence\nlfence\n" ::: "memory");
	*(uint16_t *)((void *)addr) = 0x9090;
	((volatile char *)(volatile void *)addr)[2] = 0x48; ((volatile char *)(volatile void *)addr)[3] = 0xb8;
	*((volatile uint64_t *)&((volatile char *)(volatile void *)addr)[4]) = data;
	((volatile char *)(volatile void *)addr)[12] = 0xc3;
	__uaccess_end();
}

// The following function are from TLBDR
static inline __attribute__((always_inline)) unsigned long read_data(volatile unsigned long addr){
	volatile unsigned long val;
	__uaccess_begin_nospec();
	asm volatile("mfence\nlfence\n" ::: "memory");
	val = *((volatile uint64_t *)&((volatile char *)(volatile void *)addr)[4]);
	__uaccess_end();

	return val;
}

static inline __attribute__((always_inline)) unsigned long fetch_instruction(volatile unsigned long addr){
	volatile unsigned long val;
	__uaccess_begin_nospec();
	asm volatile("mfence\nlfence\n" ::: "memory");
	val = ((volatile retf_t)addr)();
	__uaccess_end();

	return val;
}

#endif
