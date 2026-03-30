/*
 * mcp2518fd_hw.h
 * This code is taken from Matthew Jeong's can.h in his spi-to-can transceiver.
 * I think it is a combination of the following files from the manufacturer headers:
 * drv_canfdspi_defines.h and drv_canfdspi_register.h
 * https://github.com/ProgrammerDummy/SPI-CAN-Driver
 * 
 * Registers and defines for mcp2518fd driver
*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/spi.h"

#define BUF_LEN 256
#define MSG_SIZE 8u
#define HIGH 1 
#define LOW 0

//READ_SFR and WRITE_SFR are the same values as CAN_READ and CAN_WRITE respectively, but follow different formats
//READ_SFR follows CMD + ADDR
//WRITE_SFR follows CMD + ADDR + data

/*
SPI data formatting for MCP2518fd

- first byte: 4 command bits (upper nibble of first byte) + 4 address bits (lower nibble of first byte)
- second byte: 8 address bits (full byte)
- data bytes: N data bytes however many you want to transmit

NOTE: WHEN ACCESSING RAM (DATA) it must be 4 byte aligned (use padding here)

*/


/*******************************************************************************
 * mcp2518fd_hw.h
 *
 * Hardware constants for the MCP2518FD CAN FD Controller.
 *
 * IMPORTANT: This file contains ONLY direct transcriptions from the
 * MCP2518FD datasheet (register addresses, SPI instruction codes,
 * register bit-field layouts, and hardware reset values).
 *
 * These are hardware FACTS, not engineering decisions.
 * Everything in this file comes directly from:
 *   Microchip MCP2518FD Datasheet DS20006027B
 *
 * What is NOT in this file (write these yourself):
 *   - SPI transfer functions
 *   - Read/write register functions
 *   - Init and configuration logic
 *   - TX/RX message handling
 *   - FIFO management
 *   - Bit timing calculations
 ******************************************************************************/

#ifndef MCP2518FD_HW_H
#define MCP2518FD_HW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================*/
/* SECTION 1: SPI INSTRUCTION SET                                            */
/* Source: MCP2518FD Datasheet, Section "SPI Instruction Set"                */
/* These are the command nibbles packed into the upper 4 bits of byte 0      */
/* of every SPI transaction.                                                 */
/*===========================================================================*/

#define MCP2518FD_INSTR_RESET       0x00  /* Reset internal registers to default state */
#define MCP2518FD_INSTR_READ        0x03  /* Read data from register at address         */
#define MCP2518FD_INSTR_WRITE       0x02  /* Write data to register at address          */
#define MCP2518FD_INSTR_READ_CRC    0x0B  /* Read data with CRC check                   */
#define MCP2518FD_INSTR_WRITE_CRC   0x0A  /* Write data with CRC check                  */
#define MCP2518FD_INSTR_WRITE_SAFE  0x0C  /* Write single byte with CRC (safe write)    */


/*===========================================================================*/
/* SECTION 2: REGISTER ADDRESSES                                             */
/* Source: MCP2518FD Datasheet, Section "Register Map"                       */
/* All addresses are 12-bit (0x000 - 0xFFF)                                 */
/*===========================================================================*/

/*--- CAN FD Controller Registers (0x000 - 0x04C) --------------------------*/
#define MCP2518FD_REG_CiCON         0x000  /* CAN Control Register                      */
#define MCP2518FD_REG_CiNBTCFG      0x004  /* Nominal Bit Time Configuration            */
#define MCP2518FD_REG_CiDBTCFG      0x008  /* Data Bit Time Configuration               */
#define MCP2518FD_REG_CiTDC         0x00C  /* Transmitter Delay Compensation            */

#define MCP2518FD_REG_CiTBC         0x010  /* Time Base Counter                         */
#define MCP2518FD_REG_CiTSCON       0x014  /* Time Stamp Configuration                  */
#define MCP2518FD_REG_CiVEC         0x018  /* Interrupt Vector                          */
#define MCP2518FD_REG_CiINT         0x01C  /* Interrupt Register (flags + enables)      */
#define MCP2518FD_REG_CiINTFLAG     0x01C  /* Interrupt Flag Register (lower 16 bits)   */
#define MCP2518FD_REG_CiINTENABLE   0x01E  /* Interrupt Enable Register (upper 16 bits) */

#define MCP2518FD_REG_CiRXIF        0x020  /* Receive Interrupt Pending                 */
#define MCP2518FD_REG_CiTXIF        0x024  /* Transmit Interrupt Pending                */
#define MCP2518FD_REG_CiRXOVIF      0x028  /* Receive Overflow Interrupt Pending        */
#define MCP2518FD_REG_CiTXATIF      0x02C  /* TX Attempts Interrupt Pending             */

