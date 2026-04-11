#include "mcp2518fd.h"
#include "mcp2518fd_hw.h"

// reverse order of bits in byte
const uint8_t BitReverseTable256[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

// look up table for crc calculations in the future
const uint16_t crc16_table[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

// initialize mcp2518fd with hardcoded settings. Namely, 500kbps with no crc.
int8_t mcp2518fd_init(uint32_t spi_clk_rate) {
    /*
      I won't be doing error checking for the writes and reads because the internal
      spi_write_read_blocking function never returns anything other than the number
      of bytes transmitted/received. If ou want to see this, look inside pico SDK's 
      spi_write_read_blocking function and it only returns "len", no matter what. 
    */
    mcp2518fd_spi_init(spi_clk_rate);

    mcp2518fd_reset();

    // oscillator configuration, CLKO divisor 1, SCLK divisor 1, PLL disabled
    // SOLDERED CAN breakout board has 40 MHz internal oscillator, so no divisors or PLL needed
    REG_OSC osc;
    osc.word = mcp2518fd_specific_reset_vals[0]; 
    osc.bF.CLKODIV = 0b00;
    osc.bF.SCLKDIV = 0b0;
    osc.bF.PllEnable = 0b0;
    mcp2518fd_write_word(MCP2518FD_REG_OSC, osc.word);

    // check for osc_ready bit to be set (no timeout because I haven't implemented timers)
    uint8_t osc_data = 0;
    while (1) {
        mcp2518fd_read_byte(MCP2518FD_REG_OSC + 1, &osc_data);
        if (osc_data & 0x04) {
            break;
        }
    }

    // I/O configuration, GPIO0 and GPIO1 to be both inputs 
    // also, bit fields in the IOCON register must be written using single data byte SFR WRITE instructions
    REG_IOCON iocon;
    iocon.word = mcp2518fd_specific_reset_vals[1];
    mcp2518fd_write_byte(MCP2518FD_REG_IOCON, *iocon.byte); 

    // CAN configuration, disable crc, disable TXQ, disable TEF
    REG_CiCON ciCon;
    ciCon.word = mcp2518fd_ctrl_reset_vals[MCP2518FD_REG_CiCON / 4];
    ciCon.bF.IsoCrcEnable = 0; 
    ciCon.bF.StoreInTEF = 0; 
    ciCon.bF.TXQEnable = 0;
    mcp2518fd_write_word(MCP2518FD_REG_CiCON, ciCon.word);

    // matthew nominal bit timing config 
    // I think this is 500Kbps, 80% sample point
    REG_CiNBTCFG ciNbtcfg;
    ciNbtcfg.word = mcp2518fd_ctrl_reset_vals[MCP2518FD_REG_CiNBTCFG / 4];
    ciNbtcfg.bF.SJW = 15;
    ciNbtcfg.bF.TSEG2 = 15;
    ciNbtcfg.bF.TSEG1 = 62;
    ciNbtcfg.bF.BRP = 0; // baudrate prescaler of 1
    mcp2518fd_write_word(MCP2518FD_REG_CiNBTCFG, ciNbtcfg.word);

    // Fifo 1: transmit fifo; 5 messages, 8 byte max payload, high priority
    REG_CiFIFOCON ciFifocon1;
    ciFifocon1.word = mcp2518fd_fifo_reset_vals[0];
    ciFifocon1.txBF.TxEnable = 1;
    ciFifocon1.txBF.FifoSize = 4;
    ciFifocon1.txBF.PayLoadSize = 0b000;
    ciFifocon1.txBF.TxPriority = 1;
    mcp2518fd_write_word(MCP2518FD_REG_CiFIFOCON + (1 * MCP2518FD_FIFO_REG_STRIDE), ciFifocon1.word);
    
    // Fifo 2: receive fifo; 16 messages, 8 byte max payload, time stamping disabled
    REG_CiFIFOCON ciFifocon2;
    ciFifocon2.word = mcp2518fd_fifo_reset_vals[0];
    ciFifocon2.rxBF.TxEnable = 0;
    ciFifocon2.rxBF.FifoSize = 15;
    ciFifocon2.rxBF.PayLoadSize = 0b000;
    ciFifocon2.rxBF.RxTimeStampEnable = 0;
    mcp2518fd_write_word(MCP2518FD_REG_CiFIFOCON + (2 * MCP2518FD_FIFO_REG_STRIDE), ciFifocon2.word);


    // Initialize RAM
    mcp2518fd_ram_init(0x00);

    // Select Normal Mode
    mcp2518fd_opmode_select(CAN_NORMAL_MODE);
    // mcp2518fd_opmode_select(CAN_NORMAL_MODE);

    return 0;
}

static void mcp2518fd_spi_init(uint32_t spi_clk_rate) {
    // Initialize RP2350 SPI peripheral
    stdio_init_all();

    // Enable SPI 0 at specified clock rate and connect to GPIOs
    uint32_t real_baudrate = spi_init(spi_default, spi_clk_rate);

    // Set SPI to (0,0) mode
    spi_set_format(spi_default, MSG_SIZE, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI); // SPI0RX GP16
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI); // SPI0TX GP19
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI); // SPI0SCK GP18

    // gpio_pull_up(PICO_DEFAULT_SPI_RX_PIN);
    // gpio_pull_up(PICO_DEFAULT_SPI_TX_PIN);

    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SIO); // SPICSN GP17
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, HIGH);
}

