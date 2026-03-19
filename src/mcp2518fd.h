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
 Read/write and initialization functions 
 These functions use Pico SDK SPI functions to perform basic operations necessary to interface 
 with the MCP2518fd device.
 I was considering making these functions static, but I chose not to because the application
 might find these functions useful. 

*/
void mcp2518fd_init(uint32_t spi_clk_rate);
uint8_t mcp2518fd_write_byte();
uint8_t mcp2518fd_write_word();
uint8_t mcp2518fd_read_byte();
uint8_t mcp2518fd_read_word();

/*
 Abstracted CAN interface functions
 These functions use the read/write initialization functions defined above
 to create a CAN abstraction layer. 
*/

#endif