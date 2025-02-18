#include <cpuid_wrapper.h>

#include <cpuid.h>
#include <stdint.h>
#include <stdio.h>

// Function to decode TLB descriptor obtained from CPUID branch 2
void decode_tlb_descriptor(struct tlb *tlbs, uint8_t descriptor)
{
	switch (descriptor) {
		case 0x01:
			tlbs[0].i_tlb.ways = 4;
			tlbs[0].i_tlb.entries = 32;
			printf("[*] 4K iTLB: 4-way set associative, 32 entries\n");
			break;
		case 0x02:
			printf("[*] 4M iTLB: fully associative, 2 entries\n");
			break;
		case 0x03:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 64;
			printf("[*] 4K dTLB: 4-way set associative, 64 entries\n");
			break;
		case 0x04:
			printf("[*] 4M dTLB: 4-way set associative, 8 entries\n");
			break;
		case 0x05:
			printf("[*] 4M dTLB: 4-way set associative, 32 entries\n");
			break;
		case 0x0b:
			printf("[*] 4M iTLB: 4-way set associative, 4 entries\n");
			break;
		case 0x4f:
			tlbs[0].i_tlb.entries = 32;
			printf("[*] 4K iTLB: 32 entries\n");
			break;
		case 0x50:
			tlbs[0].i_tlb.entries = 64;
			tlbs[1].i_tlb.entries = 64;
			printf("[*] 4K/2M/4M iTLB: 64 entries\n");
			break;
		case 0x51:
			tlbs[0].i_tlb.entries = 128;
			tlbs[1].i_tlb.entries = 128;
			printf("[*] 4K/2M/4M iTLB: 128 entries\n");
			break;
		case 0x52:
			tlbs[0].i_tlb.entries = 256;
			tlbs[1].i_tlb.entries = 256;
			printf("[*] 4K/2M/4M iTLB: 256 entries\n");
			break;
		case 0x55:
			tlbs[1].i_tlb.ways = 7;
			tlbs[1].i_tlb.entries = 7;
			printf("[*] 2M/4M iTLB: fully associative, 7 entries\n");
			break;
		case 0x56:
			printf("[*] 4M dTLB: 4-way set associative, 16 entries\n");
			break;
		case 0x57:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 16;
			printf("[*] 4K dTLB: 4-way set associative, 16 entries\n");
			break;
		case 0x59:
			tlbs[0].d_tlb.ways = 16;
			tlbs[0].d_tlb.entries = 16;
			printf("[*] 4K dTLB: fully associative, 16 entries\n");
			break;
		case 0x5a:
			tlbs[1].d_tlb.ways = 4;
			tlbs[1].d_tlb.entries = 32;
			printf("[*] 2M/4M dTLB: 4-way set associative, 32 entries\n");
			break;
		case 0x5b:
			tlbs[0].d_tlb.entries = 64;
			printf("[*] 4K/4M dTLB: 64 entries\n");
			break;
		case 0x5c:
			tlbs[0].d_tlb.entries = 128;
			printf("[*] 4K/4M dTLB: 128 entries\n");
			break;
		case 0x5d:
			tlbs[0].d_tlb.entries = 256;
			printf("[*] 4K/4M dTLB: 256 entries\n");
			break;
		case 0x61:
			tlbs[0].i_tlb.ways = 48;
			tlbs[0].i_tlb.entries = 48;
			printf("[*] 4K iTLB: fully associative, 48 entries\n");
			break;
		case 0x63:
			tlbs[1].d_tlb.ways = 4;
			tlbs[1].d_tlb.entries = 32;
			tlbs[2].d_tlb.ways = 4;
			tlbs[2].d_tlb.entries = 4;
			printf("[*] 2M/4M dTLB: 4-way set associative, 32 entries\n[*] 1G dTLB: 4-way set associative, 4 entries\n");
			break;
		case 0x64:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 512;
			printf("[*] 4K dTLB: 4-way set associative, 512 entries\n");
			break;
		case 0x76:
			tlbs[1].i_tlb.ways = 8;
			tlbs[1].i_tlb.entries = 8;
			printf("[*] 2M/4M iTLB: fully associative, 8 entries\n");
			break;
		case 0xa0:
			tlbs[0].d_tlb.ways = 32;
			tlbs[0].d_tlb.entries = 32;
			printf("[*] 4K dTLB: fully associative, 32 entries\n");
			break;
		case 0xb0:
			tlbs[0].i_tlb.ways = 4;
			tlbs[0].i_tlb.entries = 128;
			printf("[*] 4K iTLB: 4-way set associative, 128 entries\n");
			break;
		case 0xb1:
			tlbs[1].i_tlb.ways = 4;
			tlbs[1].i_tlb.entries = 8;
			printf("[*] 2M iTLB: 4-way set associative, 8 entries\n");
			printf("[*] 4M iTLB: 4-way set associative, 4 entries\n");
			break;
		case 0xb2:
			tlbs[0].i_tlb.ways = 4;
			tlbs[0].i_tlb.entries = 64;
			printf("[*] 4K iTLB: 4-way set associative, 64 entries\n");
			break;
		case 0xb3:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 128;
			printf("[*] 4K dTLB: 4-way set associative, 128 entries\n");
			break;
		case 0xb4:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 256;
			printf("[*] 4K dTLB: 4-way set associative, 256 entries\n");
			break;
		case 0xb5:
			tlbs[0].i_tlb.ways = 8;
			tlbs[0].i_tlb.entries = 64;
			printf("[*] 4K iTLB: 8-way set associative, 64 entries\n");
			break;
		case 0xb6:
			tlbs[0].i_tlb.ways = 8;
			tlbs[0].i_tlb.entries = 128;
			printf("[*] 4K iTLB: 8-way set associative, 128 entries\n");
			break;
		case 0xba:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 64;
			printf("[*] 4K dTLB: 4-way set associative, 64 entries\n");
			break;
		case 0xc0:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 8;
			printf("[*] 4K/4M dTLB: 4-way set associative, 8 entries\n");
			break;
		case 0xc1:
			tlbs[0].s_tlb.ways = 8;
			tlbs[0].s_tlb.entries = 1024;
			tlbs[1].s_tlb.ways = 8;
			tlbs[1].s_tlb.entries = 1024;
			printf("[*] 4K/2M sTLB: 8-way set associative, 1024 entries\n");
			break;
		case 0xc2:
			tlbs[0].d_tlb.ways = 4;
			tlbs[0].d_tlb.entries = 16;
			tlbs[1].d_tlb.ways = 4;
			tlbs[1].d_tlb.entries = 16;
			printf("[*] 4K/2M dTLB: 4-way set associative, 16 entries\n");
			break;
		case 0xc3:
			tlbs[0].s_tlb.ways = 6;
			tlbs[0].s_tlb.entries = 1536;
			tlbs[1].s_tlb.ways = 6;
			tlbs[1].s_tlb.entries = 1536;
			tlbs[2].s_tlb.ways = 4;
			tlbs[2].s_tlb.entries = 16;
			printf("[*] 4K/2M sTLB: 6-way set associative, 1536 entries\n[*] 1G sTLB: 4-way set associative, 16 entries\n");
			break;
		case 0xc4:
			tlbs[1].d_tlb.ways = 4;
			tlbs[1].d_tlb.entries = 32;
			printf("[*] 2M/4M dTLB: 4-way set associative, 32 entries\n");
			break;
		case 0xca:
			tlbs[0].s_tlb.ways = 4;
			tlbs[0].s_tlb.entries = 512;
			printf("[*] 4K sTLB: 4-way set associative, 512 entries\n");
			break;
		default:
			//printf("Unknown descriptor: 0x%02x\n", descriptor);
			break;
    }
}

void cpuid(int code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    __cpuid(code, *a, *b, *c, *d);
}

void get_tlb_info(struct tlb *tlbs)
{
    uint32_t registers[4] = {0, 0, 0, 0};
    uint8_t descriptor;

    cpuid(0x02, &registers[0], &registers[1], &registers[2], &registers[3]);

    for (int reg = 0; reg < 4; reg++) {
    	if(registers[reg] >> 31)
    		continue;
    	for(int byte_offset = 0; byte_offset < 32; byte_offset += 8)
    	{
    		descriptor = (registers[reg] >> byte_offset) & 0xff;
    		if(reg == 0 && byte_offset == 0)
    			continue;
			decode_tlb_descriptor(tlbs, descriptor);
    	}
    }
}