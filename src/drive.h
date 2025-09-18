#pragma once

#include <stdint.h>

#include "cpu.h"

#define DRIVE_MMIO_BASE 0x03

#define DRIVE_MMIO_STATUS DRIVE_MMIO_BASE
#define DRIVE_MMIO_COMMAND DRIVE_MMIO_BASE + 0x01

#define DRIVE_MMIO_LBA DRIVE_MMIO_BASE + 0x02


#define DRIVE_STATUS_READY_MASK 0b00000001
#define DRIVE_STATUS_ERR_MASK 0b00000010

#define BLOCK_SIZE 512

#define DRIVE_COMMAND_READ 0x00
#define DRIVE_COMMAND_WRITE 0x01
#define DRIVE_COMMAND_SEEK 0x02


void drive_init(char*);

void drive_lba_write(cpu_state*, uint8_t);