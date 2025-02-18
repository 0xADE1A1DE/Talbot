#define _GNU_SOURCE
#include <definitions.h>

#include <tlb.h>
#include <cpuid_wrapper.h>
#include <cores.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include <x86intrin.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <sched.h>

#ifndef MFD_HUGE_2MB
#define MFD_HUGE_2MB (MAP_HUGE_2MB)
#endif

#ifndef MFD_HUGE_1GB
#define MFD_HUGE_1GB (MAP_HUGE_1GB)
#endif

#define MMAP_FLAGS (MAP_SHARED | MAP_FIXED | MAP_POPULATE)


void print_tlbs(struct tlb *tlbs)
{
	printf("[*] 4K Pages:\n");
	printf("	[*] iTLB: %d ways, %d entries\n", tlbs[0].i_tlb.ways, tlbs[0].i_tlb.entries);
	printf("	[*] dTLB: %d ways, %d entries\n", tlbs[0].d_tlb.ways, tlbs[0].d_tlb.entries);
	printf("	[*] sTLB: %d ways, %d entries\n", tlbs[0].s_tlb.ways, tlbs[0].s_tlb.entries);

	printf("[*] 2M Pages:\n");
	printf("	[*] iTLB: %d ways, %d entries\n", tlbs[1].i_tlb.ways, tlbs[1].i_tlb.entries);
	printf("	[*] dTLB: %d ways, %d entries\n", tlbs[1].d_tlb.ways, tlbs[1].d_tlb.entries);
	printf("	[*] sTLB: %d ways, %d entries\n", tlbs[1].s_tlb.ways, tlbs[1].s_tlb.entries);

	printf("[*] 1G Pages:\n");
	printf("	[*] iTLB: %d ways, %d entries\n", tlbs[2].i_tlb.ways, tlbs[2].i_tlb.entries);
	printf("	[*] dTLB: %d ways, %d entries\n", tlbs[2].d_tlb.ways, tlbs[2].d_tlb.entries);
	printf("	[*] sTLB: %d ways, %d entries\n", tlbs[2].s_tlb.ways, tlbs[2].s_tlb.entries);
}


void map_page(void *address, size_t page_size, int fd, int flags, int mapped_address_count) 
{
	if(mmap(address, page_size, MEMORY_PROTECTIONS, flags, fd, page_size * mapped_address_count) == MAP_FAILED)
	{
		printf("[-] Memory allocation failed! Virtual address: %p\n", address);
		perror("mmap");
		close(fd);
		exit(-1);
	}
}

void initialize_page(volatile unsigned char *address, uint64_t value)
{
	// Initialize memory in similar fashion as in TLBDR
	*(uint16_t *)address = 0x9090;
	address[2] = 0x48; address[3] = 0xb8;
	*(uint64_t *)(&address[4]) = value;
	address[12] = 0xc3;
}

// maps and initializes the required lower level entries for a given entry on a given level, skipping the first start_index lower level entries
void setup_level_entry(uint64_t base_address, int fd, int level, uint64_t entry_index, int *mapped_page_count, int start_index, uint64_t initialization_value)
{
	volatile unsigned char *memory_pointer = NULL;

	for(uint64_t i = start_index; i < 3; i++)
	{
		memory_pointer = (unsigned char *) (base_address + (entry_index << PAGE_TABLE_INDEX_SHIFT[level]) + (i << PAGE_TABLE_INDEX_SHIFT[level - 1]));

		map_page((void *)memory_pointer, 4 * KILO_BYTE, fd, MMAP_FLAGS, *mapped_page_count);
		initialize_page((void *)memory_pointer, initialization_value);
		(*mapped_page_count)++;
	}

	if(start_index > 0x1ff)
		return;

	memory_pointer = (unsigned char *) (base_address + (entry_index << PAGE_TABLE_INDEX_SHIFT[level]) + ((uint64_t)0x1ff << PAGE_TABLE_INDEX_SHIFT[level - 1]));

	map_page((void *)memory_pointer, 4 * KILO_BYTE, fd, MMAP_FLAGS, *mapped_page_count);
	initialize_page((void *)memory_pointer, initialization_value);
	(*mapped_page_count)++;
}

