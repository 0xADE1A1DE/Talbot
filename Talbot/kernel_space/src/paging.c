#include <paging.h>

#include <linux/mm.h>

int walk_page_table(unsigned long addr, struct page_table_walk *walk)
{
	struct mm_struct *mm = current->mm;

	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if(!mm)
		mm = current->active_mm;

	pgd = pgd_offset(mm, addr);
	if(pgd_none(*(pgd)))
	{
		walk->page_table_entries[5] = NULL;
		return -5;
	}
	walk->page_table_entries[5] = &pgd->pgd;

	p4d = p4d_offset(pgd, addr);
	if(p4d_none(*(p4d)) || pgd_bad(*(pgd)))
	{
		walk->page_table_entries[4] = NULL;
		return -4;
	}
	walk->page_table_entries[4] = &p4d->p4d;

	pud = pud_offset(p4d, addr);
	if(pud_none(*(pud)) || p4d_bad(*(p4d)))
	{
		walk->page_table_entries[3] = NULL;
		return -3;
	}
	walk->page_table_entries[3] = &pud->pud;

	pmd = pmd_offset(pud, addr);
	if(pmd_none(*(pmd)) || pud_bad(*(pud)))
	{
		walk->page_table_entries[2] = NULL;
		return -2;
	}
	walk->page_table_entries[2] = &pmd->pmd;

	pte = pte_offset_kernel(pmd, addr);
	if(!pte || pmd_bad(*pmd))
	{
		walk->page_table_entries[1] = NULL;
		return -1;
	}
	walk->page_table_entries[1] = &pte->pte;

	return 0;
}

void clear_nx(struct page_table_walk *walk)
{
	*(walk->page_table_entries[5]) &= ~NXBIT;
}