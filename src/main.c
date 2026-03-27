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

void blink_init();
void blink(int delay_ms);

int main() {
    mcp2518fd_init(SPI_CLK_RATE);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t data = 0xAAAAAAAA;

    for (;;) {
      blink(500);
      mcp2518fd_write_word(MCP2518FD_REG_CiCON, data);
    }
}

void blink_init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void blink(int delay_ms) {
    gpio_put(LED_PIN, 1);
    sleep_ms(delay_ms);
    gpio_put(LED_PIN, 0);
    sleep_ms(delay_ms);
}