/* 
Bare-metal LED blink for Nucleo-F103RB

The RM0008 reference manual has most of the information needed. 
https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf

For every register we want to use, we need:
- The peripheral base address (RM008: Memory map / peripheral address mapping)
- The register offset from that base (RM0008: peripheral register map)
- The bitfield meaning + bit position (RM0008: register description tables)

This program uses:
- RCC (to enable GPIOA clock)
- GPIOA (configure and toggle PA5 (LED on nucleo board))
*/

#include <stdint.h>

#define LED_PIN 5U // LED on Nucleo-F103RB is PA5
#define BUTTON_PIN 13U // Button on Nucleo-F103RB is PC13

/* --- Peripheral base addresses --- */

// Peripherals start at 0x4000_0000 (Table 3 (Register boundary addresses))
#define PERIPH_BASE 0x40000000UL

// RCC starts at 0x4002_1000 (Reset and clock control RCC in Table 3 (Register boundary addresses))
#define RCC_BASE 0x40021000UL

// GPIOA starts at 0x4001_0800 (GPIO Port A in Table 3 (Register boundary addresses))
// GPIOC starts at 0x4001_1000
#define GPIOA_BASE 0x40010800UL
#define GPIOC_BASE 0x40011000UL

/* --- Register offsets --- */

// (Reset and clock control RCC in Table 3, link to Table 18 (RCC Register map))
#define RCC_APB2ENR_OFFSET 0x18UL // APB2 peripheral clock enable register

// (GPIOx Table 3, link to Table 59 (GPIO register map))
#define GPIOx_CRL_OFFSET 0x00UL // Configuration register LOW (pins 0..7)
#define GPIOx_CRH_OFFSET 0x04UL // Configuration register HIGH (pins 8..15)
#define GPIOx_BSRR_OFFSET 0x10UL // Bit set/reset register ([15:0] bit set, [31:16] bit reset)
#define GPIOx_IDR_OFFSET 0x08UL // Input data register

/* --- Register addresses (base + offset) */

// volatile prevents compiler from optimizing (important for hardware access)
#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_APB2ENR REG32(RCC_BASE + RCC_APB2ENR_OFFSET)

#define GPIOA_CRL REG32(GPIOA_BASE + GPIOx_CRL_OFFSET) // Configuration Register Low
#define GPIOA_BSRR REG32(GPIOA_BASE + GPIOx_BSRR_OFFSET) // Bit Set/Reset Register
#define GPIOC_CRH REG32(GPIOC_BASE + GPIOx_CRH_OFFSET) // Configuration Register High
#define GPIOC_IDR REG32(GPIOC_BASE + GPIOx_IDR_OFFSET) // Input Data Register

/* --- Bit positions / field encodings --- */
// [peripheral] chapter -> register description -> bitfield tables

// 7.3 RCC registers -> 7.3.7 APB2 peripheral clock enable register
#define RCC_APB2ENR_IOPAEN_BIT 2U // I/O port a enable
#define RCC_APB2ENR_IOPCEN_BIT 4U // I/O port c enable

/* 9.2 GPIO registers -> 9.2.1 port configuration register low
Each pin uses 4 bits: 
 - [1:0] MODEy (input/output(with max speeds))
 - [3:2] CNFy (input/output config)
For output: 
 - MODE = 10 (output mode, max speed 2 MHz)
 - CNF = 00 (in output mode, general-purpose push-pull)
 - combined is 0b0010
*/ 
#define GPIO_CRL_PIN5_SHIFT 20U // pin 5 is [23:20] on CRL
#define GPIO_CRH_PIN13_SHIFT 20U // pin 13 is [23:20] on CRH

#define GPIO_CRL_PIN_MASK 0xFU // 4 bit mask
#define GPIO_CRL_OUTPUT_2MHZ_PP 0b0010 // CNF[3:2] MODE[1:0]

#define GPIO_CRH_PIN_MASK 0xFU // 4 bit mask
#define GPIO_CRH_INPUT_F 0b0100 // CNF[3:2] MODE[1:0]

/* Simple delay that just use empty instructions */
static void delay (volatile uint32_t n) {
    while (n--) {
        __asm volatile ("nop"); // No OPeration
    }
}

int main(void) {
    RCC_APB2ENR |= (1U << RCC_APB2ENR_IOPAEN_BIT); // enable peripheral clock to GPIOA
    RCC_APB2ENR |= (1U << RCC_APB2ENR_IOPCEN_BIT); // enable peripheral clock to GPIOC
    
    GPIOA_CRL &= ~(GPIO_CRL_PIN_MASK << GPIO_CRL_PIN5_SHIFT); // set PA5 pins all to 0
    GPIOA_CRL |= (GPIO_CRL_OUTPUT_2MHZ_PP << GPIO_CRL_PIN5_SHIFT); // configure PA5

    GPIOC_CRH &= ~(GPIO_CRH_PIN_MASK << GPIO_CRH_PIN13_SHIFT);
    GPIOC_CRH |= (GPIO_CRH_INPUT_F << GPIO_CRH_PIN13_SHIFT);
    
    int delay_time = 200000U;

    while(1) {
        if (GPIOC_IDR & (1U << BUTTON_PIN)) {
            delay_time = 200000;
        } 
        else {
            delay_time = 50000;
        }

        GPIOA_BSRR = (1U << LED_PIN); // set LED
        delay(delay_time);

        GPIOA_BSRR = (1U << (LED_PIN + 16)); // reset LED
        delay(delay_time);
    } 
}
