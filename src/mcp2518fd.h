/*
 * MCP2518FD_internal.h
 * API and implementation for mcp2518fd driver
*/

#ifndef MCP2518FD_H
#define MCP2518FD_H

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "mcp2518fd_hw.h"

void mcp2518_init();
uint8_t mcp2518fd_read_byte();
uint8_t mcp2518fd_write_byte();
uint8_t mcp2518_read_word();
uint8_t mcp2518_write_word();

#endif