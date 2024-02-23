#include "memory.h"
#include "cpu.h"

#include <stdio.h>


memory_region regions[8];

void memory_init(cpu_state* state, uint32_t rom_base, uint32_t rom_size)
{
	// RAM
	regions[0].base = 0;
	regions[0].size = PETUCHPC_RAM_SIZE;
	regions[0].data = state->ram;

	// ROM
	regions[1].base = rom_base;
	regions[1].size = rom_size;
	regions[1].data = state->rom;
}

memory_region* get_region(uint32_t address)
{
	for (int i=0;i<2;i++){
		if (address >= regions[i].base && address <= (regions[i].base+regions[i].size)) return &regions[i];
	}

	fprintf(stderr, "Invalid address: 0x%x", address);

	return 0;
}

uint8_t memory_get8(cpu_state* state, uint32_t address)
{
	memory_region* region = get_region(address);

	if (!region) return 0;

	return region->data[address-region->base];
}

uint16_t memory_get16(cpu_state* state, uint32_t address)
{
        return \
		   (uint16_t)memory_get8(state, address) \
		| ((uint16_t)memory_get8(state, address+1) << 8);
}

uint32_t memory_get32(cpu_state* state, uint32_t address)
{
        return \
		   (uint32_t)memory_get16(state, address) \
		|  (uint32_t)memory_get16(state, address+2) << 16;
}
