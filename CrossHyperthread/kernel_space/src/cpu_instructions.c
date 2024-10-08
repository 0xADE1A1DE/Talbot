#include <cpu_instructions.h>

#include <linux/mm.h>

// The following is from TLB;dr
void claim_cpu(void){
	preempt_disable();
	raw_local_irq_save(flags);
}

void release_cpu(void){
	raw_local_irq_restore(flags);
	preempt_enable();
}