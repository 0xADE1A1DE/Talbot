#ifndef PAGING_H
#define PAGING_H

#include <linux/pgtable.h>

#define NXBIT (1ULL << 63)

struct page_table_walk
{
	unsigned long *page_table_entries[6];
};

int walk_page_table(unsigned long addr, struct page_table_walk *walk);

void clear_nx(struct page_table_walk *walk);

static inline __attribute__((always_inline)) void switch_page_table_entries(unsigned long *page_table_entry_1, unsigned long *page_table_entry_2)
{
	unsigned long page_table_entry = *page_table_entry_1;
	*page_table_entry_1 = *page_table_entry_2;
	*page_table_entry_2 = page_table_entry;
}

#endif

