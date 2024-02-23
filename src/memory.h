#include "cpu.h"

#include <stdint.h>

typedef struct {
	uint32_t base;
	uint32_t size;
	uint8_t* data;
} memory_region;

void memory_init(cpu_state*, uint32_t, uint32_t);

uint8_t memory_get8(cpu_state*, uint32_t);
uint16_t memory_get16(cpu_state*, uint32_t);
uint32_t memory_get32(cpu_state*, uint32_t);

void memory_set8(cpu_state*, uint32_t, uint8_t);
void memory_set16(cpu_state*, uint32_t, uint16_t);
void memory_set32(cpu_state*, uint32_t, uint32_t);