/*
  Perform mcp2518fd RESET instruction
*/
void mcp2518fd_reset() {
    uint16_t addr = 0x0000;
    uint8_t txbuffer[6] = {0};
    uint8_t rxbuffer[6] = {0};

    txbuffer[0] = (MCP2518FD_INSTR_RESET << 4) | ((addr >> 8) & 0x0F);
    txbuffer[1] = (addr >> 0) & 0xFF;

    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, LOW);
    uint8_t error = spi_write_read_blocking(spi_default, txbuffer, rxbuffer, 2);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, HIGH);
}

/* 
  Use SPI to read a byte from a specified address of the MCP2518FD
  Returns number of bytes written/read via SPI. Should always be 3 though.
*/
uint8_t mcp2518fd_read_byte(uint16_t addr, uint8_t *data) {
    uint8_t txbuffer[6] = {0};
    uint8_t rxbuffer[6] = {0};

    txbuffer[0] = (MCP2518FD_INSTR_READ << 4) | ((addr >> 8) & 0x0F);
    txbuffer[1] = (addr >> 0) & 0xFF;

    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, LOW);
    uint8_t error = spi_write_read_blocking(spi_default, txbuffer, rxbuffer, 3);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, HIGH);

    *data = (rxbuffer[2] << 0);
    return error;
}

/* 
  Use SPI to read a word from a specified address of the MCP2518FD
  Returns number of bytes written/read via SPI. Should always be 6 though.
*/
uint8_t mcp2518fd_read_word(uint16_t addr, uint32_t *data) {
    uint8_t txbuffer[6] = {0};
    uint8_t rxbuffer[6] = {0};

    txbuffer[0] = (MCP2518FD_INSTR_READ << 4) | ((addr >> 8) & 0x0F);
    txbuffer[1] = (addr >> 0) & 0xFF;

    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, LOW);
    uint8_t error = spi_write_read_blocking(spi_default, txbuffer, rxbuffer, 6);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, HIGH);

    *data = (rxbuffer[2] << 0) | (rxbuffer[3] << 8) | (rxbuffer[4] << 16) | (rxbuffer[5] << 24);
    return error;
}

/* 
  Use SPI to write a byte to a specified address of the MCP2518FD
  This can be used in general to write to RAM or any SFR without CRC
  Returns number of bytes written/read via SPI. Should always be 3 though.
*/
uint8_t mcp2518fd_write_byte(uint16_t addr, uint8_t data) {
    uint8_t txbuffer[6] = {0};
    uint8_t rxbuffer[6] = {0};

    txbuffer[0] = (MCP2518FD_INSTR_WRITE << 4) | ((addr >> 8) & 0x0F);
    txbuffer[1] = (addr >> 0) & 0xFF;
    txbuffer[2] = (data >> 0) & 0xFF;

    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, LOW);
    uint8_t error = spi_write_read_blocking(spi_default, txbuffer, rxbuffer, 3);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, HIGH);
    return error;
}

/* 
  Use SPI to write a 32-bit word a specified address of the MCP2518FD
  This can be used in general to write to RAM or any SFR without CRC
  Returns number of bytes written/read via SPI. Should always be 6 though.
*/
uint8_t mcp2518fd_write_word(uint16_t addr, uint32_t data) {
    uint8_t txbuffer[6] = {0};
    uint8_t rxbuffer[6] = {0};

    txbuffer[0] = (MCP2518FD_INSTR_WRITE << 4) | ((addr >> 8) & 0x0F);
    txbuffer[1] = (addr >> 0) & 0xFF;
    txbuffer[2] = (data >> 0) & 0xFF;
    txbuffer[3] = (data >> 8) & 0xFF;
    txbuffer[4] = (data >> 16) & 0xFF;
    txbuffer[5] = (data >> 24) & 0xFF;
    
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, LOW);
    uint8_t error = spi_write_read_blocking(spi_default, txbuffer, rxbuffer, 6);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, HIGH);
    return error;
}

