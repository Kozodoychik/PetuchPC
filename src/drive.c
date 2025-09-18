#define _CRT_SECURE_NO_DEPRECATE

#include "drive.h"
#include "board.h"

#include <stdio.h>
#include <stdlib.h>


FILE* image;

uint8_t* buffer;

uint16_t lba = 0;

void drive_init(char* filename) {

	image = fopen(filename, "rb+");

	if (!image) {
		fprintf(stderr, "ОШИБКА: Накопитель: Невозможно открыть образ: %s\n", filename);
		return;
	}

	buffer = (uint8_t*)calloc(BLOCK_SIZE, 1);

	mmio_ports[DRIVE_MMIO_LBA].write = drive_lba_write;
}

void drive_lba_write(cpu_state* state, uint8_t value) {
	lba <<= 8;
	lba &= value;
}