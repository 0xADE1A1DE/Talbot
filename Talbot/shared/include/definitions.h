#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define KILO_BYTE (1 << 10)
#define MEGA_BYTE (1 << 20)
#define GIGA_BYTE (1 << 30)

#define BASE_ADDRESS 				(0x100000000000ULL)
#define HUGE_2MB_BASE_ADDRESS 		(0x200000000000ULL)
#define HUGE_1GB_BASE_ADDRESS 		(0x300000000000ULL)
#define COMPLEMENTARY_BASE_ADDRESS 	(0x700000000000ULL)

#define AMOUNT_2M_PAGES	(2048)
#define AMOUNT_1G_PAGES (64)

#define THRESHOLD (.8)

#define SIGNAL (0x1337)

#define MEMORY_PROTECTIONS (PROT_READ | PROT_WRITE | PROT_EXEC)

#define ITERATIONS (1000)

#define PINNED_CORE (0)

#define MAXIMUM_ENTRIES (512) // this defines "many" when touching "many" addresses! (e.g. Split layer experiment)

#define CORE1 (0)
#define CORE2 (0)

#define PRINT_MINIMAL_EVICTION_SET (0)

static const int PAGE_TABLE_INDEX_SHIFT[] = {0, 12, 21, 30, 39, 48};
static const int PAGE_TABLE_ENTRIES[] = {-1, -1, 512*32, 512, 32};

#endif
