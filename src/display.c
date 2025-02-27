#include "cpu.h"
#include "display.h"
#include "keyboard.h"
#include "board.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


SDL_Renderer* display_renderer;
SDL_Window* display_window;
SDL_Texture* display_texture;

SDL_Rect display_rect;

uint8_t* texture_buffer;

// Для мигающего курсора
uint8_t cursor_timer = 0;

/*  ВИДЕОРЕЖИМЫ
    0 - Текстовый 80x25 (шрифт 8x16)
    1 - Графический (640x480 32 бита)
*/
uint8_t mode = 1;

uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

uint32_t palette[256] = {
    0x000000,
    0x0000aa,
    0x00aa00,
    0x00aaaa,
    0xaa0000,
    0xaa00aa,
    0xaa5500,
    0xaaaaaa,
    0x555555,
    0x5555ff,
    0x55ff55,
    0x55ffff,
    0xff5555,
    0xff55ff,
    0xffff55,
    0xffffff
};

int display_update();

void display_init() {
	SDL_Init(SDL_INIT_VIDEO);

	display_window = SDL_CreateWindow(
        "PetuchPC Emu", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        DISPLAY_WIDTH, DISPLAY_HEIGHT, 
        0
    );
    if (!display_window){
        fprintf(stderr, "ОШИБКА: SDL_CreateWindow\n");
        return;
    }

    display_renderer = SDL_CreateRenderer(display_window, -1, 0);
    if (!display_renderer){
        fprintf(stderr, "ОШИБКА: SDL_CreateRenderer\n");
        return;
    }

    display_rect = (SDL_Rect){
        .w = DISPLAY_WIDTH,
        .h = DISPLAY_HEIGHT
    };

    display_texture = SDL_CreateTexture(
        display_renderer, 
        SDL_PIXELFORMAT_RGB888, 
        SDL_TEXTUREACCESS_STREAMING,
        DISPLAY_WIDTH, DISPLAY_HEIGHT
    );

    framebuffer = (uint8_t*)calloc((DISPLAY_WIDTH * DISPLAY_HEIGHT) * 4, 1);
    texture_buffer = (uint8_t*)calloc((DISPLAY_WIDTH * DISPLAY_HEIGHT) * 4, 1);
    font = (uint8_t*)calloc((TEXT_MODE_FONT_HEIGHT * TEXT_MODE_FONT_GLYPH_COUNT), 1);

    if (!font) {
        fprintf(stderr, "ОШИБКА: Невозможно выделить память под образ шрифта\r\n");
        return;
    }

    FILE* font_file = fopen(TEXT_MODE_FONT_FILE, "rb");

    if (!font_file) {
        fprintf(stderr, "ОШИБКА: Невозможно открыть образ шрифта: %s\r\n", TEXT_MODE_FONT_FILE);
    }
    else {
        fread(font, 1, (TEXT_MODE_FONT_HEIGHT * TEXT_MODE_FONT_GLYPH_COUNT), font_file);
        fclose(font_file);
    }

    mmio_ports[MMIO_DISPLAY_COMMAND].write = display_command_port_write;

    display_update();

    SDL_ShowWindow(display_window);

}

int display_update() {
    switch (mode) {
        case 0: {
            // Фиговая реализация текстового режима
            for (int i = 0;i < (80 * 25);i++) {
                uint8_t character = framebuffer[i * 2];
                uint8_t attribute = framebuffer[i * 2 + 1];

                uint32_t fg = palette[attribute & 0x0f];
                uint32_t bg = palette[(attribute & 0xf0) >> 4];

                uint8_t* glyph = font + (character * TEXT_MODE_FONT_HEIGHT);

                for (int y = 0;y < TEXT_MODE_FONT_HEIGHT;y++) {
                    uint8_t line = *glyph;
                    for (int x = 0;x < TEXT_MODE_FONT_WIDTH;x++) {
                        int base_x = (i % 80) * TEXT_MODE_FONT_WIDTH;
                        int base_y = (i / 80) * TEXT_MODE_FONT_HEIGHT;
                        
                        // Мигающий курсор
                        if ((i % 80) == cursor_x && (i / 80) == cursor_y && cursor_timer > 30 && y > 12) {
                            bg = fg;
                        }

                        texture_buffer[((base_x + x) + ((base_y + y) * DISPLAY_WIDTH)) * 4 + 2] = (line & 0x80) ? ((fg & 0xff0000) >> 16) : ((bg & 0xff0000) >> 16);
                        texture_buffer[(((base_x + x) + ((base_y + y) * DISPLAY_WIDTH)) * 4) + 1] = (line & 0x80) ? ((fg & 0x00ff00) >> 8) : ((bg & 0x00ff00) >> 8);
                        texture_buffer[(((base_x + x) + ((base_y + y) * DISPLAY_WIDTH)) * 4)] = (line & 0x80) ? (fg & 0xff) : (bg & 0xff);

                        line <<= 1;
                    }
                    glyph++;
                }
            }
            break;
        }
        case 1: {
            memcpy(texture_buffer, framebuffer, DISPLAY_FRAMEBUFFER_LEN);
            break;
        }
    }
    if (cursor_timer < 60)
        cursor_timer++;
    else
        cursor_timer = 0;

    SDL_UpdateTexture(display_texture, NULL, texture_buffer, DISPLAY_WIDTH * 4);

    SDL_RenderClear(display_renderer);
    SDL_RenderCopy(display_renderer, display_texture, NULL, NULL);
    SDL_RenderPresent(display_renderer);

    SDL_Event e;
	while (SDL_PollEvent(&e)){
        switch(e.type){
            case SDL_QUIT:{
                SDL_DestroyTexture(display_texture);
                SDL_DestroyRenderer(display_renderer);
                SDL_DestroyWindow(display_window);
                SDL_Quit();
                
                free(framebuffer);
                free(font);
                return -1;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                keyboard_handle_event(&e.key);
                break;
            }
        }
	}
    return 0;
}

void display_command_port_write(cpu_state* state, uint8_t command) {
    uint8_t value = command & MMIO_DISPLAY_COMMAND_VALUE;
    
    if (command & MMIO_DISPLAY_COMMAND_CURSOR_X) {
        cursor_x = value;
    }
    if (command & MMIO_DISPLAY_COMMAND_CURSOR_Y) {
        cursor_y = value;
    }
    
    if (command & MMIO_DISPLAY_COMMAND_MODE) {
        mode = value;
        switch (mode) {
            case 0: {
                SDL_SetWindowSize(display_window, 640, 400);
            }
            case 1: {
                SDL_SetWindowSize(display_window, 640, 480);
            }
        }
    }
}