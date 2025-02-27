#pragma once

#include "cpu.h"

#include <stdint.h>

#define MMIO_BASE 0x80000000
#define MMIO_PORT_COUNT 0xff
#define MMIO_END MMIO_BASE + MMIO_PORT_COUNT

typedef struct {

	uint8_t (*read)(cpu_state*);
	void (*write)(cpu_state*, uint8_t);

} mmio_port;

mmio_port mmio_ports[MMIO_PORT_COUNT];

void board_init(cpu_state*);

uint32_t board_read(cpu_state*, uint32_t, int);
void board_write(cpu_state*, uint32_t, int, uint32_t);

uint8_t mmio_dummy_port_read_handler(cpu_state*);
void mmio_dummy_port_write_handler(cpu_state*, uint8_t);
