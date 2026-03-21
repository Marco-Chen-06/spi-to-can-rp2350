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
int main() {
    mcp2518fd_init(SPI_CLK_RATE);
    for (;;) {
        printf("hello world");
    }
}