#define MCP2518FD_REG_CiTXREQ       0x030  /* TX Request Register (trigger TX)          */
#define MCP2518FD_REG_CiTREC        0x034  /* TX/RX Error Count                         */
#define MCP2518FD_REG_CiBDIAG0      0x038  /* Bus Diagnostic Register 0                 */
#define MCP2518FD_REG_CiBDIAG1      0x03C  /* Bus Diagnostic Register 1                 */

#define MCP2518FD_REG_CiTEFCON      0x040  /* TX Event FIFO Control                     */
#define MCP2518FD_REG_CiTEFSTA      0x044  /* TX Event FIFO Status                      */
#define MCP2518FD_REG_CiTEFUA       0x048  /* TX Event FIFO User Address                */
#define MCP2518FD_REG_CiFIFOBA      0x04C  /* FIFO Base Address                         */

/*--- TX Queue Registers (FIFO CH0 is the TX Queue) ------------------------*/
#define MCP2518FD_REG_CiTXQCON      0x050  /* TX Queue Control                          */
#define MCP2518FD_REG_CiTXQSTA      0x054  /* TX Queue Status                           */
#define MCP2518FD_REG_CiTXQUA       0x058  /* TX Queue User Address                     */

/*--- FIFO Registers (CH1-CH31, each 3 registers × 4 bytes apart) ----------*/
/*
 * FIFO n register base address:
 *   MCP2518FD_REG_CiFIFOCON + (n * MCP2518FD_FIFO_REG_STRIDE)
 * where n = 1..31 (CH0 is the TX Queue above)
 */
#define MCP2518FD_REG_CiFIFOCON     0x050  /* FIFO Control Register base                */
#define MCP2518FD_REG_CiFIFOSTA     0x054  /* FIFO Status Register base                 */
#define MCP2518FD_REG_CiFIFOUA      0x058  /* FIFO User Address Register base           */
#define MCP2518FD_FIFO_REG_STRIDE   (3 * 4) /* 3 registers × 4 bytes = 12 bytes between FIFOs */

/*--- Filter Registers ------------------------------------------------------*/
/*
 * Filter CON registers start right after the last FIFO register.
 * There are 32 FIFOs (CH0-CH31), each 12 bytes:
 *   MCP2518FD_REG_CiFLTCON = MCP2518FD_REG_CiFIFOCON + (32 * 12) = 0x1D0
 */
#define MCP2518FD_REG_CiFLTCON      0x1D0  /* Filter Control Register (32 × 1 byte)     */
#define MCP2518FD_REG_CiFLTOBJ      0x1F0  /* Filter Object Register (ID to match)      */
#define MCP2518FD_REG_CiMASK        0x1F4  /* Filter Mask Register                      */
#define MCP2518FD_FILTER_REG_STRIDE (2 * 4) /* 2 registers × 4 bytes between filters   */

/*--- MCP2518FD Specific Registers (0xE00+) ---------------------------------*/
#define MCP2518FD_REG_OSC           0xE00  /* Oscillator Control                        */
#define MCP2518FD_REG_IOCON         0xE04  /* IO Control (GPIO pins)                    */
#define MCP2518FD_REG_CRC           0xE08  /* CRC Register                              */
#define MCP2518FD_REG_ECCCON        0xE0C  /* ECC Control                               */
#define MCP2518FD_REG_ECCSTA        0xE10  /* ECC Status                                */
#define MCP2518FD_REG_DEVID         0xE14  /* Device ID (read this first to verify SPI) */


/*===========================================================================*/
/* SECTION 3: RAM BOUNDARIES                                                 */
/* Source: MCP2518FD Datasheet, Section "RAM Memory Map"                     */
/* CAN message data (TX/RX payloads) is stored in this RAM region            */
/*===========================================================================*/

#define MCP2518FD_RAM_SIZE          2048          /* Total RAM in bytes                  */
#define MCP2518FD_RAM_START         0x400         /* First RAM address                   */
#define MCP2518FD_RAM_END           (MCP2518FD_RAM_START + MCP2518FD_RAM_SIZE)  /* 0xBFF + 1 */


/*===========================================================================*/
/* SECTION 4: REGISTER BIT-FIELD STRUCTURES (UNIONS)                        */
/* Source: MCP2518FD Datasheet, register description tables                  */
/* Each union lets you access a register three ways:                         */
/*   .bF.FieldName  - by individual named bit field                          */
/*   .word          - as a single 32-bit value (use for SPI read/write)      */
/*   .byte[n]       - as individual bytes                                    */
/*===========================================================================*/

/*--- General purpose 32-bit register (use when no specific type needed) ---*/
typedef union {
    uint8_t  byte[4];
    uint32_t word;
} REG_t;

