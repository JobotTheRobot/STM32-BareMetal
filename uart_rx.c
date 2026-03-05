// AI-Generated for testing

// uart_rx.c - bare-metal USART2 RX (PA3), store incoming bytes into a buffer.
// Assumes reset clocks (HSI=8MHz) -> PCLK1 ~ 8MHz (default). Uses 115200 8N1.
//
// If your clock setup differs, BRR must be recomputed.

#include <uart_rx.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

// --- Base addresses (RM0008) ---
#define PERIPH_BASE     0x40000000UL
#define APB1PERIPH_BASE (PERIPH_BASE + 0x00000000UL)
#define APB2PERIPH_BASE (PERIPH_BASE + 0x00010000UL)

#define RCC_BASE        (PERIPH_BASE + 0x00021000UL)
#define GPIOA_BASE      (APB2PERIPH_BASE + 0x00000800UL)
#define USART2_BASE     (APB1PERIPH_BASE + 0x00004400UL)

// --- RCC registers ---
#define RCC_APB2ENR REG32(RCC_BASE + 0x18UL)
#define RCC_APB1ENR REG32(RCC_BASE + 0x1CUL)

// Bits
#define RCC_APB2ENR_IOPAEN   (1U << 2)
#define RCC_APB1ENR_USART2EN (1U << 17)

// --- GPIOA registers ---
#define GPIOA_CRL REG32(GPIOA_BASE + 0x00UL)

// --- USART registers (USART2) ---
#define USART2_SR   REG32(USART2_BASE + 0x00UL)
#define USART2_DR   REG32(USART2_BASE + 0x04UL)
#define USART2_BRR  REG32(USART2_BASE + 0x08UL)
#define USART2_CR1  REG32(USART2_BASE + 0x0CUL)
#define USART2_CR2  REG32(USART2_BASE + 0x10UL)
#define USART2_CR3  REG32(USART2_BASE + 0x14UL)

// SR bits
#define USART_SR_RXNE (1U << 5)

// CR1 bits
#define USART_CR1_UE  (1U << 13)
#define USART_CR1_RE  (1U << 2)
#define USART_CR1_TE  (1U << 3)

// --- Simple ring/buffer ---
#define RX_MAX (16U * 1024U)
static uint8_t rx_buf[RX_MAX];
static volatile uint32_t rx_len = 0;
static volatile uint32_t rx_wr  = 0;

static void usart2_init_115200_8n1_default8MHz(void) {
  // Enable clocks
  RCC_APB2ENR |= RCC_APB2ENR_IOPAEN;
  RCC_APB1ENR |= RCC_APB1ENR_USART2EN;

  // PA2 (USART2_TX) config optional; PA3 (USART2_RX) needed.
  // GPIOA_CRL: each pin 4 bits.
  // PA3 input floating: MODE3=00, CNF3=01 => 0b0100
  // PA2 AF push-pull 2MHz (optional): MODE2=10, CNF2=10 => 0b1010
  // If you truly only receive, TX pin config can be omitted.

  // Clear PA2/PA3 config fields
  GPIOA_CRL &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));

  // PA2 = AF PP 2MHz
  GPIOA_CRL |=  (0xAU << (2U * 4U));
  // PA3 = input floating
  GPIOA_CRL |=  (0x4U << (3U * 4U));

  // 8N1: CR2 STOP=00 default, parity disabled default in CR1, 8-bit default (M=0).
  USART2_CR1 = 0;
  USART2_CR2 = 0;
  USART2_CR3 = 0;

  // Baud = 115200 with PCLK1 = 8 MHz (default after reset if you don't re-clock):
  // USARTDIV = 8e6 / (16 * 115200) = 4.340277...
  // Mantissa = 4, Fraction = round(0.340277*16)=5 -> BRR = (4<<4) | 5 = 0x45
  USART2_BRR = 0x0045U;

  // Enable RX (and TX if you want later), then USART
  USART2_CR1 |= USART_CR1_RE | USART_CR1_TE;
  USART2_CR1 |= USART_CR1_UE;
}

static uint8_t usart2_read_byte_blocking(void) {
  while ((USART2_SR & USART_SR_RXNE) == 0) { }
  return (uint8_t)(USART2_DR & 0xFFU);
}

int main(void) {
  usart2_init_115200_8n1_default8MHz();

  // Minimal protocol: first 4 bytes = little-endian payload length
  uint32_t expected = 0;
  expected |= (uint32_t)usart2_read_byte_blocking() << 0;
  expected |= (uint32_t)usart2_read_byte_blocking() << 8;
  expected |= (uint32_t)usart2_read_byte_blocking() << 16;
  expected |= (uint32_t)usart2_read_byte_blocking() << 24;

  if (expected > RX_MAX) expected = RX_MAX;

  rx_len = expected;
  rx_wr = 0;

  while (rx_wr < rx_len) {
    uint8_t b = usart2_read_byte_blocking();
    rx_buf[rx_wr++] = b;
  }

  // At this point, rx_buf[0..rx_len-1] contains the received binary.
  // Next step in a bootloader: validate + program into flash.
  while (1) { }
}