// maps the required entries for each level and initializes the mapped pages. Also considers overlapping (e.g. 1024 level two entries also map 2 level three entries)
void setup_levels(uint64_t base_address, int fd, uint64_t initialization_value)
{
	int mapped_address_count = 0;

	for(uint64_t page_table_2_index = 0; page_table_2_index < PAGE_TABLE_ENTRIES[2]; page_table_2_index++)
	{
		setup_level_entry(base_address, fd, 2, page_table_2_index, &mapped_address_count, 0, initialization_value);
	}

	// Map remaining level 3 entries. For each 512 level 2 entries subtract one level 3 entry.
	for(uint64_t index_page_table_3 = (PAGE_TABLE_ENTRIES[2]/512); index_page_table_3 < PAGE_TABLE_ENTRIES[3]; index_page_table_3++)
	{
		if(index_page_table_3 == (PAGE_TABLE_ENTRIES[2]/512))
		{
			setup_level_entry(base_address, fd, 3, index_page_table_3, &mapped_address_count, PAGE_TABLE_ENTRIES[2] % 512, initialization_value);
			continue;
		}

		setup_level_entry(base_address, fd, 3, index_page_table_3, &mapped_address_count, 0, initialization_value);
	}

	// Map remaining level 4 entries. For each 512 level 3 entries subtract one level 4 entry.
	for(uint64_t index_page_table_4 = ((PAGE_TABLE_ENTRIES[2]/512) > PAGE_TABLE_ENTRIES[3]) ? (PAGE_TABLE_ENTRIES[2]/512)/512 : PAGE_TABLE_ENTRIES[3]/512; index_page_table_4 < PAGE_TABLE_ENTRIES[4]; index_page_table_4++)
	{
		if(index_page_table_4 == ((PAGE_TABLE_ENTRIES[2]/512) > PAGE_TABLE_ENTRIES[3]) ? (PAGE_TABLE_ENTRIES[2]/512)/512 : PAGE_TABLE_ENTRIES[3]/512)
		{
			setup_level_entry(base_address, fd, 4, index_page_table_4, &mapped_address_count, (((PAGE_TABLE_ENTRIES[2]/512) > PAGE_TABLE_ENTRIES[3]) ? (PAGE_TABLE_ENTRIES[2]/512) % 512 : PAGE_TABLE_ENTRIES[3] % 512), initialization_value);
			continue;
		}

		setup_level_entry(base_address, fd, 4, index_page_table_4, &mapped_address_count, 0, initialization_value);
	}
}

size_t allocate_huge_pages(size_t page_size, size_t number_huge_pages)
{
	FILE *f;
	size_t validation_number;

	if(page_size == 2 * MEGA_BYTE)
		f = fopen("/sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages", "r+");
	else if(page_size == 1 * GIGA_BYTE)
		f = fopen("/sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages", "r+");
	else
	{
		printf("[-] Page size not supported!\n");
		return -1;
	}

	fscanf(f, "%zu", &validation_number);

	_mm_mfence();
	_mm_lfence();

	fprintf(f, "%zu", number_huge_pages);
	fflush(f);
	fscanf(f, "%zu", &validation_number);

	if(number_huge_pages != validation_number)
		printf("[-] Cannot allocate %zu 2M/1G huge pages - instead allocated %zu\n", number_huge_pages, validation_number);

	fclose(f);

	return validation_number;
}

void setup_huge_pages(unsigned long base_address, size_t page_size, int fd, uint amount)
{
	volatile unsigned char *memory_pointer = NULL;

	for(uint64_t index = 0; index < amount; index++)
	{
		memory_pointer = (unsigned char *)base_address + (index * page_size);

		map_page((void *)memory_pointer, page_size, fd, MMAP_FLAGS | MAP_HUGETLB, 0);
		initialize_page((void *)memory_pointer, 0);
	}
}



