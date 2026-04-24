/* 
  Long term todo: Update all functions to allow choosing which CAN module 
  you want (can0, can1, ...), this will need updating the spi intructions too.

  In addition, add a function that lets you write SPI in arrays because thats
  more efficient then writing 1 word at at time in some cases.
*/

/*
 * MCP2518FD.h
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

//! CAN FIFO Channels

typedef enum {
	CAN_FIFO_CH0, // CAN_TXQUEUE_CH0
	CAN_FIFO_CH1,
	CAN_FIFO_CH2,
	CAN_FIFO_CH3,
	CAN_FIFO_CH4,
	CAN_FIFO_CH5,
	CAN_FIFO_CH6,
	CAN_FIFO_CH7,
	CAN_FIFO_CH8,
	CAN_FIFO_CH9,
	CAN_FIFO_CH10,
	CAN_FIFO_CH11,
	CAN_FIFO_CH12,
	CAN_FIFO_CH13,
	CAN_FIFO_CH14,
	CAN_FIFO_CH15,
	CAN_FIFO_CH16,
	CAN_FIFO_CH17,
	CAN_FIFO_CH18,
	CAN_FIFO_CH19,
	CAN_FIFO_CH20,
	CAN_FIFO_CH21,
	CAN_FIFO_CH22,
	CAN_FIFO_CH23,
	CAN_FIFO_CH24,
	CAN_FIFO_CH25,
	CAN_FIFO_CH26,
	CAN_FIFO_CH27,
	CAN_FIFO_CH28,
	CAN_FIFO_CH29,
	CAN_FIFO_CH30,
	CAN_FIFO_CH31,
	CAN_FIFO_TOTAL_CHANNELS
} CAN_FIFO_CHANNEL;

// FIFO0 is a special FIFO, the TX Queue
#define CAN_TXQUEUE_CH0 CAN_FIFO_CH0

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

//! CAN Nominal Bit Time Setup

typedef enum { CAN_NBT_125K, CAN_NBT_250K, CAN_NBT_500K, CAN_NBT_1M } CAN_NOMINAL_BITTIME_SETUP;

//! CAN Configure
typedef struct _CAN_CONFIG {
	uint32_t DNetFilterCount : 5;
	uint32_t IsoCrcEnable : 1;
	uint32_t ProtocolExpectionEventDisable : 1;
	uint32_t WakeUpFilterEnable : 1;
	uint32_t WakeUpFilterTime : 2;
	uint32_t BitRateSwitchDisable : 1;
	uint32_t RestrictReTxAttempts : 1;
	uint32_t EsiInGatewayMode : 1;
	uint32_t SystemErrorToListenOnly : 1;
	uint32_t StoreInTEF : 1;
	uint32_t TXQEnable : 1;
	uint32_t TxBandWidthSharing : 4;
} CAN_CONFIG;

//! Oscillator Conrol
typedef struct _CAN_OSC_CTRL {
	uint32_t PllEnable : 1; /* bit  0  - Enable PLL (×10 from XTAL)      */
	uint32_t unimplemented1 : 1;
	uint32_t OscDisable : 1; /* bit  2  - Disable oscillator              */
	uint32_t LowPowerModeEnable : 1; /* bit  3  - Low power mode (MCP2518FD only) */
	uint32_t SCLKDIV : 1; /* bit  4  - System clock divisor (1 or 2)   */
	uint32_t CLKODIV : 2; /* bits 6:5 - Clock output divisor           */
	uint32_t unimplemented2 : 1;
	uint32_t PllReady : 1; /* bit  8  - PLL locked (read only)          */
	uint32_t unimplemented3 : 1;
	uint32_t OscReady : 1; /* bit 10  - Oscillator running (read only)  */
	uint32_t unimplemented4 : 1;
	uint32_t SclkReady : 1; /* bit 12  - System clock stable (read only) */
	uint32_t unimplemented5 : 19;
} CAN_OSC_CTRL;