// fill up RAM with data
void mcp2518fd_ram_init(uint8_t data) {
    for (uint32_t addr = MCP2518FD_RAM_START; addr < MCP2518FD_RAM_END; addr++) {
      mcp2518fd_write_byte(addr, data);
    }
}

void mcp2518fd_opmode_select(CAN_OPERATION_MODE opmode) {
    uint8_t ciCon_data = 0;
    mcp2518fd_read_byte(MCP2518FD_REG_CiCON + 3, &ciCon_data);
    ciCon_data &= ~0x07;
    ciCon_data |= opmode;
    mcp2518fd_write_byte(MCP2518FD_REG_CiCON + 3, ciCon_data);

    // wait for opmod to match the requested operation mode before continuing
    ciCon_data = 0;

    while (1) {
        mcp2518fd_read_byte(MCP2518FD_REG_CiCON + 2, &ciCon_data);
        if ((ciCon_data >> 5 & 0x07) == opmode) {
            break;
        }
    }
}   

// this function is just for testing creating a tx object and sending it.
// as a result, it has magic numbers and terrible programming semantics 
// it also doesn't have any defensive programming whatsoever and assumes
// that fifo1 is a tx fifo with 8 byte payload
// DO NOT USE!!!
// return value: error code
int mcp2518fd_tx_fifo_test() {
    // first section: load message into transmit fifo

    // initialize ID and control bits
    CAN_TX_MSGOBJ tx_obj;
    uint8_t tx_data[8];
    // size of data payload in bytes
    uint8_t data_payload_size = 8;

    tx_obj.word[0] = 0;
    tx_obj.word[1] = 0;
    
    tx_obj.bF.id.SID = 0x200; // 0x200 is arbitrary, just a random ID number I chose
    tx_obj.bF.id.EID = 0;

    // data BRS not faster
    tx_obj.bF.ctrl.BRS = 0;
    // 8 byte payload (
    tx_obj.bF.ctrl.DLC = 0b1000;
    // not can FD frame
    tx_obj.bF.ctrl.FDF = 0;
    // base format, not extended format
    tx_obj.bF.ctrl.IDE = 0;

    // fill data with counting numbers (just for testing)
    for (int i = 0; i < data_payload_size; i++) {
        tx_data[i] = i;
    }

    uint8_t fifo1_status_data;
    mcp2518fd_read_byte(MCP2518FD_REG_CiFIFOSTA + 
        (1 * MCP2518FD_FIFO_REG_STRIDE), &fifo1_status_data);
    
    // check if fifo is not full by seeing if CiFifosta1.TFNRFNIF is set. Exit early if it is full.
    if (!(fifo1_status_data & 0x01)) {
        return -1;
    }

    // get address in RAM of next TX object from CiFIFOUA1
    REG_CiFIFOUA ciFifoua1;
    mcp2518fd_read_word(MCP2518FD_REG_CiFIFOUA + 
        (1 * MCP2518FD_FIFO_REG_STRIDE), &ciFifoua1.word);
    uint32_t addr = MCP2518FD_RAM_START + ciFifoua1.bF.UserAddress;

    REG_CiFIFOCON ciFifoCon1;
    uint8_t txbuffer[8+8]; // 8 bytes of tx_obj config + 8 bytes of data
    for (int i = 0; i < data_payload_size; i++) {
        txbuffer[i] = tx_obj.byte[i];
        txbuffer[i+8] = tx_data[i];
    }

    // write tx_data to RAM in multiples of 4 bytes
    uint32_t ram_tx_data = 0;
    for (int i = 0; i < 16; i += 4) {
        ram_tx_data = txbuffer[i] | (txbuffer[i+1] << 8) 
        | (txbuffer[i+2] << 16) | (txbuffer[i+3] << 24);
        mcp2518fd_write_word(addr, ram_tx_data);
        addr += 4;
    }

    // second section: request transmission of message in transmit fifo
    // store address of CiFIFOCON1
    uint8_t fifo1_config_data;
    addr = MCP2518FD_REG_CiFIFOCON + (1 * MCP2518FD_FIFO_REG_STRIDE);
    mcp2518fd_read_byte(addr + 1, &fifo1_config_data);

    // check if fifo is still resetting by checking FRESET bit.
    // if FRESET is set, then return early. Normally, we should
    // wait until FRESET is clear before taking any action. So this
    // guard clause NOT sufficient in the final driver.
    if (fifo1_config_data & (1 << 2)) {
        return -2;
    }

    // set UINC and TXREQ
    mcp2518fd_write_byte(addr + 1, 0b11);

    
    // wait until TX is cleared before continuing
    while (1) {
        mcp2518fd_read_byte(addr + 1, &fifo1_config_data);
        if (!(fifo1_config_data & (1 << 1))) {
            break;
        }
    }
    return 0;
}

