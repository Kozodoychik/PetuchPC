#include "mmu.h"
#include "cpu.h"
#include "board.h"

#include <stdio.h>


uint32_t mmu_virtual_to_physical(cpu_state* state, uint32_t virtual_address) {

	uint16_t pde_index = (virtual_address & MMU_ADDR_PDE_MASK) >> 22;
	uint16_t pte_index = (virtual_address & MMU_ADDR_PTE_MASK) >> 12;
	uint16_t offset = virtual_address & MMU_ADDR_OFFSET_MASK;

	uint32_t pd_entry = board_read(state, state->pd + (pde_index * 4), 4);

	if (!(pd_entry & MMU_PD_PRESENT_MASK)) {
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: MMU: PD: Страница недоступна (Виртуальный адрес: 0x%08X)\n", virtual_address);
		
		cpu_interrupt(state, 0x01);
		
		return 0;
	}

	uint32_t page_table_addr = pd_entry & MMU_PD_ADDR_MASK;

	uint32_t pt_entry = board_read(state, page_table_addr + (pte_index * 4), 4);

	if (!(pt_entry & MMU_PT_PRESENT_MASK)) {
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: MMU: PT: Страница недоступна (Виртуальный адрес: 0x%08X)\n", virtual_address);

		cpu_interrupt(state, 0x01);

		return 0;
	}

	uint32_t base_physical_address = pt_entry & MMU_PT_ADDR_MASK;

	//printf("ОТЛАДКА: MMU: Виртуальный: 0x%08X -> Физический: 0x%08X\r\n", virtual_address, base_physical_address + offset);

	return base_physical_address + offset;

}

void mmu_debug_print_page_directory(cpu_state* state) {
	uint32_t entry_address = state->pd;
	
	printf("ОТЛАДКА: MMU: Вывод содержимого страничной директории по адресу 0x%08X\n", entry_address);

	for (int i = 0; i < 1024; i++) {
		uint32_t entry = board_read(state, entry_address, 4);

		if (entry & MMU_PD_PRESENT_MASK) {
			printf("ОТЛАДКА: MMU: PD: Запись %d: 0x%08X\n", i, entry & MMU_PD_ADDR_MASK);

			uint32_t pt_entry_address = entry & MMU_PD_ADDR_MASK;

			for (int j = 0; j < 1024; j++) {
				uint32_t pt_entry = board_read(state, pt_entry_address, 4);

				if (pt_entry & MMU_PT_PRESENT_MASK) {
					printf("ОТЛАДКА: MMU: PT:	Запись %d: 0x%08X\n", j, pt_entry & MMU_PT_ADDR_MASK);
				}
				pt_entry_address += 4;
			}
		}
		entry_address += 4;
	}
}