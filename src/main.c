#include "mcp2518fd.h"
#include "mcp2518fd_hw.h"

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

int main() {
    blink_builtin_init();
    mcp2518fd_spi_init(SPI_CLK_RATE);
    mcp2518fd_reset();    

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