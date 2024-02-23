#include "cpu.h"
#include "memory.h"

#include <stdio.h>
#include <math.h>

void cpu_init(cpu_state* state)
{
	// Инициализируем память, регистры и устанавливаем значения по умолчанию для ip, sp и it

	for (int i=0;i<PETUCHPC_RAM_SIZE;i++) state->ram[i] = 0;
	for (int i=0;i<PETUCHPC_ROM_SIZE;i++) state->rom[i] = 0;

	state->ip = PETUCHPC_ROM_BASE;				// Указатель текущей инструкции
	state->sp = PETUCHPC_STACK_BASE;			// Указатель стека
	state->it = PETUCHPC_INTERRUPT_TABLE_BASE;	// Указатель на таблицу прерываний

	for (int i=0;i<PETUCHPC_REGISTER_COUNT;i++) state->r[i] = 0; // Инициализация регистров

	// Флаги

	state->flags.zero = false;
	state->flags.carry = false;
	state->flags.interrupt = false;
	state->flags.negative = false;
}

void cpu_interrupt(cpu_state* state, int interrupt)
{

}

void print_registers(cpu_state* state){
	for (int i=0;i<PETUCHPC_REGISTER_COUNT;i++){
		printf("r%.2d=0x%016X\n", i, state->r[i]);
	}
}

void cpu_execute(cpu_state* state)
{
	uint16_t op = memory_get16(state, state->ip);
	switch(GET_OPCODE(op)){
		case NOP:{
			CHECK_TYPE5_RESERVED(op);
			state->ip++;
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
					state->r[dest] = state->r[dest] + memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] + memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] + memory_get32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] - memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] - memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] - memory_get32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] * memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] * memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] * memory_get32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] / memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] / memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] / memory_get32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] & memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] & memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] & memory_get32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] | memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] | memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] | memory_get32(state, state->ip+2);
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
					state->r[dest] = state->r[dest] ^ memory_get8(state, state->ip+2);
					state->ip += 3;
					break;
				}
				case WORD:{
					state->r[dest] = state->r[dest] ^ memory_get16(state, state->ip+2);
					state->ip += 4;
					break;
				}
				case DWORD:{
					state->r[dest] = state->r[dest] ^ memory_get32(state, state->ip+2);
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
			break;
		}
		case DEC:{
			CHECK_TYPE4_RESERVED(op);
			uint8_t dest = GET_TYPE4_DEST(op);
			state->r[dest]--;
			state->ip += 2;
			break;
			break;
		}
		case PUSH4:{

			state->ip += 2;
			break;
		}
		case PUSH2:{

			state->ip += (1 + (pow(2, GET_TYPE2_SIZE(op))));
			break;
		}
		case POP4:{

			state->ip += 2;
			break;
		}
		case JMP:{
			
		}
	}
	print_registers(state);
}
