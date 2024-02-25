#include "cpu.h"
#include "board.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

void cpu_reset(cpu_state* state)
{
	// Инициализируем память, регистры и устанавливаем значения по умолчанию для ip, sp и it

	memset(state->ram, 0, PETUCHPC_RAM_SIZE);
	memset(state->rom, 0, PETUCHPC_ROM_SIZE);

	state->ip = PETUCHPC_ROM_BASE;				// Указатель текущей инструкции
	state->sp = PETUCHPC_STACK_BASE;			// Указатель стека
	state->it = PETUCHPC_INTERRUPT_TABLE_BASE;	// Указатель на таблицу прерываний

	memset(state->r, 0, PETUCHPC_REGISTER_COUNT * sizeof(uint32_t)); // Инициализация регистров

	// Флаги

	state->flags.zero = false;
	state->flags.carry = false;
	state->flags.interrupt = false;
	state->flags.negative = false;
}

void cpu_interrupt(cpu_state* state, int interrupt)
{

}

void print_registers(cpu_state* state)
{
	for (int i=0;i<PETUCHPC_REGISTER_COUNT;i++){
		printf("r%.2d=0x%08X\n", i, state->r[i]);
	}
}

bool skip_instr(cpu_state* state, uint8_t cond)
{
	switch (cond){
		case COND_ALWAYS: return false;
		case COND_EQ: return !state->flags.zero;
		case COND_NEQ: return state->flags.zero;
		case COND_GR: return state->flags.negative;
		case COND_L: return !state->flags.negative;
	}

	fprintf(stderr, "Некорректное условие: 0x%x\n", cond);
	return true;
}

void cpu_execute(cpu_state* state)
{
	if (state->halted) return;

	uint16_t op = board_read16(state, state->ip);
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
					state->r[dest] = state->r[dest] + board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] + board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] + board_read32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] - board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] - board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] - board_read32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] * board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] * board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] * board_read32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] / board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] / board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] / board_read32(state, state->ip+2);
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
		case XCHG:{
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
					state->r[dest] = state->r[dest] & board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] & board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] & board_read32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] | board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] | board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] | board_read32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] ^ board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] ^ board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] ^ board_read32(state, state->ip+2);
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
		// PUSH и POP как-нибудь потом
		case PUSH4:{

			state->ip += 2;
			break;
		}
		case PUSH2:{

			state->ip += (2 + (pow(2, GET_TYPE2_SIZE(op))));
			break;
		}
		case POP4:{

			state->ip += 2;
			break;
		}
		case JMP:{
			CHECK_TYPE2_RESERVED(op);

			uint8_t size = GET_TYPE2_SIZE(op);
			uint8_t condition = GET_TYPE2_COND(op);

			bool skip = skip_instr(state, condition);
			if (skip){
				state->ip += (2 + (pow(2, size)));
				break;
			}

			switch (size){
				case BYTE: state->ip = (uint32_t)board_read8(state, state->ip+2); break;
				case WORD: state->ip = (uint32_t)board_read16(state, state->ip+2); break;
				case DWORD: state->ip = board_read32(state, state->ip+2); break;
			}

			break;
		}
		case LD1:{
			CHECK_TYPE1_RESERVED(op);

			uint8_t size = GET_TYPE1_SIZE(op);
			uint8_t dest = GET_TYPE1_DEST(op);
			uint32_t ptr = board_read32(state, state->ip+2);
			
			switch (size){
				case BYTE: state->r[dest] = (uint32_t)board_read8(state, ptr); break;
				case WORD: state->r[dest] = (uint32_t)board_read16(state, ptr); break;
				case DWORD: state->r[dest] = board_read32(state, ptr); break;
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
					state->r[dest] = (uint32_t)board_read8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = (uint32_t)board_read16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = board_read32(state, state->ip+2);
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
				case BYTE: state->r[dest] = (uint32_t)board_read8(state, state->r[src]); break;
				case WORD: state->r[dest] = (uint32_t)board_read16(state, state->r[src]); break;
				case DWORD: state->r[dest] = board_read32(state, state->r[src]); break;
			}

			break;
		}
		case ST1:{
			CHECK_TYPE1_RESERVED(op);

			uint8_t size = GET_TYPE1_SIZE(op);
			uint8_t src = GET_TYPE1_DEST(op);	// В данном случае вместо DEST будет SRC
			uint32_t ptr = board_read32(state, state->ip+2);

			switch (size){
				case BYTE: board_write8(state, ptr, state->r[src]); break;
				case WORD: board_write16(state, ptr, state->r[src]); break;
				case DWORD: board_write32(state, ptr, state->r[src]); break;
			}

			state->ip += 2;
			break;
		}
		case ST6:{
			uint8_t src = GET_TYPE6_SRC(op);
			uint8_t dest = GET_TYPE6_DEST(op);
			uint8_t size = GET_TYPE6_SIZE(op);

			switch (size){
				case BYTE: board_write8(state, state->r[dest], (uint8_t)state->r[src]); break;
				case WORD: board_write16(state, state->r[dest], (uint16_t)state->r[src]); break;
				case DWORD: board_write32(state, state->r[dest], state->r[src]); break;
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
				case BYTE: value = (uint32_t)board_read8(state, state->ip+2); break;
				case WORD: value = (uint32_t)board_read16(state, state->ip+2); break;
				case DWORD: value = board_read32(state, state->ip+2); break;
			}

			uint32_t x = state->r[dest] - value;

			state->flags.zero = (x == 0);
			state->flags.negative = (x < 0);

			state->ip += (2 + (pow(2, size)));
			break;
		}
		case HLT:{
			printf("Emulator halted!\n");
			state->halted = true;
			break;
		}
	}
	// Вывод регистров и опкода, потом уберу
	/*print_registers(state);
	printf("ip: 0x%08x\n", state->ip);
	printf("%x\n", op);*/
}
