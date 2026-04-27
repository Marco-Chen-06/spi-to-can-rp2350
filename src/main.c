#include "mcp2518fd.h"
#include "mcp2518fd_hw.h"
#include <stdio.h>

/*
  Frequency of SCK must be less than or equal to 
  0.85 * half the frequency of SYSCLK. 
  (MCP2518FD Datasheet, 69)

*/
#define SPI_CLK_RATE 12500000

#define LED_PIN 25

void blink_builtin_init();
void blink_builtin(int delay_ms);
int8_t mcp2518fd_init(uint32_t spi_clk_rate);
int8_t mcp2518fd_rx_init();

int main()
{
	// The test below continuously sends [0, 1, 2, 3, 4, 5, 6, 7] to 0x200
	// and continiously receives message with IDs 0x200 - 0x20F
	stdio_init_all();
	blink_builtin_init();

	mcp2518fd_init(SPI_CLK_RATE);
	mcp2518fd_rx_init();

	printf("\n-----STARTING TEST-----\n");
	uint8_t rx_data[8];
	uint8_t tx_data[8];
	CAN_RX_MSGOBJ rxObj;
	CAN_TX_MSGOBJ tx_obj;

	tx_obj.word[0] = 0;
	tx_obj.word[1] = 0;
	tx_obj.bF.id.SID = 0x200; // 0x200 is arbitrary, just a random ID number I chose
	tx_obj.bF.id.EID = 0;
	// data BRS not faster
	tx_obj.bF.ctrl.BRS = 0;
	// 8 byte payload
	tx_obj.bF.ctrl.DLC = 0b1000;
	// not can FD frame
	tx_obj.bF.ctrl.FDF = 0;
	// base format, not extended format
	tx_obj.bF.ctrl.IDE = 0;
	for (int i = 0; i < 8; i++) {
		tx_data[i] = i;
	}

	for (;;) {
		// receive a message, and retry if there is an error
		if (mcp2518fd_receive_message(CAN_FIFO_CH2, &rxObj, rx_data) == 0) {
			printf("Out (Decimal): ");
			for (int i = 0; i < 8; i++) {
				printf("%d ", rx_data[i]);
			}
			printf("\n");
		}

		// continuously send messages
		mcp2518fd_load_message(CAN_FIFO_CH1, &tx_obj, tx_data);
		mcp2518fd_send_message(CAN_FIFO_CH1);

		// do a blocking blink every half second
		blink_builtin(500);
	}
}

void blink_builtin_init()
{
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
}

void blink_builtin(int delay_ms)
{
	gpio_put(LED_PIN, 1);
	sleep_ms(delay_ms);
	gpio_put(LED_PIN, 0);
	sleep_ms(delay_ms);
}

/* 
	Basic initialization of CAN communication
	Notable features:
	FIFO1 is set as TX FIFO
	FIFO2 is set as RX FIFO
*/
int8_t mcp2518fd_init(uint32_t spi_clk_rate)
{
	// Initialize RP2350 SPI peripheral
	mcp2518fd_spi_init(spi_clk_rate);

	mcp2518fd_reset();

	// oscillator configuration, CLKO divisor 1, SCLK divisor 1, PLL disabled
	// SOLDERED CAN breakout board has 40 MHz internal oscillator, so no divisors or PLL needed
	CAN_OSC_CTRL osc;
	mcp2518fd_osc_configure_reset(&osc);
	osc.CLKODIV = 0b00;
	osc.SCLKDIV = 0b0;
	osc.PllEnable = 0b0;
	mcp2518fd_osc_configure(&osc);

	// I/O configuration, GPIO0 and GPIO1 to be both inputs (by using default config)
	CAN_IO_CTRL iocon;
	mcp2518fd_io_configure_reset(&iocon);
	mcp2518fd_io_configure(&iocon);

	// CAN configuration, disable crc, disable TXQ, disable TEF
	CAN_CONFIG ciCon;
	mcp2518fd_configure_reset(&ciCon);
	ciCon.IsoCrcEnable = 0;
	ciCon.StoreInTEF = 0;
	ciCon.TXQEnable = 0;
	mcp2518fd_configure(&ciCon);

	mcp2518fd_configure_bit_time_40MHz(CAN_NBT_500K);

	CAN_TX_FIFO_CONFIG tx_config;
	mcp2518fd_tx_fifo_configure_reset(&tx_config);
	tx_config.FifoSize = 4;
	tx_config.PayLoadSize = 0b000;
	tx_config.TxPriority = 1;
	mcp2518fd_tx_fifo_configure(CAN_FIFO_CH1, &tx_config);

	CAN_RX_FIFO_CONFIG rx_config;
	mcp2518fd_rx_fifo_configure_reset(&rx_config);
	rx_config.FifoSize = 15;
	rx_config.PayLoadSize = 0b000;
	rx_config.RxTimeStampEnable = 0;
	mcp2518fd_rx_fifo_configure(CAN_FIFO_CH2, &rx_config);

	// Initialize RAM
	mcp2518fd_ram_init(0x00);

	// Select Normal Mode
	mcp2518fd_opmode_select(CAN_NORMAL_MODE);

	return 0;
}

/*
	Configure filter and mask objects to accept CAN messages
	with SID 0x200 - 0x20F on FIFO2
*/
int8_t mcp2518fd_rx_init()
{
	// configure filter object 0
	CAN_FILTEROBJ_ID fObj;
	fObj.SID = 0x200;
	fObj.SID11 = 0;
	fObj.EID = 0;
	fObj.EXIDE = 0;
	mcp2518fd_filter_configure(CAN_FILTER0, &fObj);

	// configure message object 0
	CAN_MASKOBJ_ID mObj;
	mObj.MSID = 0x7F0; // make mask 4 bits wide from the LSB
	mObj.MSID11 = 0;
	mObj.MEID = 0;
	mObj.MIDE = 1; // match EXIDE bit
	mcp2518fd_filter_mask_configure(CAN_FILTER0, &mObj);

	mcp2518fd_filter_assign(CAN_FILTER0, CAN_FIFO_CH2);

	mcp2518fd_filter_enable(CAN_FILTER0);
}
