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

/*
 read/write initialization functions 
*/
void mcp2518fd_init(uint32_t spi_clk_rate);
uint8_t mcp2518fd_read_byte();
uint8_t mcp2518fd_write_byte();
uint8_t mcp2518fd_read_word();
uint8_t mcp2518fd_write_word();

/*
 abstracted CAN interface functions
 (these functions call the read/write initialization functions)
*/

#endif