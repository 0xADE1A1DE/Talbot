#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define KILO_BYTE (1 << 10)
#define MEGA_BYTE (1 << 20)
#define GIGA_BYTE (1 << 30)

#define BASE_ADDRESS 				(0x100000000000ULL)
#define COMPLEMENTARY_BASE_ADDRESS 	(0x700000000000ULL)

#define THRESHOLD (.8)

#define SIGNAL (0x1337)

#define MEMORY_PROTECTIONS (PROT_READ | PROT_WRITE | PROT_EXEC)

#define ITERATIONS (1000)

#define PINNED_CORE (0)

#define CORE1 (0)
#define CORE2 (4)

static const int PAGE_TABLE_INDEX_SHIFT[] = {0, 12, 21, 30, 39, 48};
static const int PAGE_TABLE_ENTRIES[] = {-1, -1, 512*32, 512, 32};

#endif
