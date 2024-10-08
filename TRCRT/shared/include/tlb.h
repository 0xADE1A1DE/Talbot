#ifndef TLB_H
#define TLB_H

#define IOCTL_SET_TLB_INFO _IOW('t', 1, struct tlb[3])

struct tlb_level
{
	int ways;
	int entries;
};

struct tlb
{
	struct tlb_level i_tlb;
	struct tlb_level d_tlb;
	struct tlb_level s_tlb;
};

extern struct tlb tlbs[3];

#endif