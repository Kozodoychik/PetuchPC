#include "board.h"
#include "display.h"
#include "cpu.h"

#include <stdio.h>


void debug_port_write_handler(cpu_state* state, uint8_t value) {
	putc(value, stdout);
}

void board_init(cpu_state* state) {
	for (uint8_t port = 0;port < MMIO_PORT_COUNT;port++) {
		mmio_ports[port].read = mmio_dummy_port_read_handler;
		mmio_ports[port].write = mmio_dummy_port_write_handler;
	}

	mmio_ports[0x0].read = mmio_dummy_port_read_handler;
	mmio_ports[0x0].write = debug_port_write_handler;
}

uint32_t board_read(cpu_state* state, uint32_t physical_address, int length) {
	if (length > 4) {
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Чтение больше 4 байт не реализовано (размер: %d байт)\n", length);
		length = 4;
	}
	uint32_t value = 0;

	// Адовая попытка оптимизации

	if (physical_address < PETUCHPC_RAM_SIZE)
		value = *(uint32_t*)&state->ram[physical_address];
	else if (physical_address >= PETUCHPC_ROM_BASE && physical_address < PETUCHPC_ROM_BASE + PETUCHPC_ROM_SIZE)
		value = *(uint32_t*)&state->rom[physical_address & 0x0fffffff];
	else if (physical_address >= DISPLAY_FRAMEBUFFER_BASE && physical_address < DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN)
		value = *(uint32_t*)&framebuffer[physical_address - DISPLAY_FRAMEBUFFER_BASE];
	else if (physical_address >= MMIO_BASE && physical_address < MMIO_END) {
		uint8_t port = (uint8_t)(physical_address & 0xff);
		value |= mmio_ports[port].read(state) << 24;
	}

	return value;
}

void board_write(cpu_state* state, uint32_t physical_address, int length, uint32_t value) {
	if (length > 4) {
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Запись больше 4 байт не реализована (размер: %d байт)\n", length);
		length = 4;
	}

	for (int i = 0;i < length;i++) {
		//printf("len=%d\n", length);
		if (physical_address + i < PETUCHPC_RAM_SIZE)
			state->ram[physical_address + i] = value & 0xff;
		else if (physical_address + i >= DISPLAY_FRAMEBUFFER_BASE && physical_address + i < DISPLAY_FRAMEBUFFER_BASE + DISPLAY_FRAMEBUFFER_LEN)
			framebuffer[(physical_address + i) - DISPLAY_FRAMEBUFFER_BASE] = value & 0xff;
		else if (physical_address >= MMIO_BASE && physical_address < MMIO_END) {
			uint8_t port = (uint8_t)(physical_address & 0xff);
			mmio_ports[port].write(state, value & 0xff);
		}
		else {
			fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Недопустимая запись: 0x%08x\n", physical_address + i);
		}
		value >>= 8;
	}
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