#include <stdint.h>

// --- Register addresses (Table 3. Register Boundary Addresses) ---
#define RCC_BASE        0x40021000UL
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1C))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

#define GPIOA_BASE      0x40010800UL
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

#define USART2_BASE     0x40004400UL
#define USART2_SR       (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_CR2      (*(volatile uint32_t *)(USART2_BASE + 0x10))


// --- RCC enable bits (7.3.7 APB2 peripheral clock enable register (RCC_APB2ENR)) ---
#define IOPAEN          (1U << 2)
#define USART2EN        (1U << 17)


// --- USART status bits (27.6.1 Status register (USART_SR)) ---
#define RXNE            (1U << 5)
#define TXE             (1U << 7)


// --- masks ---
#define MASK1           0b1 
#define MASK2           0b11
#define MASK4           0b1111


// --- USART control register 1 fields (27.6.4 Control register 1 (USART_CR1)) ---
#define USART_UE_SHIFT      13
#define USART_UE_VALUE      1       // 0: USART disable, 1: USART enable

#define USART_M_SHIFT       12
#define USART_M_VALUE       0       // 0: 8 data bits, 1: 9 data bits

#define USART_PCE_SHIFT     10
#define USART_PCE_VALUE     0       // 0: parity disabled, 1: parity enabled

#define USART_TE_SHIFT      3
#define USART_TE_VALUE      1       // 1: transmitter enabled

#define USART_RE_SHIFT      2
#define USART_RE_VALUE      1       // 1: receiver enabled


// --- USART control register 2 fields (27.6.5 Control register 2 (USART_CR2)) ---
#define USART_STOP_SHIFT    12
#define USART_STOP_VALUE    0b00    // 00: 1 stop bit, 01: 0.5 stop bit, 10: 2 stop bits, 11: 1.5 stop bits


// --- GPIO port configurations (9.2.2 Port configuration register high (GPIOx_CRH)) ---

//  PA2 = USART2_TX
//  GPIOA_CRH controls pins PA8-PA15
//  PA2 bits are [7:4] (CNF9[1:0] and MODE9[1:0])
//      CNF9  = 10 (Alternate function output Push-pull)
//      MODE9 = 11 (Output mode, max speed 50MHz)
#define PA2_CRL_SHIFT       8
#define PA2_CRL_VALUE       0b1011

//  PA3 = USART2_RX
//  GPIOA_CRL controls pins PA0-PA7
//  PA3 bits are [15:12] (CNF3[1:0] and MODE3[1:0])
//      CNF3  = 01 (Floating input)
//      MODE3 = 00 (Input mode)
#define PA3_CRL_SHIFT      12
#define PA3_CRL_VALUE      0b0100


// --- USART2_BRR settings (27.6.3 Baud rate register (USART_BRR)) ---
//  UART Format: 115200 8N1
//  Assuming USART2 clock = 8MHz
//  USARTDIV = fCK / (16 * baud) = 8000000 / (16 * 115200) = 4.3403
//      DIV_Mantissa = 4
//      DIV_Fraction = 0.3404 * 16 = 5.4448 ~= 5
//  BRR[15:4] DIV_Mantissa
//  BRR[3:0]  DIV_Fraction
#define USART2_DIV_MANTISSA     4
#define USART2_DIV_FRACTION     5
#define USART2_BRR_115200       ((USART2_DIV_MANTISSA << 4) | (USART2_DIV_FRACTION))
// #define USART2_BRR_115200 0x271

void uart_init(void) {

    RCC_APB2ENR |= IOPAEN;              // Enable clock to GPIOA
    RCC_APB1ENR |= USART2EN;            // Enable clock to USART2

    GPIOA_CRL &= ~(MASK4 << PA2_CRL_SHIFT);      // clear PA2 [11:8]
    GPIOA_CRL |=  (PA2_CRL_VALUE << PA2_CRL_SHIFT);

    GPIOA_CRL &= ~(MASK4 << PA3_CRL_SHIFT);      // clear PA3 [15:12]
    GPIOA_CRL |=  (PA3_CRL_VALUE << PA3_CRL_SHIFT);

    USART2_BRR =  USART2_BRR_115200;    // Set baud rate

    USART2_CR1 &= ~(MASK1 << USART_M_SHIFT);      // M   = 0:    8 data bits
    USART2_CR1 |=  (USART_M_VALUE << USART_M_SHIFT);

    USART2_CR1 &= ~(MASK1 << USART_PCE_SHIFT);    // PCE = 0:    no parity
    USART2_CR1 |=  (USART_PCE_VALUE << USART_PCE_SHIFT);

    USART2_CR2 &= ~(MASK2 << USART_STOP_SHIFT);   // STOP[1:0] = 00: 1 stop bit
    USART2_CR2 |=  (USART_STOP_VALUE << USART_STOP_SHIFT);

    USART2_CR1 &= ~(MASK1 << USART_TE_SHIFT);     // TE  = 1:    enable transmitter
    USART2_CR1 |=  (USART_TE_VALUE << USART_TE_SHIFT);

    USART2_CR1 &= ~(MASK1 << USART_RE_SHIFT);     // RE  = 1:    enable receiver
    USART2_CR1 |=  (USART_RE_VALUE << USART_RE_SHIFT);

    USART2_CR1 &= ~(MASK1 << USART_UE_SHIFT);     // UE  = 1:    enable USART
    USART2_CR1 |=  (USART_UE_VALUE << USART_UE_SHIFT);
}

void uart_send_byte(uint8_t byte) {
    while ((USART2_SR & TXE) == 0) {}   // TXE = 1: transmit data register empty

    USART2_DR = byte;
}

uint8_t uart_receive_byte(void) {
    while ((USART2_SR & RXNE) == 0) {}  // RXNE = 1: read data register not empty

    return (uint8_t)USART2_DR;
}

int main(void) {
    uart_init();

    while(1) {
        uint8_t byte = uart_receive_byte();     // wait for one received byte
        uart_send_byte(byte);                   // echo byte back
    }
}