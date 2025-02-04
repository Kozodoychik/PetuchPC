#pragma once

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

#define DISPLAY_FRAMEBUFFER_BASE 0xa0000000
#define DISPLAY_FRAMEBUFFER_LEN ((DISPLAY_WIDTH * DISPLAY_HEIGHT) * 4)


uint8_t* framebuffer;

void display_init();
int display_update();