// input-output control based on IOCON
typedef struct _CAN_IO_CTRL {
	uint32_t TRIS0 : 1; /* bit  0  - GPIO0 direction (0=out, 1=in)   */
	uint32_t TRIS1 : 1; /* bit  1  - GPIO1 direction                 */
	uint32_t unimplemented1 : 2;
	uint32_t ClearAutoSleepOnMatch : 1; /* bit 4 - Clear auto-sleep on filter match */
	uint32_t AutoSleepEnable : 1; /* bit  5  - Auto-sleep enable               */
	uint32_t XcrSTBYEnable : 1; /* bit  6  - XSTBY pin control               */
	uint32_t unimplemented2 : 1;
	uint32_t LAT0 : 1; /* bit  8  - GPIO0 latch (output value)      */
	uint32_t LAT1 : 1; /* bit  9  - GPIO1 latch                     */
	uint32_t unimplemented3 : 5;
	uint32_t HVDETSEL : 1; /* bit 15  - High voltage detect select      */
	uint32_t GPIO0 : 1; /* bit 16  - GPIO0 input state (read only)   */
	uint32_t GPIO1 : 1; /* bit 17  - GPIO1 input state               */
	uint32_t unimplemented4 : 6;
	uint32_t PinMode0 : 1; /* bit 24  - INT0/GPIO0 pin mode             */
	uint32_t PinMode1 : 1; /* bit 25  - INT1/GPIO1 pin mode             */
	uint32_t unimplemented5 : 2;
	uint32_t TXCANOpenDrain : 1; /* bit 28  - TXCAN open drain mode           */
	uint32_t SOFOutputEnable : 1; /* bit 29  - SOF signal output enable        */
	uint32_t INTPinOpenDrain : 1; /* bit 30  - INT pins open drain mode        */
	uint32_t unimplemented6 : 1;
} CAN_IO_CTRL;

//! CAN Transmit Channel Configure
typedef struct _CAN_TX_FIFO_CONFIG {
	uint32_t RTREnable : 1;
	uint32_t TxPriority : 5;
	uint32_t TxAttempts : 2;
	uint32_t FifoSize : 5;
	uint32_t PayLoadSize : 3;
} CAN_TX_FIFO_CONFIG;

//! CAN Receive Channel Configure
typedef struct _CAN_RX_FIFO_CONFIG {
	uint32_t RxTimeStampEnable : 1;
	uint32_t FifoSize : 5;
	uint32_t PayLoadSize : 3;
} CAN_RX_FIFO_CONFIG;

/* NEED THE FOLLOWING CONFIGS:*/
/* 
ciFifocon1
ciFifocon2

*/
/* ------------ */

//! CAN Message Object ID
typedef struct CAN_MSGOBJ_ID {
	uint32_t SID : 11;
	uint32_t EID : 18;
	uint32_t SID11 : 1;
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
	uint32_t ESI : 1;
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
void mcp2518fd_spi_init(uint32_t spi_clk_rate);
void mcp2518fd_reset();
void mcp2518fd_ram_init(uint8_t data);

/*
 Abstracted CAN interface functions (API functions)
 These functions use the read/write initialization functions defined above
 to create a CAN abstraction layer. 
*/

int8_t mcp2518fd_configure(CAN_CONFIG *config);
int8_t mcp2518fd_configure_reset(CAN_CONFIG *config);
int8_t mcp2518fd_osc_configure(CAN_OSC_CTRL *config);
int8_t mcp2518fd_osc_configure_reset(CAN_OSC_CTRL *config);
int8_t mcp2518fd_io_configure(CAN_IO_CTRL *config);
int8_t mcp2518fd_io_configure_reset(CAN_IO_CTRL *config);

int8_t mcp2518fd_tx_fifo_configure(CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_CONFIG *config);
int8_t mcp2518fd_tx_fifo_configure_reset(CAN_TX_FIFO_CONFIG *config);
int8_t mcp2518fd_rx_fifo_configure(CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_CONFIG *config);
int8_t mcp2518fd_rx_fifo_configure_reset(CAN_RX_FIFO_CONFIG *config);

void mcp2518fd_opmode_select(CAN_OPERATION_MODE opmode);
int8_t mcp2518fd_configure_bit_time_40MHz(CAN_NOMINAL_BITTIME_SETUP bit_time);

// TODO: Implement paraeters: CAN_FIFO_CHANNEL channel, CAN_RX_MSGOBJ* rxObj,
// uint8_t *rxd, uint8_t nBytes
int8_t mcp2518fd_receive_message();

#endif