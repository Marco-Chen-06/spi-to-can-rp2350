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

/* structs and enums for API, mainly taken from mcp25XXfd application code*/

typedef enum {
    CAN_NORMAL_MODE = 0b000,
    CAN_SLEEP_MODE = 0b001,
    CAN_INTERNAL_LOOPBACK_MODE = 0b010,
    CAN_LISTEN_ONLY_MODE = 0b011,
    CAN_CONFIGURATION_MODE = 0b100,
    CAN_EXTERNAL_LOOPBACK_MODE = 0b101,
    CAN_CLASSIC_MODE = 0b110,
    CAN_RESTRICTED_MODE = 0b111
} CAN_OPERATION_MODE;

//! CAN Message Object ID
typedef struct CAN_MSGOBJ_ID {
    uint32_t SID : 11;
    uint32_t EID : 18;
    uint32_t SID11: 1;
    uint32_t unimplemented1 : 2;
} CAN_MSGOBJ_ID;

//! CAN Filter Object ID
typedef struct _CAN_FILTEROBJ_ID {
    uint32_t SID : 11;
    uint32_t EID : 18;
    uint32_t SID11 : 1;
    uint32_t EXIDE : 1;
    uint32_t unimplemented1 : 1;
} CAN_FILTEROBJ_ID;

//! CAN Mask Object ID
typedef struct _CAN_MASKOBJ_ID {
    uint32_t MSID : 11;
    uint32_t MEID : 18;
    uint32_t MSID11 : 1;
    uint32_t MIDE : 1;
    uint32_t unimplemented1 : 1;
} CAN_MASKOBJ_ID;

//! CAN TX Message Object Control
typedef struct CAN_TX_MSGOBJ_CTRL {
    uint32_t DLC : 4;
    uint32_t IDE : 1;
    uint32_t RTR : 1;
    uint32_t BRS : 1;
    uint32_t FDF : 1;
    uint32_t ESI: 1;
    uint32_t SEQ : 23;
} CAN_TX_MSGOBJ_CTRL;

/* -- Transmit Message Object (TXQ and TX FIFO) ------------------------------*/
/* This is from MCP25XXFD Datasheet, page 27 and MCP2518FD Datasheet, page 66 */
typedef union {
    struct {
        CAN_MSGOBJ_ID id;
        CAN_TX_MSGOBJ_CTRL ctrl;
        uint32_t timeStamp;
    } bF;
    uint32_t word[3];
    uint8_t byte[12];
} CAN_TX_MSGOBJ;

//! CAN RX Message Object Control
typedef struct _CAN_RX_MSGOBJ_CTRL {
    uint32_t DLC : 4;
    uint32_t IDE : 1;
    uint32_t RTR : 1;
    uint32_t BRS : 1;
    uint32_t FDF : 1;
    uint32_t ESI : 1;
    uint32_t unimplemented1 : 2;
    uint32_t FilterHit : 5;
    uint32_t unimplemented2 : 16;
} CAN_RX_MSGOBJ_CTRL;

typedef union _CAN_RX_MSGOBJ {

    struct {
        CAN_MSGOBJ_ID id;
        CAN_RX_MSGOBJ_CTRL ctrl;
        uint32_t timeStamp;
    } bF;
    uint32_t word[3];
    uint8_t byte[12];
} CAN_RX_MSGOBJ;


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
void mcp2518fd_reset();
void mcp2518fd_ram_init(uint8_t data);
void mcp2518fd_opmode_select(CAN_OPERATION_MODE opmode);

/*
 Abstracted CAN interface functions
 These functions use the read/write initialization functions defined above
 to create a CAN abstraction layer. 
*/



#endif