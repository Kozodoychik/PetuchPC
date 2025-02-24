#pragma once

#include "cpu.h"

#include <stdint.h>

#define MMU_PD_PRESENT_MASK	1
#define MMU_PT_PRESENT_MASK	1

#define MMU_PD_ADDR_MASK 0xfffff000
#define MMU_PT_ADDR_MASK 0xfffff000

#define MMU_ADDR_PDE_MASK 0xffc00000
#define MMU_ADDR_PTE_MASK 0x003ff000
#define MMU_ADDR_OFFSET_MASK 0xfff

uint32_t mmu_virtual_to_physical(cpu_state*, uint32_t);

void mmu_debug_print_page_directory(cpu_state*);