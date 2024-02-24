#include "cpu.h"
#include "board.h"
#include "display.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


void main_loop();
int load_rom(cpu_state*, char*);

int main(int argc, char* argv[])
{
	char* rom_file;

	if (argc > 1) {
		for (int i=1;i<argc;i++) {
			if (strcmp(argv[i], "-h") == 0) {
				fprintf(stderr, "Использование: %s [параметры]\n"
						"Параметры:\n"
						"  -h				Вывод данного сообщения.\n"
						"  -rom [файл]		Использование образа ПЗУ.\n", argv[0]);
				return 0;
			}
			else if (strcmp(argv[i], "-rom") == 0) {
				if (i+1 != argc){
					rom_file = argv[i+1];
					i++;
				}
			}
			else {
				fprintf(stderr, "Неизвестный параметр: %s\n", argv[i]);
				return 1;
			}
		}
	}

	cpu_state* state = (cpu_state*)malloc(sizeof(cpu_state));

	if (!state){
		fprintf(stderr, "Невозможно выделить память\n");
		return 1;
	}

	cpu_reset(state);

	load_rom(state, rom_file);

	display_init();
	
	main_loop(state);

	free(state);

	return 0;

}

void main_loop(cpu_state* state)
{
	//	Не очень хорошая реализация, но пока пусть будет так
	while (1){
		cpu_execute(state);
		int status = display_update();

		if (status) break;
		getc(stdin);
	}
}

int load_rom(cpu_state* state, char* file)
{
	FILE* rom = fopen(file, "rb");

	if (!rom) {
		fprintf(stderr, "Невозможно открыть образ ПЗУ: %s\n", file);
		return 1;
	}
	fread(&state->rom, 1, PETUCHPC_ROM_SIZE, rom);
	fclose(rom);

	return 0;
}

