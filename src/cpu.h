#pragma once

#include <stdint.h>
#include <stdbool.h>

// сколько же тут макросов...

#define PETUCHPC_RAM_SIZE 0x04000000
#define PETUCHPC_ROM_SIZE 0x01000000

#define PETUCHPC_INTERRUPT_TABLE_BASE 0x00000000
#define PETUCHPC_STACK_BASE 0x03ffffff
#define PETUCHPC_ROM_BASE 0xf0000000

#define PETUCHPC_REGISTER_COUNT 16

#define PETUCHPC_MSR_MMU_MASK 1

#define GET_OPCODE(a) ((uint8_t)((a & 0b1111110000000000) >> 10))

#define GET_TYPE0_DEST(a) ((uint8_t)((a & 0b0000001111000000) >> 6))
#define GET_TYPE0_SRC(a) ((uint8_t)((a & 0b0000000000111100) >> 2))
#define GET_TYPE0_RESERVED(a) ((uint8_t)(a & 0b11))

#define GET_TYPE1_SIZE(a) ((uint8_t)((a & 0b0000001100000000) >> 8))
#define GET_TYPE1_DEST(a) ((uint8_t)((a & 0b0000000011110000) >> 4))
#define GET_TYPE1_RESERVED(a) ((uint8_t)(a & 0b1111))

#define GET_TYPE2_COND(a) ((uint8_t)((a & 0b1110000000) >> 7))
#define GET_TYPE2_RESERVED(a) ((uint8_t)(a & 0b11111))

#define GET_TYPE3_SIZE(a) ((uint8_t)((a & 0b0000001100000000) >> 8))
#define GET_TYPE3_DEST(a) ((uint8_t)((a & 0b0000000011110000) >> 4))
#define GET_TYPE3_RESERVED(a) ((uint8_t)(a & 0b1111))

#define GET_TYPE4_DEST(a) ((uint8_t)((a & 0b0000001111000000) >> 6))
#define GET_TYPE4_RESERVED(a) ((uint8_t)(a & 0b0000000000111111))

#define GET_TYPE5_RESERVED(a) ((uint8_t)(a & 0b1111111111))

#define GET_TYPE6_DEST(a) ((uint8_t)((a & 0b0000001111000000) >> 6))
#define GET_TYPE6_SRC(a) ((uint8_t)((a & 0b0000000000111100) >> 2))
#define GET_TYPE6_SIZE(a) ((uint8_t)(a & 0b11))

#define CHECK_TYPE0_RESERVED(a) { if (GET_TYPE0_RESERVED(a) != 0) fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Зарезервированные биты != 0!!! (0x%02X)\r\n", a);  }
#define CHECK_TYPE1_RESERVED(a) { if (GET_TYPE1_RESERVED(a) != 0) fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Зарезервированные биты != 0!!! (0x%02X)\r\n", a);  }
#define CHECK_TYPE2_RESERVED(a) { if (GET_TYPE2_RESERVED(a) != 0) fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Зарезервированные биты != 0!!! (0x%02X)\r\n", a);  }
#define CHECK_TYPE3_RESERVED(a) { if (GET_TYPE3_RESERVED(a) != 0) fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Зарезервированные биты != 0!!! (0x%02X)\r\n", a);  }
#define CHECK_TYPE4_RESERVED(a) { if (GET_TYPE4_RESERVED(a) != 0) fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Зарезервированные биты != 0!!! (0x%02X)\r\n", a);  }
#define CHECK_TYPE5_RESERVED(a) { if (GET_TYPE5_RESERVED(a) != 0) fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ: Зарезервированные биты != 0!!! (0x%02X)\r\n", a);  }

#define PUSH(value)	{ state->sp -= 4; cpu_write32(state, state->sp, value); }


#define POP(dest) {dest = cpu_read32(state, state->sp); state->sp += 4; }

typedef struct {

	bool zero;
	bool carry;
	bool interrupt;
	bool negative;

} cpu_flags;

typedef struct {

	bool ram_dump_on_halt;

} cpu_emulation_settings;

typedef struct {

	uint32_t r[PETUCHPC_REGISTER_COUNT];	// Регистры общего назначения
	uint32_t ip;							// Указатель инструкции
	uint32_t sp;							// Указатель стека
	uint32_t it;							// Указатель на начало векторной таблицы прерываний
	uint32_t msr;							// Регистр состояния машины
	uint32_t pd;							// Указатель на page directory

	bool halted;							// true если процессор остановлен (инструкция HLT)

	cpu_flags flags;						// Флаги

	uint8_t ram[PETUCHPC_RAM_SIZE];			// ОЗУ
	uint8_t rom[PETUCHPC_ROM_SIZE];			// ПЗУ

	cpu_emulation_settings settings;

} cpu_state;

typedef enum {
	BYTE,
	WORD,
	DWORD
} cpu_operand_size;

typedef enum {
	COND_ALWAYS,	// Всегда
	COND_EQ,		// Равно (zero = 1)
	COND_NEQ,		// Не равно (zero = 0)
	COND_GR,		// Больше (negative = 0)
	COND_L			// Меньше (negative = 1)
} cpu_condition;

typedef enum {
	NOP,
	ADD0,
	ADD3,
	SUB0,
	SUB3,
	MUL0,
	MUL3,
	DIV0,
	DIV3,
	CPY,
	SWP,
	AND0,
	AND3,
	OR0,
	OR3,
	NOT,
	XOR0,
	XOR3,
	INC,
	DEC,
	PUSH4,
	POP4,
	JMP,
	CALL,
	INT,
	LD1,
	LD3,
	LD6,
	ST1,
	ST6,
	CMP0,
	CMP3,
	RET,
	IRET,
	HLT,
	LDIT,
	STIT,
	LDSP,
	STSP,
	LDMSR,
	STMSR,
	LDPD,
	STPD,
	SHL,
	SHR
} cpu_opcode;

void cpu_reset(cpu_state*);

uint8_t cpu_read8(cpu_state*, uint32_t);
uint16_t cpu_read16(cpu_state*, uint32_t);
uint32_t cpu_read32(cpu_state*, uint32_t);

void cpu_write8(cpu_state*, uint32_t, uint8_t);
void cpu_write16(cpu_state*, uint32_t, uint16_t);
void cpu_write32(cpu_state*, uint32_t, uint32_t);

void cpu_interrupt(cpu_state*, int);
void cpu_execute(cpu_state*);