/*--- CAN Control Register (CiCON) at 0x000 --------------------------------*/
typedef union {
    struct {
        uint32_t DNetFilterCount            : 5;  /* bits  4:0  - Device Net filter bit count  */
        uint32_t IsoCrcEnable               : 1;  /* bit   5    - ISO CRC enable               */
        uint32_t ProtocolExceptionEventDisable : 1; /* bit  6   - Protocol exception disable   */
        uint32_t unimplemented1             : 1;  /* bit   7                                   */
        uint32_t WakeUpFilterEnable         : 1;  /* bit   8    - Bus wake-up filter           */
        uint32_t WakeUpFilterTime           : 2;  /* bits 10:9  - Wake-up filter time select   */
        uint32_t unimplemented2             : 1;  /* bit  11                                   */
        uint32_t BitRateSwitchDisable       : 1;  /* bit  12    - Disable bit rate switch      */
        uint32_t unimplemented3             : 3;  /* bits 15:13                                */
        uint32_t RestrictReTxAttempts       : 1;  /* bit  16    - Restrict retransmit attempts */
        uint32_t EsiInGatewayMode           : 1;  /* bit  17    - ESI in gateway mode          */
        uint32_t SystemErrorToListenOnly    : 1;  /* bit  18    - Go to listen only on error   */
        uint32_t StoreInTEF                 : 1;  /* bit  19    - Store TX messages in TEF     */
        uint32_t TXQEnable                  : 1;  /* bit  20    - Enable TX Queue              */
        uint32_t OpMode                     : 3;  /* bits 23:21 - Current operation mode       */
        uint32_t RequestOpMode              : 3;  /* bits 26:24 - Request operation mode       */
        uint32_t AbortAllTx                 : 1;  /* bit  27    - Abort all pending TX         */
        uint32_t TxBandWidthSharing         : 4;  /* bits 31:28 - TX bandwidth sharing delay   */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiCON;

//the following is a custom enum for easy-to-understand modes to set opmode bit in CiCON register to change CAN modes

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


/*--- Nominal Bit Time Configuration (CiNBTCFG) at 0x004 -------------------*/
typedef union {
    struct {
        uint32_t SJW            : 7;  /* bits  6:0  - Synchronization Jump Width       */
        uint32_t unimplemented1 : 1;  /* bit   7                                        */
        uint32_t TSEG2          : 7;  /* bits 14:8  - Time Segment 2                    */
        uint32_t unimplemented2 : 1;  /* bit  15                                        */
        uint32_t TSEG1          : 8;  /* bits 23:16 - Time Segment 1                    */
        uint32_t BRP            : 8;  /* bits 31:24 - Baud Rate Prescaler               */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiNBTCFG;

/*--- Data Bit Time Configuration (CiDBTCFG) at 0x008 ----------------------*/
typedef union {
    struct {
        uint32_t SJW            : 4;  /* bits  3:0  - Synchronization Jump Width (FD)  */
        uint32_t unimplemented1 : 4;  /* bits  7:4                                      */
        uint32_t TSEG2          : 4;  /* bits 11:8  - Time Segment 2 (FD)              */
        uint32_t unimplemented2 : 4;  /* bits 15:12                                     */
        uint32_t TSEG1          : 5;  /* bits 20:16 - Time Segment 1 (FD)              */
        uint32_t unimplemented3 : 3;  /* bits 23:21                                     */
        uint32_t BRP            : 8;  /* bits 31:24 - Baud Rate Prescaler (FD)         */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiDBTCFG;

/*--- Transmitter Delay Compensation (CiTDC) at 0x00C ----------------------*/
typedef union {
    struct {
        uint32_t TDCValue       : 6;  /* bits  5:0  - TDC value                        */
        uint32_t unimplemented1 : 2;  /* bits  7:6                                      */
        uint32_t TDCOffset      : 7;  /* bits 14:8  - TDC offset                       */
        uint32_t unimplemented2 : 1;  /* bit  15                                        */
        uint32_t TDCMode        : 2;  /* bits 17:16 - TDC mode (off/manual/auto)       */
        uint32_t unimplemented3 : 6;  /* bits 23:18                                     */
        uint32_t SID11Enable    : 1;  /* bit  24    - Enable SID11 in FD mode          */
        uint32_t EdgeFilterEnable : 1;/* bit  25    - Edge filter during bus integration*/
        uint32_t unimplemented4 : 6;  /* bits 31:26                                     */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTDC;

/*--- Time Stamp Configuration (CiTSCON) at 0x014 --------------------------*/
typedef union {
    struct {
        uint32_t TBCPrescaler   : 10; /* bits  9:0  - Time base counter prescaler      */
        uint32_t unimplemented1 : 6;  /* bits 15:10                                     */
        uint32_t TBCEnable      : 1;  /* bit  16    - Time base counter enable         */
        uint32_t TimeStampEOF   : 1;  /* bit  17    - Timestamp at EOF (vs SOF)        */
        uint32_t unimplemented2 : 14; /* bits 31:18                                     */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTSCON;

/*--- Interrupt Vector Register (CiVEC) at 0x018 ---------------------------*/
typedef union {
    struct {
        uint32_t ICODE          : 7;  /* bits  6:0  - Interrupt code                   */
        uint32_t unimplemented1 : 1;  /* bit   7                                        */
        uint32_t FilterHit      : 5;  /* bits 12:8  - Filter that matched message      */
        uint32_t unimplemented2 : 3;  /* bits 15:13                                     */
        uint32_t TXCODE         : 7;  /* bits 22:16 - TX interrupt code                */
        uint32_t unimplemented3 : 1;  /* bit  23                                        */
        uint32_t RXCODE         : 7;  /* bits 30:24 - RX interrupt code                */
        uint32_t unimplemented4 : 1;  /* bit  31                                        */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiVEC;

/*--- Interrupt Flags (lower 16 bits of CiINT) -----------------------------*/
typedef struct {
    uint32_t TXIF       : 1;  /* bit  0 - TX interrupt pending                         */
    uint32_t RXIF       : 1;  /* bit  1 - RX interrupt pending                         */
    uint32_t TBCIF      : 1;  /* bit  2 - Time base counter overflow                   */
    uint32_t MODIF      : 1;  /* bit  3 - Operation mode change                        */
    uint32_t TEFIF      : 1;  /* bit  4 - TX event FIFO interrupt                      */
    uint32_t unimplemented1 : 3;
    uint32_t ECCIF      : 1;  /* bit  8 - ECC error                                    */
    uint32_t SPICRCIF   : 1;  /* bit  9 - SPI CRC error                                */
    uint32_t TXATIF     : 1;  /* bit 10 - TX attempt interrupt                         */
    uint32_t RXOVIF     : 1;  /* bit 11 - RX object overflow                           */
    uint32_t SERRIF     : 1;  /* bit 12 - System error                                 */
    uint32_t CERRIF     : 1;  /* bit 13 - CAN bus error                                */
    uint32_t WAKIF      : 1;  /* bit 14 - Bus wake-up                                  */
    uint32_t IVMIF      : 1;  /* bit 15 - Invalid message                              */
} CAN_INT_FLAGS;

/*--- Interrupt Enables (upper 16 bits of CiINT) ---------------------------*/
typedef struct {
    uint32_t TXIE       : 1;  /* bit  0 - Enable TX interrupt                          */
    uint32_t RXIE       : 1;  /* bit  1 - Enable RX interrupt                          */
    uint32_t TBCIE      : 1;  /* bit  2 - Enable time base counter overflow            */
    uint32_t MODIE      : 1;  /* bit  3 - Enable mode change interrupt                 */
    uint32_t TEFIE      : 1;  /* bit  4 - Enable TEF interrupt                         */
    uint32_t unimplemented2 : 3;
    uint32_t ECCIE      : 1;  /* bit  8 - Enable ECC error interrupt                   */
    uint32_t SPICRCIE   : 1;  /* bit  9 - Enable SPI CRC error interrupt               */
    uint32_t TXATIE     : 1;  /* bit 10 - Enable TX attempt interrupt                  */
    uint32_t RXOVIE     : 1;  /* bit 11 - Enable RX overflow interrupt                 */
    uint32_t SERRIE     : 1;  /* bit 12 - Enable system error interrupt                */
    uint32_t CERRIE     : 1;  /* bit 13 - Enable CAN bus error interrupt               */
    uint32_t WAKIE      : 1;  /* bit 14 - Enable wake-up interrupt                     */
    uint32_t IVMIE      : 1;  /* bit 15 - Enable invalid message interrupt             */
} CAN_INT_ENABLES;

/*--- Full Interrupt Register (CiINT) at 0x01C -----------------------------*/
typedef union {
    struct {
        CAN_INT_FLAGS   IF;   /* Lower 16 bits: interrupt flags   */
        CAN_INT_ENABLES IE;   /* Upper 16 bits: interrupt enables */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiINT;

/*--- TX/RX Error Count (CiTREC) at 0x034 ----------------------------------*/
typedef union {
    struct {
        uint32_t RxErrorCount           : 8;  /* bits  7:0  - Receive error count       */
        uint32_t TxErrorCount           : 8;  /* bits 15:8  - Transmit error count      */
        uint32_t ErrorStateWarning      : 1;  /* bit  16    - Error warning state        */
        uint32_t RxErrorStateWarning    : 1;  /* bit  17    - RX error warning           */
        uint32_t TxErrorStateWarning    : 1;  /* bit  18    - TX error warning           */
        uint32_t RxErrorStatePassive    : 1;  /* bit  19    - RX error passive state     */
        uint32_t TxErrorStatePassive    : 1;  /* bit  20    - TX error passive state     */
        uint32_t TxErrorStateBusOff     : 1;  /* bit  21    - TX bus-off state           */
        uint32_t unimplemented1         : 10;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTREC;

/*--- Bus Diagnostic Register 0 (CiBDIAG0) at 0x038 -----------------------*/
typedef union {
    struct {
        uint32_t NRxErrorCount  : 8;  /* bits  7:0  - Nominal RX error count            */
        uint32_t NTxErrorCount  : 8;  /* bits 15:8  - Nominal TX error count            */
        uint32_t DRxErrorCount  : 8;  /* bits 23:16 - Data RX error count               */
        uint32_t DTxErrorCount  : 8;  /* bits 31:24 - Data TX error count               */
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiBDIAG0;

/*--- Bus Diagnostic Register 1 (CiBDIAG1) at 0x03C -----------------------*/
typedef union {
    struct {
        uint32_t ErrorFreeMsgCount  : 16; /* bits 15:0  - Error-free message count      */
        uint32_t NBit0Error         : 1;  /* bit  16    - Nominal bit0 error            */
        uint32_t NBit1Error         : 1;  /* bit  17    - Nominal bit1 error            */
        uint32_t NAckError          : 1;  /* bit  18    - Nominal ACK error             */
        uint32_t NFormError         : 1;  /* bit  19    - Nominal form error            */
        uint32_t NStuffError        : 1;  /* bit  20    - Nominal stuff error           */
        uint32_t NCRCError          : 1;  /* bit  21    - Nominal CRC error             */
        uint32_t unimplemented1     : 1;
        uint32_t TXBOError          : 1;  /* bit  23    - TX bus-off error              */
        uint32_t DBit0Error         : 1;  /* bit  24    - Data bit0 error               */
        uint32_t DBit1Error         : 1;  /* bit  25    - Data bit1 error               */
        uint32_t DAckError          : 1;  /* bit  26    - Data ACK error                */
        uint32_t DFormError         : 1;  /* bit  27    - Data form error               */
        uint32_t DStuffError        : 1;  /* bit  28    - Data stuff error              */
        uint32_t DCRCError          : 1;  /* bit  29    - Data CRC error                */
        uint32_t ESI                : 1;  /* bit  30    - Error status indicator        */
        uint32_t unimplemented2     : 1;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiBDIAG1;

/*--- TX Event FIFO Control (CiTEFCON) at 0x040 ---------------------------*/
typedef union {
    struct {
        uint32_t TEFNEIE        : 1;  /* bit  0  - TEF not empty interrupt enable      */
        uint32_t TEFHFIE        : 1;  /* bit  1  - TEF half full interrupt enable      */
        uint32_t TEFFULIE       : 1;  /* bit  2  - TEF full interrupt enable           */
        uint32_t TEFOVIE        : 1;  /* bit  3  - TEF overflow interrupt enable       */
        uint32_t unimplemented1 : 1;
        uint32_t TimeStampEnable: 1;  /* bit  5  - Enable timestamps in TEF            */
        uint32_t unimplemented2 : 2;
        uint32_t UINC           : 1;  /* bit  8  - Increment head/tail pointer         */
        uint32_t unimplemented3 : 1;
        uint32_t FRESET         : 1;  /* bit  10 - Reset FIFO (clears all entries)     */
        uint32_t unimplemented4 : 13;
        uint32_t FifoSize       : 5;  /* bits 28:24 - Number of messages in TEF        */
        uint32_t unimplemented5 : 3;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTEFCON;

/*--- TX Event FIFO Status (CiTEFSTA) at 0x044 ----------------------------*/
typedef union {
    struct {
        uint32_t TEFNotEmptyIF  : 1;  /* bit  0  - TEF not empty flag                  */
        uint32_t TEFHalfFullIF  : 1;  /* bit  1  - TEF half full flag                  */
        uint32_t TEFFullIF      : 1;  /* bit  2  - TEF full flag                       */
        uint32_t TEFOVIF        : 1;  /* bit  3  - TEF overflow flag                   */
        uint32_t unimplemented1 : 28;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTEFSTA;

/*--- TX Queue Control (CiTXQCON) at 0x050 ---------------------------------*/
typedef union {
    struct {
        uint32_t TxNotFullIE    : 1;  /* bit  0  - TX queue not full interrupt enable  */
        uint32_t unimplemented1 : 1;
        uint32_t TxEmptyIE      : 1;  /* bit  2  - TX queue empty interrupt enable     */
        uint32_t unimplemented2 : 1;
        uint32_t TxAttemptIE    : 1;  /* bit  4  - TX attempt interrupt enable         */
        uint32_t unimplemented3 : 2;
        uint32_t TxEnable       : 1;  /* bit  7  - Enable TX queue                     */
        uint32_t UINC           : 1;  /* bit  8  - Increment TX queue tail             */
        uint32_t TxRequest      : 1;  /* bit  9  - Request transmission                */
        uint32_t FRESET         : 1;  /* bit 10  - Reset TX queue                      */
        uint32_t unimplemented4 : 5;
        uint32_t TxPriority     : 5;  /* bits 20:16 - TX priority (31=highest)         */
        uint32_t TxAttempts     : 2;  /* bits 22:21 - Retransmit attempts (0=disabled) */
        uint32_t unimplemented5 : 1;
        uint32_t FifoSize       : 5;  /* bits 28:24 - Number of messages in queue      */
        uint32_t PayLoadSize    : 3;  /* bits 31:29 - Payload size per message         */
    } txBF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTXQCON;

/*--- TX Queue Status (CiTXQSTA) at 0x054 ----------------------------------*/
typedef union {
    struct {
        uint32_t TxNotFullIF        : 1;  /* bit  0  - TX queue not full flag           */
        uint32_t unimplemented1     : 1;
        uint32_t TxEmptyIF          : 1;  /* bit  2  - TX queue empty flag              */
        uint32_t unimplemented2     : 1;
        uint32_t TxAttemptIF        : 1;  /* bit  4  - TX attempt exhausted flag        */
        uint32_t TxError            : 1;  /* bit  5  - TX error detected                */
        uint32_t TxLostArbitration  : 1;  /* bit  6  - Lost arbitration                 */
        uint32_t TxAborted          : 1;  /* bit  7  - TX aborted                       */
        uint32_t FifoIndex          : 5;  /* bits 12:8 - Index of next TX message       */
        uint32_t unimplemented3     : 19;
    } txBF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiTXQSTA;

/*--- FIFO Control Register (CiFIFOCON) at 0x050 + n*12 -------------------*/
typedef union {
    /* Use rxBF when FIFO is configured for receive (TxEnable=0) */
    struct {
        uint32_t RxNotEmptyIE       : 1;  /* bit  0  - Not empty interrupt enable       */
        uint32_t RxHalfFullIE       : 1;  /* bit  1  - Half full interrupt enable       */
        uint32_t RxFullIE           : 1;  /* bit  2  - Full interrupt enable            */
        uint32_t RxOverFlowIE       : 1;  /* bit  3  - Overflow interrupt enable        */
        uint32_t unimplemented1     : 1;
        uint32_t RxTimeStampEnable  : 1;  /* bit  5  - Enable RX timestamps             */
        uint32_t unimplemented2     : 1;
        uint32_t TxEnable           : 1;  /* bit  7  - 0=RX FIFO, 1=TX FIFO            */
        uint32_t UINC               : 1;  /* bit  8  - Increment head/tail              */
        uint32_t unimplemented3     : 1;
        uint32_t FRESET             : 1;  /* bit 10  - Reset FIFO                       */
        uint32_t unimplemented4     : 13;
        uint32_t FifoSize           : 5;  /* bits 28:24 - Number of messages in FIFO    */
        uint32_t PayLoadSize        : 3;  /* bits 31:29 - Payload size per message      */
    } rxBF;

    /* Use txBF when FIFO is configured for transmit (TxEnable=1) */
    struct {
        uint32_t TxNotFullIE    : 1;  /* bit  0  - Not full interrupt enable            */
        uint32_t TxHalfFullIE   : 1;  /* bit  1  - Half full interrupt enable           */
        uint32_t TxEmptyIE      : 1;  /* bit  2  - Empty interrupt enable               */
        uint32_t unimplemented1 : 1;
        uint32_t TxAttemptIE    : 1;  /* bit  4  - TX attempt interrupt enable          */
        uint32_t unimplemented2 : 1;
        uint32_t RTREnable      : 1;  /* bit  6  - Remote transmission request enable   */
        uint32_t TxEnable       : 1;  /* bit  7  - 1=TX FIFO                           */
        uint32_t UINC           : 1;  /* bit  8  - Increment tail                       */
        uint32_t TxRequest      : 1;  /* bit  9  - Request transmission                 */
        uint32_t FRESET         : 1;  /* bit 10  - Reset FIFO                           */
        uint32_t unimplemented3 : 5;
        uint32_t TxPriority     : 5;  /* bits 20:16 - TX priority                       */
        uint32_t TxAttempts     : 2;  /* bits 22:21 - Retransmit attempts               */
        uint32_t unimplemented4 : 1;
        uint32_t FifoSize       : 5;  /* bits 28:24 - Number of messages                */
        uint32_t PayLoadSize    : 3;  /* bits 31:29 - Payload size                      */
    } txBF;

    uint32_t word;
    uint8_t  byte[4];
} REG_CiFIFOCON;

/*--- FIFO Status Register (CiFIFOSTA) at 0x054 + n*12 --------------------*/
typedef union {
    struct {
        uint32_t RxNotEmptyIF   : 1;  /* bit  0  - FIFO not empty                      */
        uint32_t RxHalfFullIF   : 1;  /* bit  1  - FIFO half full                      */
        uint32_t RxFullIF       : 1;  /* bit  2  - FIFO full                           */
        uint32_t RxOverFlowIF   : 1;  /* bit  3  - FIFO overflow                       */
        uint32_t unimplemented1 : 4;
        uint32_t FifoIndex      : 5;  /* bits 12:8 - Index of next RX message          */
        uint32_t unimplemented2 : 19;
    } rxBF;
    struct {
        uint32_t TxNotFullIF        : 1;  /* bit  0  - FIFO not full                   */
        uint32_t TxHalfFullIF       : 1;  /* bit  1  - FIFO half full                  */
        uint32_t TxEmptyIF          : 1;  /* bit  2  - FIFO empty                      */
        uint32_t unimplemented1     : 1;
        uint32_t TxAttemptIF        : 1;  /* bit  4  - TX attempt exhausted            */
        uint32_t TxError            : 1;  /* bit  5  - TX error                        */
        uint32_t TxLostArbitration  : 1;  /* bit  6  - Lost arbitration                */
        uint32_t TxAborted          : 1;  /* bit  7  - TX aborted                      */
        uint32_t FifoIndex          : 5;  /* bits 12:8 - Index of next TX message      */
        uint32_t unimplemented2     : 19;
    } txBF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiFIFOSTA;

/*--- FIFO User Address (CiFIFOUA) at 0x058 + n*12 ------------------------*/
typedef union {
    struct {
        uint32_t UserAddress    : 12; /* bits 11:0 - Address of next message in RAM     */
        uint32_t unimplemented1 : 20;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CiFIFOUA;

/*--- Filter Control Register byte (one byte per filter) -------------------*/
typedef union {
    struct {
        uint8_t BufferPointer   : 5;  /* bits 4:0 - FIFO channel filter links to       */
        uint8_t unimplemented1  : 2;
        uint8_t Enable          : 1;  /* bit  7   - Enable this filter                 */
    } bF;
    uint8_t byte;
} REG_CiFLTCON_BYTE;

/*--- Oscillator Register (OSC) at 0xE00 -----------------------------------*/
typedef union {
    struct {
        uint32_t PllEnable          : 1;  /* bit  0  - Enable PLL (×10 from XTAL)      */
        uint32_t unimplemented1     : 1;
        uint32_t OscDisable         : 1;  /* bit  2  - Disable oscillator              */
        uint32_t LowPowerModeEnable : 1;  /* bit  3  - Low power mode (MCP2518FD only) */
        uint32_t SCLKDIV            : 1;  /* bit  4  - System clock divisor (1 or 2)   */
        uint32_t CLKODIV            : 2;  /* bits 6:5 - Clock output divisor           */
        uint32_t unimplemented2     : 1;
        uint32_t PllReady           : 1;  /* bit  8  - PLL locked (read only)          */
        uint32_t unimplemented3     : 1;
        uint32_t OscReady           : 1;  /* bit 10  - Oscillator running (read only)  */
        uint32_t unimplemented4     : 1;
        uint32_t SclkReady          : 1;  /* bit 12  - System clock stable (read only) */
        uint32_t unimplemented5     : 19;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_OSC;


/*--- IO Control Register (IOCON) at 0xE04 ---------------------------------*/
typedef union {
    struct {
        uint32_t TRIS0              : 1;  /* bit  0  - GPIO0 direction (0=out, 1=in)   */
        uint32_t TRIS1              : 1;  /* bit  1  - GPIO1 direction                 */
        uint32_t unimplemented1     : 2;
        uint32_t ClearAutoSleepOnMatch : 1; /* bit 4 - Clear auto-sleep on filter match */
        uint32_t AutoSleepEnable    : 1;  /* bit  5  - Auto-sleep enable               */
        uint32_t XcrSTBYEnable      : 1;  /* bit  6  - XSTBY pin control               */
        uint32_t unimplemented2     : 1;
        uint32_t LAT0               : 1;  /* bit  8  - GPIO0 latch (output value)      */
        uint32_t LAT1               : 1;  /* bit  9  - GPIO1 latch                     */
        uint32_t unimplemented3     : 5;
        uint32_t HVDETSEL           : 1;  /* bit 15  - High voltage detect select      */
        uint32_t GPIO0              : 1;  /* bit 16  - GPIO0 input state (read only)   */
        uint32_t GPIO1              : 1;  /* bit 17  - GPIO1 input state               */
        uint32_t unimplemented4     : 6;
        uint32_t PinMode0           : 1;  /* bit 24  - INT0/GPIO0 pin mode             */
        uint32_t PinMode1           : 1;  /* bit 25  - INT1/GPIO1 pin mode             */
        uint32_t unimplemented5     : 2;
        uint32_t TXCANOpenDrain     : 1;  /* bit 28  - TXCAN open drain mode           */
        uint32_t SOFOutputEnable    : 1;  /* bit 29  - SOF signal output enable        */
        uint32_t INTPinOpenDrain    : 1;  /* bit 30  - INT pins open drain mode        */
        uint32_t unimplemented6     : 1;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_IOCON;

/*--- CRC Register (CRC) at 0xE08 -----------------------------------------*/
typedef union {
    struct {
        uint32_t CRC            : 16; /* bits 15:0  - Saved CRC from last SPI transfer  */
        uint32_t CRCERRIF       : 1;  /* bit  16    - CRC mismatch flag                 */
        uint32_t FERRIF         : 1;  /* bit  17    - Format error flag                 */
        uint32_t unimplemented1 : 6;
        uint32_t CRCERRIE       : 1;  /* bit  24    - Enable CRC error interrupt        */
        uint32_t FERRIE         : 1;  /* bit  25    - Enable format error interrupt     */
        uint32_t unimplemented2 : 6;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_CRC;

/*--- ECC Control Register (ECCCON) at 0xE0C --------------------------------*/
typedef union {
    struct {
        uint32_t EccEn          : 1;  /* bit  0  - Enable ECC for RAM                  */
        uint32_t SECIE          : 1;  /* bit  1  - Single error correction interrupt    */
        uint32_t DEDIE          : 1;  /* bit  2  - Double error detection interrupt     */
        uint32_t unimplemented1 : 5;
        uint32_t Parity         : 7;  /* bits 14:8 - Parity bits for diagnostics        */
        uint32_t unimplemented2 : 17;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_ECCCON;

/*--- ECC Status Register (ECCSTA) at 0xE10 --------------------------------*/
typedef union {
    struct {
        uint32_t unimplemented1 : 1;
        uint32_t SECIF          : 1;  /* bit  1  - Single error corrected flag          */
        uint32_t DEDIF          : 1;  /* bit  2  - Double error detected flag           */
        uint32_t unimplemented2 : 13;
        uint32_t ErrorAddress   : 12; /* bits 27:16 - Address of error in RAM           */
        uint32_t unimplemented3 : 4;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_ECCSTA;

/*--- Device ID Register (DEVID) at 0xE14 ----------------------------------*/
/*  Read this first after reset to verify SPI communication is working      */
/*  Expected: DEV=0x1 (MCP2518FD), REV=chip revision                       */
typedef union {
    struct {
        uint32_t REV            : 4;  /* bits 3:0 - Silicon revision number             */
        uint32_t DEV            : 4;  /* bits 7:4 - Device ID (0x01 = MCP2518FD)        */
        uint32_t unimplemented  : 24;
    } bF;
    uint32_t word;
    uint8_t  byte[4];
} REG_DEVID;

/* Start of structs that Marco added from mcp25XXfd application code */
typedef struct CAN_MSGOBJ_ID {
    uint32_t SID : 11;
    uint32_t EIF : 18;
    uint32_t SID11: 1;
    uint32_t unimplemented1 : 2;
} CAN_MSGOBJ_ID;

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

/*===========================================================================*/
/* SECTION 5: REGISTER RESET VALUES                                         */
/* Source: MCP2518FD Datasheet, Table "Register Reset Values"               */
/* Use these to verify your init left registers in expected states           */
/* Index 0 = address 0x000, index 1 = 0x004, etc. (4 bytes per register)   */
/*===========================================================================*/

/* CAN controller registers 0x000 - 0x04C */
static const uint32_t mcp2518fd_ctrl_reset_vals[] = {
    /* 0x000 CiCON    */ 0x04980760,
    /* 0x004 CiNBTCFG */ 0x003E0F0F,
    /* 0x008 CiDBTCFG */ 0x000E0303,
    /* 0x00C CiTDC    */ 0x00021000,
    /* 0x010 CiTBC    */ 0x00000000,
    /* 0x014 CiTSCON  */ 0x00000000,
    /* 0x018 CiVEC    */ 0x40400040,
    /* 0x01C CiINT    */ 0x00000000,
    /* 0x020 CiRXIF   */ 0x00000000,
    /* 0x024 CiTXIF   */ 0x00000000,
    /* 0x028 CiRXOVIF */ 0x00000000,
    /* 0x02C CiTXATIF */ 0x00000000,
    /* 0x030 CiTXREQ  */ 0x00000000,
    /* 0x034 CiTREC   */ 0x00200000,
    /* 0x038 CiBDIAG0 */ 0x00000000,
    /* 0x03C CiBDIAG1 */ 0x00000000,
    /* 0x040 CiTEFCON */ 0x00000400,
    /* 0x044 CiTEFSTA */ 0x00000000,
    /* 0x048 CiTEFUA  */ 0x00000000,
    /* 0x04C CiFIFOBA */ 0x00000000
};

/* Per-FIFO reset values (CON, STA, UA) */
static const uint32_t mcp2518fd_fifo_reset_vals[] = {
    /* CON */ 0x00600400,
    /* STA */ 0x00000000,
    /* UA  */ 0x00000000
};

/* MCP2518FD specific registers 0xE00 - 0xE10 */
static const uint32_t mcp2518fd_specific_reset_vals[] = {
    /* 0xE00 OSC    */ 0x00000460,
    /* 0xE04 IOCON  */ 0x00000003,
    /* 0xE08 CRC    */ 0x00000000,
    /* 0xE0C ECCCON */ 0x00000000,
    /* 0xE10 ECCSTA */ 0x00000000
};



#ifdef __cplusplus
}
#endif

#endif /* MCP2518FD_HW_H */