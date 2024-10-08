#ifndef HYPERTHREADED_H
#define HYPERTHREADED_H

#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/mm.h>

extern struct completion main_done;
extern struct completion sub_done;

struct thread_data {
    int misses;
    int level;
    struct mm_struct *mm;
};

int hyperthread_pcid_main(void *data);
int hyperthread_pcid_sub(void *data);

#endif