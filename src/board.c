#include "board.h"
#include "display.h"
#include "cpu.h"

#include <stdio.h>


uint8_t board_read8(cpu_state* state, uint32_t address)
{
	switch (address){
		case 0 ... PETUCHPC_RAM_SIZE:
			return state->ram[address];
		case PETUCHPC_ROM_BASE ... PETUCHPC_ROM_BASE + PETUCHPC_ROM_SIZE:
			return state->rom[address - PETUCHPC_ROM_BASE];
		case DISPLAY_FRAMEBUFFER_BASE ... DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN:
			return framebuffer[address - DISPLAY_FRAMEBUFFER_BASE];
		default:
			fprintf(stderr, "Invalid read: 0x%08x\n", address);
			return 0;
	}
}

uint16_t board_read16(cpu_state* state, uint32_t address)
{
        return \
		   (uint16_t)board_read8(state, address) \
		| ((uint16_t)board_read8(state, address+1) << 8);
}

uint32_t board_read32(cpu_state* state, uint32_t address)
{
        return \
		   (uint32_t)board_read16(state, address) \
		|  (uint32_t)board_read16(state, address+2) << 16;
}

void board_write8(cpu_state* state, uint32_t address, uint8_t value)
{
	printf("%X ... %X\n", DISPLAY_FRAMEBUFFER_BASE, DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN);
	switch (address){
		case 0 ... PETUCHPC_RAM_SIZE:
			state->ram[address] = value; break;
		case DISPLAY_FRAMEBUFFER_BASE ... DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN:
			framebuffer[address - DISPLAY_FRAMEBUFFER_BASE] = value; break;
		default:
			fprintf(stderr, "Invalid write: 0x%08x\n", address);
	}
}

void board_write16(cpu_state* state, uint32_t address, uint16_t value)
{
	uint8_t high = (value & 0xff00) >> 8;
	uint8_t low = value & 0xff;

	board_write8(state, address, low);
	board_write8(state, address+1, high);
}

void board_write32(cpu_state* state, uint32_t address, uint32_t value)
{
	uint16_t high = (value & 0xffff0000) >> 16;
	uint16_t low = value & 0xffff;

	board_write16(state, address, low);
	board_write16(state, address+2, high);
}