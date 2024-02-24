#include "cpu.h"

#include <stdint.h>


void memory_init(cpu_state*, uint32_t, uint32_t);

uint8_t board_read8(cpu_state*, uint32_t);
uint16_t board_read16(cpu_state*, uint32_t);
uint32_t board_read32(cpu_state*, uint32_t);

void board_write8(cpu_state*, uint32_t, uint8_t);
void board_write16(cpu_state*, uint32_t, uint16_t);
void board_write32(cpu_state*, uint32_t, uint32_t);