int main()
{
	int fd_base_memory, fd_complementary_memory, mfd_huge_2MB_memory, mfd_huge_1GB_memory;
	int amount_4KB_pages;
	int entries_2M_tlb, entries_1G_tlb;

	char *output = malloc(1024*1024);

	set_number_of_cores(); 
	enable_all_cores();

	shm_unlink("/base_memory");
	shm_unlink("/complementary_memory");

	get_tlb_info(tlbs);
	print_tlbs(tlbs);

	entries_2M_tlb = tlbs[1].s_tlb.entries + tlbs[1].d_tlb.entries;
	entries_1G_tlb = tlbs[2].s_tlb.entries + tlbs[2].d_tlb.entries;

	// This will be more than enough 4KB pages, but the true formular is too complex to caluclate here, so we take this upper bound
	amount_4KB_pages = (PAGE_TABLE_ENTRIES[2] + PAGE_TABLE_ENTRIES[3] + PAGE_TABLE_ENTRIES[4]) * 4;	

	fd_base_memory = shm_open("/base_memory", O_RDWR | O_CREAT, 0777);
	fd_complementary_memory = shm_open("/complementary_memory", O_RDWR | O_CREAT, 0777);

	ftruncate(fd_base_memory, 4 * KILO_BYTE * amount_4KB_pages);
	ftruncate(fd_complementary_memory, 4 * KILO_BYTE * amount_4KB_pages);

	setup_levels(BASE_ADDRESS, fd_base_memory, 0);
	setup_levels(COMPLEMENTARY_BASE_ADDRESS, fd_complementary_memory, SIGNAL);

	_mm_lfence();
	_mm_mfence();

	allocate_huge_pages(2 * MEGA_BYTE, 1);
	allocate_huge_pages(1 * GIGA_BYTE, 1);

	mfd_huge_2MB_memory = memfd_create("huge_2mb_memory", MFD_HUGETLB | MFD_HUGE_2MB);
	mfd_huge_1GB_memory = memfd_create("huge_1gb_memory", MFD_HUGETLB | MFD_HUGE_1GB);

	ftruncate(mfd_huge_2MB_memory, 2 * MEGA_BYTE);
	ftruncate(mfd_huge_1GB_memory, 1 * GIGA_BYTE);

	if(entries_2M_tlb > AMOUNT_2M_PAGES || entries_1G_tlb > AMOUNT_1G_PAGES)
		printf("[!] Warning: You do not map enough huge pages to evict the huge TLBs!\n");

	setup_huge_pages(HUGE_2MB_BASE_ADDRESS, 2 * MEGA_BYTE, mfd_huge_2MB_memory, AMOUNT_2M_PAGES);
	setup_huge_pages(HUGE_1GB_BASE_ADDRESS, 1 * GIGA_BYTE, mfd_huge_1GB_memory, AMOUNT_1G_PAGES);

	_mm_lfence();
	_mm_mfence();

	volatile int dev = open("/dev/talbot", O_RDWR);
	if(dev < 0)
	{
		printf("[-] Failed to open device!\n");
		goto error;
	}

	ioctl(dev, IOCTL_SET_TLB_INFO, &tlbs);



	//Pin process to a core
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(PINNED_CORE, &mask);
	if(sched_setaffinity(0, sizeof(mask), &mask) == -1){
		printf("[-] Unable to pin at core %d\n", PINNED_CORE);
		return 1;
	}

	int coresident = get_co_resident(PINNED_CORE);

	//Disable co-resident core
	/*if(disable_hyper){
		if(coresident != -1){
			disable_core(coresident);
			printf("Pinned on core %d, disabled core %d.\n\n", PINNED_CORE, coresident);
		}else{
			printf("Pinned on core %d, no co-resident found\n\n", PINNED_CORE);
		}
	}*/

	if(read(dev, output, 1024*1024) < 0)
	{
		printf("[-] Failed to read from device!\n");
		goto error;
	}

	printf("\n[********************** RESULTS **********************]\n\n%s\n", output);

cleanup:
	enable_all_cores();
	close(fd_base_memory);
	close(fd_complementary_memory);
	close(mfd_huge_2MB_memory);
	close(mfd_huge_1GB_memory);
	close(dev);
	shm_unlink("/base_memory");
	shm_unlink("/complementary_memory");
	free(output);
	return 0;

error:
	enable_all_cores();
	close(fd_base_memory);
	close(fd_complementary_memory);
	close(mfd_huge_2MB_memory);
	close(mfd_huge_1GB_memory);
	close(dev);
	shm_unlink("/base_memory");
	shm_unlink("/complementary_memory");
	free(output);
	return -1;
}
