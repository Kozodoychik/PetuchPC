#include "cpu.h"
#include "board.h"
#include "mmu.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

void cpu_reset(cpu_state* state) {
	// Инициализируем память, регистры и устанавливаем значения по умолчанию для ip, sp и it

	memset(state->ram, 0, PETUCHPC_RAM_SIZE);
	memset(state->rom, 0, PETUCHPC_ROM_SIZE);

	state->ip = PETUCHPC_ROM_BASE;
	state->sp = PETUCHPC_STACK_BASE;
	state->it = PETUCHPC_INTERRUPT_TABLE_BASE;
	state->msr = 0;
	state->pd = 0;

	memset(state->r, 0, PETUCHPC_REGISTER_COUNT * sizeof(uint32_t)); // Инициализация регистров

	// Флаги

	state->flags.zero = false;
	state->flags.carry = false;
	state->flags.interrupt = true;
	state->flags.negative = false;

}

uint8_t flags_to_byte(cpu_state* state) {
	return (uint8_t)\
		state->flags.carry | \
		state->flags.interrupt << 1 | \
		state->flags.negative << 2 | \
		state->flags.zero << 3;
}

void byte_to_flags(cpu_state* state, uint8_t flags) {
	state->flags.carry = flags & 0b1;
	state->flags.interrupt = (flags & 0b10) >> 1;
	state->flags.negative = (flags & 0b100) >> 2;
	state->flags.zero = (flags & 0b1000) >> 3;
}

uint8_t cpu_read8(cpu_state* state, uint32_t address) {
	if (state->msr & PETUCHPC_MSR_MMU_MASK) {
		uint32_t physical_address = mmu_virtual_to_physical(state, address);
		return (uint8_t)board_read(state, physical_address, 1);
	}
	return (uint8_t)board_read(state, address, 1);
}

uint16_t cpu_read16(cpu_state* state, uint32_t address) {
	if (state->msr & PETUCHPC_MSR_MMU_MASK) {
		uint32_t physical_address = mmu_virtual_to_physical(state, address);
		return (uint16_t)board_read(state, physical_address, 2);
	}
	return (uint16_t)board_read(state, address, 2);
}

uint32_t cpu_read32(cpu_state* state, uint32_t address) {
	if (state->msr & PETUCHPC_MSR_MMU_MASK) {
		uint32_t physical_address = mmu_virtual_to_physical(state, address);
		return board_read(state, physical_address, 4);
	}
	return board_read(state, address, 4);
}

void cpu_write8(cpu_state* state, uint32_t address, uint8_t value) {
	if (state->msr & PETUCHPC_MSR_MMU_MASK) {
		uint32_t physical_address = mmu_virtual_to_physical(state, address);
		board_write(state, physical_address, 1, (uint32_t)value);
		return;
	}
	board_write(state, address, 1, (uint32_t)value);
}

void cpu_write16(cpu_state* state, uint32_t address, uint16_t value) {
	if (state->msr & PETUCHPC_MSR_MMU_MASK) {
		uint32_t physical_address = mmu_virtual_to_physical(state, address);
		board_write(state, physical_address, 2, (uint32_t)value);
		return;
	}
	board_write(state, address, 2, (uint32_t)value);
}

void cpu_write32(cpu_state* state, uint32_t address, uint32_t value) {
	if (state->msr & PETUCHPC_MSR_MMU_MASK) {
		uint32_t physical_address = mmu_virtual_to_physical(state, address);
		board_write(state, physical_address, 4, value);
		return;
	}
	board_write(state, address, 4, value);
}

void push_registers(cpu_state* state) {
	for (uint8_t i = 0;i < PETUCHPC_REGISTER_COUNT;i++) {
		PUSH(state->r[i]);
	}
	PUSH(state->it);
	PUSH(flags_to_byte(state));
}

void pop_registers(cpu_state* state) {
	uint8_t flags;

	POP(flags);
	
	byte_to_flags(state, flags);

	POP(state->it);

	for (uint8_t i = 1;i <= PETUCHPC_REGISTER_COUNT;i++) {
		POP(state->r[PETUCHPC_REGISTER_COUNT - i]);
	}
}

/*	ИСКЛЮЧЕНИЯ
 
	0x00 - Деление на нуль	(пока не используется лол)
	0x01 - Чтение с недоступной страницы
*/

void cpu_interrupt(cpu_state* state, int interrupt) {
	if (!state->flags.interrupt) return;

	uint32_t* interrupt_table = (uint32_t*)state->ram+state->it;

	PUSH(state->ip);

	push_registers(state);

	PUSH((uint8_t)interrupt);

	if (interrupt_table[interrupt] == 0)
		fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Необрабатываемое прерывание 0x%X\n", interrupt);

	state->ip = interrupt_table[interrupt];
}

void print_registers(cpu_state* state) {
	printf("zzz\n");
	for (int i=0;i<PETUCHPC_REGISTER_COUNT;i++){
		printf("r%.2d=0x%08X\n", i, state->r[i]);
	}
}

