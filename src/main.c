#include "cpu.h"
#include "memory.h"

#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


int load_rom(cpu_state*, char*);

int main(int argc, char* argv[])
{
	char* rom_file;

	if (argc > 1) {
		for (int i=1;i<argc;i++) {
			if (strcmp(argv[i], "-h") == 0) {
				fprintf(stderr, "Usage: %s [options]\n"
						"Options:\n"
						"  -h		Shows this message.\n", argv[0]);
				return 0;
			}
			else if (strcmp(argv[i], "-rom") == 0) {
				if (i+1 != argc){
					rom_file = argv[i+1];
					i++;
				}
			}
			else {
				fprintf(stderr, "wtf\n");
				return 1;
			}
		}
	}

	cpu_state* state = (cpu_state*)malloc(sizeof(cpu_state));

	if (!state){
		fprintf(stderr, "Cannot allocate memory for cpu state\n");
		return 1;
	}

	cpu_init(state);

	load_rom(state, rom_file);

	memory_init(state, PETUCHPC_DEFAULT_INSTR_PTR, PETUCHPC_ROM_SIZE);

	cpu_execute(state);
	/*SDL_Window* win;
	SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("shto", 100, 100, 640, 480, 0);
	SDL_Delay(3000);
	SDL_DestroyWindow(win);
	SDL_Quit();
	*/

	return 0;

}

int load_rom(cpu_state* state, char* file)
{
	FILE* rom = fopen(file, "rb");

	if (!rom) {
		fprintf(stderr, "Cannot open ROM file: %s\n", file);
		return 1;
	}
	fread(&state->rom, 1, PETUCHPC_ROM_SIZE, rom);
	fclose(rom);

	return 0;
}

