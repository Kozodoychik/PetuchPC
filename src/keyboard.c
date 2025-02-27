#include "keyboard.h"
#include "board.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


// Жесть...
uint8_t ps2_scancodes[255] = {
	[SDL_SCANCODE_0] = 0x0b,
	[SDL_SCANCODE_1] = 0x02,
	[SDL_SCANCODE_2] = 0x03,
	[SDL_SCANCODE_3] = 0x04,
	[SDL_SCANCODE_4] = 0x05,
	[SDL_SCANCODE_5] = 0x06,
	[SDL_SCANCODE_6] = 0x07,
	[SDL_SCANCODE_7] = 0x08,
	[SDL_SCANCODE_8] = 0x09,
	[SDL_SCANCODE_9] = 0x0a,

	[SDL_SCANCODE_A] = 0x1e,
	[SDL_SCANCODE_B] = 0x30,
	[SDL_SCANCODE_C] = 0x2e,
	[SDL_SCANCODE_D] = 0x20,
	[SDL_SCANCODE_E] = 0x12,
	[SDL_SCANCODE_F] = 0x21,
	[SDL_SCANCODE_G] = 0x22,
	[SDL_SCANCODE_H] = 0x23,
	[SDL_SCANCODE_I] = 0x17,
	[SDL_SCANCODE_J] = 0x24,
	[SDL_SCANCODE_K] = 0x25,
	[SDL_SCANCODE_L] = 0x26,
	[SDL_SCANCODE_M] = 0x32,
	[SDL_SCANCODE_N] = 0x31,
	[SDL_SCANCODE_O] = 0x18,
	[SDL_SCANCODE_P] = 0x19,
	[SDL_SCANCODE_Q] = 0x10,
	[SDL_SCANCODE_R] = 0x13,
	[SDL_SCANCODE_S] = 0x1f,
	[SDL_SCANCODE_T] = 0x14,
	[SDL_SCANCODE_U] = 0x16,
	[SDL_SCANCODE_V] = 0x2f,
	[SDL_SCANCODE_W] = 0x11,
	[SDL_SCANCODE_X] = 0x2d,
	[SDL_SCANCODE_Y] = 0x15,
	[SDL_SCANCODE_Z] = 0x2c,

	[SDL_SCANCODE_RETURN] = 0x1c,
	[SDL_SCANCODE_ESCAPE] = 0x01,
	[SDL_SCANCODE_BACKSPACE] = 0x0e,
	[SDL_SCANCODE_TAB] = 0x0f,
	[SDL_SCANCODE_SPACE] = 0x39,
	[SDL_SCANCODE_CAPSLOCK] = 0x3a,

	[SDL_SCANCODE_SCROLLLOCK] = 0x46,
};

cpu_state* state;

char keyboard_buffer[16];
uint8_t keyboard_buffer_pointer = 0;

bool use_irq = false;

void keyboard_init(cpu_state* _state) {

	memset(keyboard_buffer, 0, sizeof(char));

	mmio_ports[KEYBOARD_MMIO_BASE].read = keyboard_port_read;
	mmio_ports[KEYBOARD_MMIO_BASE].write = keyboard_port_write;

	state = _state;
}

void keyboard_handle_event(SDL_KeyboardEvent* event) {

	if (keyboard_buffer_pointer <= 15) {
		keyboard_buffer[keyboard_buffer_pointer] = ps2_scancodes[event->keysym.scancode];
		keyboard_buffer_pointer++;

		// Исключительно для отладки
		//printf("Клавиатура: Получен сканкод: 0x%02X\r\n", ps2_scancodes[event->keysym.scancode]);
	}
	else {
		printf("ПРЕДУПРЕЖДЕНИЕ: Клавиатура: Переполнение внутреннего буфера, игнорирование 0x%02X\r\n", ps2_scancodes[event->keysym.scancode]);
	}

	if (use_irq)
		cpu_interrupt(state, IRQ_KEYBOARD);
}

uint8_t keyboard_port_read(cpu_state* state) {
	uint8_t data = keyboard_buffer[0];
	if (!data) return 0;

	memcpy(keyboard_buffer, keyboard_buffer + 1, 15);
	keyboard_buffer[15] = 0;

	if (keyboard_buffer_pointer > 0)
		keyboard_buffer_pointer--;

	return data;
}

void keyboard_port_write(cpu_state* state, uint8_t data) {
	use_irq = (bool)(data && 1);
}

