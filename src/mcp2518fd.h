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
  Basic read/write functions
  Use Pico SDK SPI library to perform basic read/write operations to interface with
  the mcp2518fd
*/
uint8_t mcp2518fd_read_byte(uint16_t addr, uint8_t *data);
uint8_t mcp2518fd_read_word(uint16_t addr, uint32_t *data);
uint8_t mcp2518fd_write_byte(uint16_t addr, uint8_t data);
uint8_t mcp2518fd_write_word(uint16_t addr, uint32_t data);

/* 
  mcp2518fd hardware init and configuration functions
  Initialize and configure the mcp2518fd hardware registers.
  People using this driver for CAN should not touch these. 
*/
int8_t mcp2518fd_init(uint32_t spi_clk_rate);
static void mcp2518fd_spi_init(uint32_t spi_clk_rate);
void mcp2518fd_reset();
void mcp2518fd_ram_init(uint8_t data);
void mcp2518fd_opmode_select(CAN_OPERATION_MODE opmode);

/*
 Abstracted CAN interface functions
 These functions use the read/write initialization functions defined above
 to create a CAN abstraction layer. 
*/


#endif