bool skip_instr(cpu_state* state, uint8_t cond) {
	switch (cond){
		case COND_ALWAYS: return false;
		case COND_EQ: return !state->flags.zero;
		case COND_NEQ: return state->flags.zero;
		case COND_GR: return state->flags.negative;
		case COND_L: return !state->flags.negative;
	}

	fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Некорректное условие: 0x%x\n", cond);
	return true;
}

void cpu_execute(cpu_state* state) {
	if (state->halted) return;

	uint16_t op = cpu_read16(state, state->ip);
	
	//print_registers(state);
	//printf("ip: 0x%08x\n", state->ip);
	//printf("%x\n", op);
	
	switch(GET_OPCODE(op)){
		case NOP:{
			CHECK_TYPE5_RESERVED(op);

			state->ip += 2;
			break;
		}
		case ADD0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] + state->r[src];
			
			state->ip += 2;
			break;
		}
		case ADD3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] + cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] + cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] + cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}
			break;
		}
		case SUB0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] - state->r[src];
			
			state->ip += 2;
			break;
		}
		case SUB3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] - cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] - cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] - cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case MUL0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] * state->r[src];
			
			state->ip += 2;
			break;
		}
		case MUL3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] * cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] * cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] * cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case DIV0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] / state->r[src];
			
			state->ip += 2;
			break;
		}
		case DIV3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] / cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] / cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] / cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case CPY:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[src];

			state->ip += 2;
			break;
		}
		case SWP:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);
			uint32_t temp = state->r[dest];

			state->r[dest] = state->r[src];
			state->r[src] = temp;

			state->ip += 2;
			break;
		}
		case AND0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] & state->r[src];

			state->ip += 2;
			break;
		}
		case AND3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] & cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] & cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] & cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case OR0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] | state->r[src];

			state->ip += 2;
			break;
		}
		case OR3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] | cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] | cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] | cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case NOT:{
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest] = ~state->r[dest];

			state->ip += 2;
			break;
		}
		case XOR0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			state->r[dest] = state->r[dest] ^ state->r[src];

			state->ip += 2;
			break;
		}
		case XOR3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			switch (size){
				case BYTE:{
					state->r[dest] = state->r[dest] ^ cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] ^ cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] ^ cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case INC:{
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest]++;

			state->ip += 2;
			break;
		}
		case DEC:{
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest]--;

			state->ip += 2;
			break;
		}
		case PUSH4:{
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);
			PUSH(state->r[dest]);
			state->ip += 2;
			break;
		}
		case POP4:{
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);
			POP(dest);
			state->ip += 2;
			break;
		}
		case JMP:{
			CHECK_TYPE2_RESERVED(op);

			uint8_t condition = GET_TYPE2_COND(op);

			bool skip = skip_instr(state, condition);
			if (skip){
				state->ip += 6;
				break;
			}

			state->ip = cpu_read32(state, state->ip+2);

			break;
		}
		case CALL:{
			CHECK_TYPE2_RESERVED(op);

			uint8_t condition = GET_TYPE2_COND(op);
			
			bool skip = skip_instr(state, condition);
			if (skip){
				state->ip += 6;
				break;
			}

			PUSH(state->ip+6);

			state->ip = cpu_read32(state, state->ip+2); 
			
			break;
		}
		case INT: {
			CHECK_TYPE2_RESERVED(op);

			uint8_t interrupt = cpu_read8(state, state->ip + 2);

			cpu_interrupt(state, interrupt);
			break;
		}
		case LD1:{
			CHECK_TYPE1_RESERVED(op);

			uint8_t size = GET_TYPE1_SIZE(op);
			uint8_t dest = GET_TYPE1_DEST(op);
			uint32_t ptr = cpu_read32(state, state->ip+2);
			
			switch (size){
				case BYTE: state->r[dest] = (uint32_t)cpu_read8(state, ptr); break;
				case WORD: state->r[dest] = (uint32_t)cpu_read16(state, ptr); break;
				case DWORD: state->r[dest] = cpu_read32(state, ptr); break;
			}

			state->ip += 6;
			break;
		}
		case LD3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t size = GET_TYPE3_SIZE(op);
			uint8_t dest = GET_TYPE3_DEST(op);

			switch (size){
				case BYTE:{
					state->r[dest] = (uint32_t)cpu_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = (uint32_t)cpu_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = cpu_read32(state, state->ip+2);
					state->ip += 6;
					break;
				}
			}

			break;
		}
		case LD6:{
			uint8_t src = GET_TYPE6_SRC(op);
			uint8_t dest = GET_TYPE6_DEST(op);
			uint8_t size = GET_TYPE6_SIZE(op);

			switch (size){
				case BYTE: state->r[dest] = (uint32_t)cpu_read8(state, state->r[src]); break;
				case WORD: state->r[dest] = (uint32_t)cpu_read16(state, state->r[src]); break;
				case DWORD: state->r[dest] = cpu_read32(state, state->r[src]); break;
			}

			state->ip += 2;
			break;
		}
		case ST1:{
			CHECK_TYPE1_RESERVED(op);

			uint8_t size = GET_TYPE1_SIZE(op);
			uint8_t src = GET_TYPE1_DEST(op);	// В данном случае вместо DEST будет SRC
			uint32_t ptr = cpu_read32(state, state->ip+2);

			switch (size){
				case BYTE: cpu_write8(state, ptr, state->r[src]); break;
				case WORD: cpu_write16(state, ptr, state->r[src]); break;
				case DWORD: cpu_write32(state, ptr, state->r[src]); break;
			}

			state->ip += 6;
			break;
		}
		case ST6:{
			uint8_t src = GET_TYPE6_SRC(op);
			uint8_t dest = GET_TYPE6_DEST(op);
			uint8_t size = GET_TYPE6_SIZE(op);

			switch (size){
				case BYTE: cpu_write8(state, state->r[src], (uint8_t)state->r[dest]); break;
				case WORD: cpu_write16(state, state->r[src], (uint16_t)state->r[dest]); break;
				case DWORD: cpu_write32(state, state->r[src], state->r[dest]); break;
			}

			state->ip += 2;
			break;
		}
		case CMP0:{
			CHECK_TYPE0_RESERVED(op);

			uint8_t src = GET_TYPE0_SRC(op);
			uint8_t dest = GET_TYPE0_DEST(op);

			uint32_t x = state->r[dest] - state->r[src];

			state->flags.zero = (x == 0);
			state->flags.negative = (x < 0);

			state->ip += 2;
			break;
		}
		case CMP3:{
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			uint32_t value = 0;

			switch (size){
				case BYTE: {
					value = (uint32_t)cpu_read8(state, state->ip + 2);
					state->ip += 3;
					break;
				}
				case WORD: { 
					value = (uint32_t)cpu_read16(state, state->ip + 2);
					state->ip += 4;
					break;
				}
				case DWORD: { 
					value = cpu_read32(state, state->ip + 2);
					state->ip += 6;
					break;
				}
			}

			uint32_t x = state->r[dest] - value;

			state->flags.zero = (x == 0);
			state->flags.negative = (x < 0);

			//state->ip += (2 + (pow(2, size)));
			break;
		}
		case RET:{
			CHECK_TYPE5_RESERVED(op);

			POP(state->ip);
			break;
		}
		case IRET: {
			CHECK_TYPE5_RESERVED(op);

			state->sp += 4;		// То же самое, что и POP() в никуда

			pop_registers(state);

			POP(state->ip);

			state->ip += 2;
			break;
		}
		case HLT:{
			CHECK_TYPE5_RESERVED(op);

			printf("ИНФО: Остановка (инструкция HLT)\n");
			state->halted = true;
			break;
		}
		case SHL: {
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			uint32_t value = 0;

			switch (size) {
				case BYTE: value = (uint32_t)cpu_read8(state, state->ip + 2); break;
				case WORD: value = (uint32_t)cpu_read16(state, state->ip + 2); break;
				case DWORD: value = cpu_read32(state, state->ip + 2); break;
			}

			state->r[dest] <<= value;

			state->ip += (2 + (pow(2, size)));
			break;
		}
		case SHR: {
			CHECK_TYPE3_RESERVED(op);

			uint8_t dest = GET_TYPE3_DEST(op);
			uint8_t size = GET_TYPE3_SIZE(op);

			uint32_t value = 0;

			switch (size) {
				case BYTE: value = (uint32_t)cpu_read8(state, state->ip + 2); break;
				case WORD: value = (uint32_t)cpu_read16(state, state->ip + 2); break;
				case DWORD: value = cpu_read32(state, state->ip + 2); break;
			}

			state->r[dest] >>= value;

			state->ip += (2 + (pow(2, size)));
			break;
		}
		case LDSP: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t src = GET_TYPE4_DEST(op);

			state->sp = state->r[src];

			state->ip += 2;
			break;
		}
		case STSP: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest] = state->sp;

			state->ip += 2;
			break;
		}
		case LDIT: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t src = GET_TYPE4_DEST(op);

			state->it = state->r[src];

			state->ip += 2;
			break;
		}
		case STIT: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest] = state->it;

			state->ip += 2;
			break;
		}
		case LDMSR: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t src = GET_TYPE4_DEST(op);

			state->msr = state->r[src];
			printf("ИНФО: MSR перезаписан: 0x%08X\r\n", state->msr);

			// Для отладки, в будущем будет удалено
			//if (state->msr & PETUCHPC_MSR_MMU_MASK) {
			//	mmu_debug_print_page_directory(state);
			//}

			state->ip += 2;
			break;
		}
		case STMSR: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest] = state->msr;

			state->ip += 2;
			break;
		}
		case LDPD: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t src = GET_TYPE4_DEST(op);

			state->pd = state->r[src];

			state->ip += 2;
			break;
		}
		case STPD: {
			CHECK_TYPE4_RESERVED(op);

			uint8_t dest = GET_TYPE4_DEST(op);

			state->r[dest] = state->pd;

			state->ip += 2;
			break;
		}
	}
}