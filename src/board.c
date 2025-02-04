#include "board.h"
#include "display.h"
#include "cpu.h"

#include <stdio.h>


void debug_port_write_handler(cpu_state* state, uint8_t value) {
	putc(value, stdout);
}

void board_init(cpu_state* state) {
	for (uint8_t port = 0;port < MMIO_PORT_COUNT;port++) {
		mmio_ports[port].port_read_handler = mmio_dummy_port_read_handler;
		mmio_ports[port].port_write_handler = mmio_dummy_port_write_handler;
	}

	mmio_ports[0x0].port_read_handler = mmio_dummy_port_read_handler;
	mmio_ports[0x0].port_write_handler = debug_port_write_handler;
}

uint8_t board_read8(cpu_state* state, uint32_t physical_address)
{
	// MSVC не поддерживет эллипсис, можете раскомментировать это если используете GCC
	/* switch (physical_address) {
		case 0 ... PETUCHPC_RAM_SIZE:
			return state->ram[physical_address];
		case PETUCHPC_ROM_BASE ... PETUCHPC_ROM_BASE + PETUCHPC_ROM_SIZE:
			return state->rom[physical_address - PETUCHPC_ROM_BASE];
		case DISPLAY_FRAMEBUFFER_BASE ... DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN:
			return framebuffer[physical_address - DISPLAY_FRAMEBUFFER_BASE];
		case MMIO_BASE ... MMIO_END:
			return 0;
		default:
			fprintf(stderr, "Недопустимое чтение: 0x%08x\n", physical_address);
			return 0;
	} */

	if (physical_address >= 0 && physical_address < PETUCHPC_RAM_SIZE)
		return state->ram[physical_address];
	else if (physical_address >= PETUCHPC_ROM_BASE && physical_address < PETUCHPC_ROM_BASE + PETUCHPC_ROM_SIZE)
		return state->rom[physical_address - PETUCHPC_ROM_BASE];
	else if (physical_address >= DISPLAY_FRAMEBUFFER_BASE && physical_address < DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN)
		return framebuffer[physical_address - DISPLAY_FRAMEBUFFER_BASE];
	else if (physical_address >= MMIO_BASE && physical_address < MMIO_END) {
		uint8_t port = (uint8_t)(physical_address & 0xff);
		return mmio_ports[port].port_read_handler(state);
	}
	else {
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Недопустимое чтение: 0x%08x\n", physical_address);
		getc(stdin);
		return 0;
	}

}

uint16_t board_read16(cpu_state* state, uint32_t physical_address)
{
        return \
		   (uint16_t)board_read8(state, physical_address) \
		| ((uint16_t)board_read8(state, physical_address+1) << 8);
}

uint32_t board_read32(cpu_state* state, uint32_t physical_address)
{
        return \
		   (uint32_t)board_read16(state, physical_address) \
		|  (uint32_t)board_read16(state, physical_address+2) << 16;
}

void board_write8(cpu_state* state, uint32_t physical_address, uint8_t value)
{
	// MSVC не поддерживет эллипсис, можете раскомментировать это если используете GCC
	/* switch (physical_address) {
		case 0 ... PETUCHPC_RAM_SIZE:
			state->ram[physical_address] = value; break;
		case DISPLAY_FRAMEBUFFER_BASE ... DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN:
			framebuffer[physical_address - DISPLAY_FRAMEBUFFER_BASE] = value; break;
		case MMIO_BASE ... MMIO_END:{
			uint8_t port = (uint8_t)(physical_address & 0xff);
			switch (port){
				case 0x0: putc(value, stdout); break;
			}
			break;
		}
		default:
			fprintf(stderr, "Недопустимая запись: 0x%08x\n", physical_address);
	} */

	//printf("%X\n", physical_address);

	if (physical_address >= 0 && physical_address < PETUCHPC_RAM_SIZE)
		state->ram[physical_address] = value;
	else if (physical_address >= DISPLAY_FRAMEBUFFER_BASE && physical_address < DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN)
		framebuffer[physical_address - DISPLAY_FRAMEBUFFER_BASE] = value;
	else if (physical_address >= MMIO_BASE && physical_address < MMIO_END) {
		uint8_t port = (uint8_t)(physical_address & 0xff);
		mmio_ports[port].port_write_handler(state, value);
	}
	else 
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Недопустимая запись: 0x%08x\n", physical_address);
}

void board_write16(cpu_state* state, uint32_t physical_address, uint16_t value)
{
	uint8_t high = (value & 0xff00) >> 8;
	uint8_t low = value & 0xff;

	board_write8(state, physical_address, low);
	board_write8(state, physical_address+1, high);
}

void board_write32(cpu_state* state, uint32_t physical_address, uint32_t value)
{
	uint16_t high = (value & 0xffff0000) >> 16;
	uint16_t low = value & 0xffff;

	board_write16(state, physical_address, low);
	board_write16(state, physical_address+2, high);
}

// Обработчики-пустышки. Нужны, чтобы не оставлять пустыми указатели на функции чтения/записи с порта

uint8_t mmio_dummy_port_read_handler(cpu_state* state) {
	fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Недопустимое чтение с порта\n");
	return 0;
}

void mmio_dummy_port_write_handler(cpu_state* state, uint8_t value) {
	fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Недопустимая запись в порт\n");
	return;
}