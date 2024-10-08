#include <hyperthreaded.h>

#include <definitions.h>

#include <memory_access.h>
#include <cpu_instructions.h>
#include <paging.h>
#include <pointer_chasing.h>

#include <linux/random.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <asm/mmu_context.h>
#include <linux/sched/mm.h>

struct completion main_done;
struct completion sub_done;

static DECLARE_COMPLETION(desynchronization);
static DECLARE_COMPLETION(pcid_switching);
static DECLARE_COMPLETION(setup);

int hyperthread_pcid_main(void *data_arg)
{
	unsigned long target_address, prefix_aligned_address, complementary_address;
	struct page_table_walk target_walk, complementary_walk;

	uint64_t cr3 = (get_cr3() >> 12 ) << 12;
	unsigned long p;

	struct thread_data *data = (struct thread_data *)data_arg;

	struct mm_struct *main_mm = data->mm;
	struct mm_struct *old_mm = current->mm;

	mmget(main_mm);
	if(!current->mm)
	{
		current->mm = main_mm;
	}

	wait_for_completion(&setup);

	claim_cpu();
	for(int i = 0; i < ITERATIONS; i++)
	{
		target_address = BASE_ADDRESS + ((uint64_t)(get_random_long() % PAGE_TABLE_ENTRIES[data->level]) << PAGE_TABLE_INDEX_SHIFT[data->level]);
		prefix_aligned_address = target_address + (0x1ffULL << PAGE_TABLE_INDEX_SHIFT[data->level - 1]);
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

		switch_page_table_entries(target_walk.page_table_entries[data->level], complementary_walk.page_table_entries[data->level]);

		fence();

		//set_cr3(cr3 | 0x1 | CR3_NOFLUSH);

		fence();

		complete(&desynchronization);
		wait_for_completion_io(&pcid_switching);

		fence();

		//set_cr3(cr3 | CR3_NOFLUSH);

		fence();

		if(read_data(p) == SIGNAL)
			data->misses++;

		fence();

		switch_page_table_entries(target_walk.page_table_entries[data->level], complementary_walk.page_table_entries[data->level]);
		set_cr3(cr3);
		flush_mmu_caches();

		fence();
	}
	release_cpu();

	if (!old_mm) {
        current->mm = old_mm;
    }
    mmput(main_mm);

	printk(KERN_DEBUG"[*] Main hyperthread finished! Waiting for sub...\n");

	complete_all(&desynchronization);
	wait_for_completion_timeout(&sub_done, msecs_to_jiffies(1000));

	printk(KERN_DEBUG"[*] Finishing\n");

	complete_all(&main_done);
	return 0;
}

int hyperthread_pcid_sub(void *data)
{
	uint64_t cr3 = (get_cr3() >> 12 ) << 12;

	claim_cpu();

	complete_all(&setup);
	for(int i = 0; i < ITERATIONS; i++)
	{
		wait_for_completion(&desynchronization);

		fence();

		for(int pcid = 1; pcid < 4096; pcid++)
		{
			set_cr3(cr3 | pcid | CR3_NOFLUSH);
			fence();
		}
		complete(&pcid_switching);
	}

	release_cpu();

	printk(KERN_DEBUG"[*] Sub hyperthread finished!\n");

	complete_all(&pcid_switching);
	complete_all(&sub_done);
	return 0;
}