// this function is expected to only be called before mcp2518fd_rx_fifo_test
// this function sets up filter object 0 and mask object 0 with hardcoded values
// It captures frames with SID from 0x200 - 0x20F and points the filter to FIFO2
// return value: error code
int mcp2518fd_rx_init_test() {
    // first section: configure filter0 and mask0 to match standard
    // frames with SID 0x200 - 0x20F
    uint16_t filter_num = 0;
    uint16_t fifo_channel_num = 2; // fifo channel number to receive the data

    // clear FLTEN bit before changing filter or mask object
    REG_CiFLTCON_BYTE ciFltcon0;
    uint32_t addr = MCP2518FD_REG_CiFLTCON + filter_num;
    mcp2518fd_read_byte(addr, &ciFltcon0.byte);
    ciFltcon0.bF.Enable = 0; // disable fifo
    mcp2518fd_write_byte(addr, ciFltcon0.byte);

    // configure filter object 0
    REG_CiFLTOBJ fObj0;
    fObj0.word = 0;
    fObj0.bF.SID = 0x200;
    fObj0.bF.SID11 = 0;
    fObj0.bF.EID = 0;
    fObj0.bF.EXIDE = 0; // only match messages with SID
    addr = MCP2518FD_REG_CiFLTOBJ + (filter_num * MCP2518FD_FILTER_REG_STRIDE);
    mcp2518fd_write_word(addr, fObj0.word);

    REG_CiMASK mObj0;
    mObj0.word = 0;
    mObj0.bF.MSID = 0x7F0; // make mask 4 bits wide from the LSB
    mObj0.bF.MSID11 = 0;
    mObj0.bF.MEID = 0;
    mObj0.bF.MIDE = 1; // match EXIDE bit 
    addr = MCP2518FD_REG_CiMASK + (filter_num * MCP2518FD_FILTER_REG_STRIDE);
    mcp2518fd_write_word(addr, mObj0.word);

    ciFltcon0.bF.BufferPointer = fifo_channel_num;
    ciFltcon0.bF.Enable = 1; // enable fifo
    addr = MCP2518FD_REG_CiFLTCON + filter_num;
    mcp2518fd_write_byte(addr, ciFltcon0.byte);

    return 0;
}

// this function tests RX based on settings from mcp2518fd_rx_init_test
int mcp2518fd_rx_fifo_test(uint8_t *data) {
    uint16_t fifo_channel_num = 2; // fifo channel number to receive the data
    
    // second section: receive the message and store it in data
    uint8_t fifo_status_data;
    uint32_t addr = MCP2518FD_REG_CiFIFOSTA + (fifo_channel_num * MCP2518FD_FIFO_REG_STRIDE);
    
    // block until fifo contains atleast one message (terrible practice)
    while (1) {
        mcp2518fd_read_byte(addr, &fifo_status_data);
        // check if TFNRFNIF is set (if its set, then fifo not empty)
        if (fifo_status_data & 0x01) {
            break;
        }
    }

    // get address in RAM of next TX object from CiFIFOUA1
    REG_CiFIFOUA ciFifoua2;
    addr = MCP2518FD_REG_CiFIFOUA + (fifo_channel_num * MCP2518FD_FIFO_REG_STRIDE);
    mcp2518fd_read_word(addr, &ciFifoua2.word);

    addr = MCP2518FD_RAM_START + ciFifoua2.bF.UserAddress;
    
    // read one rx obj data from RAM in multiples of 4 bytes
    uint32_t rx_obj_data[4]; 
    for (int i = 0; i < 4; i++) {
        mcp2518fd_read_word(addr + (i * 4), &rx_obj_data[i]);
    }

    data[0] = (rx_obj_data[2] >> 0) & 0xFF;
    data[1] = (rx_obj_data[2] >> 8) & 0xFF;
    data[2] = (rx_obj_data[2] >> 16) & 0xFF;
    data[3] = (rx_obj_data[2] >> 24) & 0xFF;
    data[4] = (rx_obj_data[3] >> 0) & 0xFF;
    data[5] = (rx_obj_data[3] >> 8) & 0xFF;
    data[6] = (rx_obj_data[3] >> 16) & 0xFF;
    data[7] = (rx_obj_data[3] >> 24) & 0xFF;

    // set UINC
    addr = MCP2518FD_REG_CiFIFOCON + (fifo_channel_num * MCP2518FD_FIFO_REG_STRIDE);
    mcp2518fd_write_byte(addr + 1, 0b01);

    return 0;
}