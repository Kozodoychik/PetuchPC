#pragma once

#include <stdbool.h>

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

#define DISPLAY_FRAMEBUFFER_BASE 0xa0000000
#define DISPLAY_FRAMEBUFFER_LEN ((DISPLAY_WIDTH * DISPLAY_HEIGHT) * 4)

#define TEXT_MODE_FONT_WIDTH 8
#define TEXT_MODE_FONT_HEIGHT 16
#define TEXT_MODE_FONT_GLYPH_COUNT 256
#define TEXT_MODE_FONT_FILE "font.bin"

#define MMIO_DISPLAY_COMMAND 0x02

#define MMIO_DISPLAY_COMMAND_CURSOR_X 0b10000000
#define MMIO_DISPLAY_COMMAND_CURSOR_Y 0b01000000
#define MMIO_DISPLAY_COMMAND_MODE 0b00100000
#define MMIO_DISPLAY_COMMAND_VALUE 0b00011111

uint8_t* framebuffer;
uint8_t* font;

void display_init();
int display_update();

void display_command_port_write(cpu_state*, uint8_t);