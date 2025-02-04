#include "cpu.h"
#include "display.h"
#include "keyboard.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>


SDL_Renderer* display_renderer;
SDL_Window* display_window;
SDL_Texture* display_texture;

SDL_Rect display_rect;

int display_update();

void display_init()
{
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

    display_update();

    SDL_ShowWindow(display_window);

}

int display_update(){
    SDL_UpdateTexture(display_texture, NULL, framebuffer, DISPLAY_WIDTH * 4);

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
                return -1;
            }
        }
	}
    return 0;
}