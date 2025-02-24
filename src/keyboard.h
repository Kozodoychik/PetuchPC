#pragma once

#include "cpu.h"
#include "irq.h"

#include <stdint.h>
#include <SDL.h>

#define KEYBOARD_MMIO_BASE 0x01


void keyboard_init(cpu_state*);
void keyboard_handle_event(SDL_KeyboardEvent*);

uint8_t keyboard_port_read(cpu_state*);
void keyboard_port_write(cpu_state*, uint8_t);