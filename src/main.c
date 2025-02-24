#include "cpu.h"
#include "board.h"
#include "display.h"
#include "keyboard.h"

#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <locale.h>


void main_loop(cpu_state* state)
{
	int tick_start = SDL_GetTicks();

	state->halted = false;
	//int tick_end = SDL_GetTicks();

	while (1) {
		int delta_time = SDL_GetTicks() - tick_start;
		int tick_start = SDL_GetTicks();

		if (!delta_time) delta_time = 1;

		int cpt = 33000000 / 60 / delta_time;
		int extra_cycles = 33000000 / 60 - (cpt * delta_time);

		for (int i = 0;i < delta_time;i++) {
			int cycles_left = cpt;

			if (i == delta_time - 1) cycles_left += extra_cycles;

			while (cycles_left > 0) {
				cpu_execute(state);
				//getc(stdin);
				cycles_left--;
			}
		}

		int status = display_update();
		if (status) break;
	}
}
int load_rom(cpu_state*, char*);

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");

	char* rom_file = NULL;

	if (argc > 1) {
		for (int i=1;i<argc;i++) {
			if (strcmp(argv[i], "-h") == 0) {
				fprintf(stderr, "Использование: %s [параметры]\n"
						"Параметры:\n"
						"  -h					Вывод данного сообщения.\n"
						"  -rom [файл]			Использование образа ПЗУ.\n"
						"  -d					Дамп ОЗУ после остановки процессора", argv[0]);
				return 0;
			}
			else if (strcmp(argv[i], "-rom") == 0) {
				if (i+1 != argc){
					rom_file = argv[i+1];
					i++;
				}
			}
			else {
				fprintf(stderr, "ОШИБКА: Неизвестный параметр: %s\n", argv[i]);
				return 1;
			}
		}
	}

	cpu_state* state = (cpu_state*)malloc(sizeof(cpu_state));

	if (!state){
		fprintf(stderr, "ОШИБКА: Невозможно выделить память\n");
		return 1;
	}

	cpu_reset(state);

	if (rom_file)
		load_rom(state, rom_file);
	else
		load_rom(state, "bios.bin");

	board_init(state);
	keyboard_init(state);
	display_init();
	
	main_loop(state);

	free(state);

	return 0;

}

int load_rom(cpu_state* state, char* file)
{
	FILE* rom = fopen(file, "rb");

	if (!rom) {
		fprintf(stderr, "ОШИБКА: Невозможно открыть образ ПЗУ: %s\n", file);
		return 1;
	}
	fread(&state->rom, 1, PETUCHPC_ROM_SIZE, rom);
	fclose(rom);

	/*
	FILE* dump = fopen("rom_dump.bin", "wb");
	fwrite(&state->rom, 1, PETUCHPC_ROM_SIZE, dump);
	fclose(dump);
	*/

	return 0;
}

