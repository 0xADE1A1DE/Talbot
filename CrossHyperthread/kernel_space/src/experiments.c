#include <experiments.h>

#include <definitions.h>

#include <hyperthreaded.h>

#include <linux/slab.h>

int test_hyperthreaded_pcid_support(int level)
{
	struct task_struct *task1, *task2;
	int misses;

	struct thread_data *data = kmalloc(sizeof(struct thread_data), GFP_KERNEL);

	data->misses = 0;
	data->level = level;
	data->mm = current->mm;

	task1 = kthread_create(&hyperthread_pcid_main, (void *)data, "PCID");

	if(IS_ERR(task1))
	{
		printk(KERN_DEBUG"[-] Failed to create main thread: %ld\n", PTR_ERR(task1));
		return -1;
	}

	kthread_bind(task1, CORE1);

	task2 = kthread_create(&hyperthread_pcid_sub, (void *)NULL, "PCID");

	if(IS_ERR(task2))
	{
		printk(KERN_DEBUG"[-] Failed to create sub thread: %ld\n", PTR_ERR(task2));
		return -1;
	}

	kthread_bind(task2, CORE2);

	init_completion(&main_done);
	init_completion(&sub_done);

	wake_up_process(task1);
	wake_up_process(task2);

	wait_for_completion_timeout(&main_done, msecs_to_jiffies(10000));

	misses = data->misses;

	kfree(data);

	return misses;
}