#include "mcp2518fd.h"
#include "mcp2518fd_hw.h"
#include <stdio.h>

/*
  Frequency of SCK must be less than or equal to 
  0.85 * half the frequency of SYSCLK. 
  (MCP2518FD Datasheet, 69)

*/
#define SPI_CLK_RATE 12500000
/*
  Application code for sending data continuously
*/

#define LED_PIN 25

void blink_builtin_init();
void blink_builtin(int delay_ms);
// int test_read_write();
// int mcp2518fd_tx_fifo_test();
int mcp2518fd_rx_init_test();
int mcp2518fd_rx_fifo_test(uint8_t *data);

int main() {
    stdio_init_all();
    blink_builtin_init(); 

    mcp2518fd_init(SPI_CLK_RATE);

    mcp2518fd_rx_init_test();

    printf("\n-----STARTING TEST-----\n");
    uint8_t databuf[8];
    for (;;) {
      mcp2518fd_rx_fifo_test(databuf);
      printf("Out (Decimal): ");
      for (int i = 0; i < 8; i++) {
        printf("%d ", databuf[i]);
      }
      printf("\n");
      blink_builtin(500);
    }
}

void blink_builtin_init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void blink_builtin(int delay_ms) {
    gpio_put(LED_PIN, 1);
    sleep_ms(delay_ms);
    gpio_put(LED_PIN, 0);
    sleep_ms(delay_ms);
}

// archived test of checking transmission
int test_tx() {
    blink_builtin_init(); 

    mcp2518fd_init(SPI_CLK_RATE);

    int tx_status;

    for (;;) {
      tx_status = mcp2518fd_tx_fifo_test();
      blink_builtin(500);
    }
}


// archived test of checking of read/write works
int test_read_write() {
    // read ciCon data from mcp2518fd
    uint32_t data = 0;
    mcp2518fd_read_word(MCP2518FD_REG_CiCON, &data);
    if (data != mcp2518fd_ctrl_reset_vals[MCP2518FD_REG_CiCON/4]) {
      return -1;
    }

    // select opmode
    mcp2518fd_opmode_select(CAN_NORMAL_MODE);

    // read ciCon data from mcp2518fd
    uint32_t modified_data = 0;;
    mcp2518fd_read_word(MCP2518FD_REG_CiCON, &modified_data);

    for (;;) {
      blink_builtin(500);
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