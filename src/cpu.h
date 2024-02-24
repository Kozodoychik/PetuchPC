#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PETUCHPC_RAM_SIZE 0x04000000
#define PETUCHPC_ROM_SIZE 0x00100000

#define PETUCHPC_INTERRUPT_TABLE_BASE 0x00000000
#define PETUCHPC_STACK_BASE 0x04000000
#define PETUCHPC_ROM_BASE 0xf0000000

#define PETUCHPC_REGISTER_COUNT 16

#define GET_OPCODE(a) ((uint8_t)((a & 0b1111110000000000) >> 10))

#define GET_TYPE0_DEST(a) ((uint8_t)((a & 0b0000001111000000) >> 6))
#define GET_TYPE0_SRC(a) ((uint8_t)((a & 0b0000000000111100) >> 2))
#define GET_TYPE0_RESERVED(a) ((uint8_t)(a & 0b11))

#define GET_TYPE1_SIZE(a) ((uint8_t)((a & 0b0000001100000000) >> 8))
#define GET_TYPE1_DEST(a) ((uint8_t)((a & 0b0000000011110000) >> 4))
#define GET_TYPE1_RESERVED(a) ((uint8_t)(a & 0b1111))

#define GET_TYPE2_SIZE(a) ((uint8_t)(a & 0b0000001100000000) >> 8)
#define GET_TYPE2_RESERVED(a) ((uint8_t)(a & 0b11111111))

#define GET_TYPE3_SIZE(a) ((uint8_t)((a & 0b0000001100000000) >> 8))
#define GET_TYPE3_DEST(a) ((uint8_t)((a & 0b0000000011110000) >> 4))
#define GET_TYPE3_RESERVED(a) ((uint8_t)(a & 0b1111))

#define GET_TYPE4_DEST(a) ((uint8_t)((a & 0b0000001111000000) >> 6))
#define GET_TYPE4_RESERVED(a) ((uint8_t)(a & 0b0000000000111111))

#define GET_TYPE5_RESERVED(a) ((uint8_t)(a & 0b1111111111))

#define CHECK_TYPE0_RESERVED(a) ({ if (GET_TYPE0_RESERVED(a) != 0) fprintf(stderr, "Reserved bits != 0!!!"); })
#define CHECK_TYPE1_RESERVED(a) ({ if (GET_TYPE1_RESERVED(a) != 0) fprintf(stderr, "Reserved bits != 0!!!"); })
#define CHECK_TYPE2_RESERVED(a) ({ if (GET_TYPE2_RESERVED(a) != 0) fprintf(stderr, "Reserved bits != 0!!!"); })
#define CHECK_TYPE3_RESERVED(a) ({ if (GET_TYPE3_RESERVED(a) != 0) fprintf(stderr, "Reserved bits != 0!!!"); })
#define CHECK_TYPE4_RESERVED(a) ({ if (GET_TYPE4_RESERVED(a) != 0) fprintf(stderr, "Reserved bits != 0!!!"); })
#define CHECK_TYPE5_RESERVED(a) ({ if (GET_TYPE5_RESERVED(a) != 0) fprintf(stderr, "Reserved bits != 0!!!"); })

typedef struct {

	bool zero;
	bool carry;
	bool interrupt;
	bool negative;

} cpu_flags;

typedef struct {

	uint32_t r[PETUCHPC_REGISTER_COUNT];
	uint32_t ip;
	uint32_t sp;
	uint32_t it;

	bool halted;

	cpu_flags flags;

	uint8_t ram[PETUCHPC_RAM_SIZE];
	uint8_t rom[PETUCHPC_ROM_SIZE];

} cpu_state;

typedef enum {
	BYTE,
	WORD,
	DWORD
} cpu_operand_size;

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
	XCHG,
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
	PUSH2,
	POP4,
	JMP,
	CALL,
	INT,
	LD1,
	LD3,
	ST,
	CMP0,
	CMP3,
	RET,
	IRET,
	HLT
} cpu_opcode;

void cpu_reset(cpu_state*);
void cpu_interrupt(cpu_state*, int);
void cpu_execute(cpu_state*);
