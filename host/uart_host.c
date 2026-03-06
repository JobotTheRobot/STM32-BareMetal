#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>    // File control definitions (flags for open, etc)
#include <unistd.h>   // POSIX system calls (open, read, write, close)
#include <termios.h>  // POSIX terminal/serial port configuration API (https://man7.org/linux/man-pages/man3/termios.3.html)

void test_host(int file_descriptor);

int main() {

	int fd = open("/dev/ttyACM0", O_RDWR);	// open serial device for read/write (open returns file descriptor)

	struct termios tty; 					// struct holding UART settings
	tcgetattr(fd, &tty);					// read current config


	// UART Format: 115200 8N1 (Baud 115200, 8-bit data size, no parity, 1 stop bit)

	cfsetispeed(&tty, B115200);				// set RX baud rate
	cfsetospeed(&tty, B115200); 			// set TX baud rate

	tty.c_cflag &= ~CSIZE; 					// clear current data-size bits
	tty.c_cflag |=  CS8;					// set 8 data bits
	tty.c_cflag &= ~PARENB;					// no parity
	tty.c_cflag &= ~CSTOPB; 				// 1 stop bit
	tty.c_cflag |=  CREAD; 					// enable receiver
	tty.c_cflag |=  CLOCAL;					// ignore modem control lines (needed for USB serial devices)

	tty.c_iflag = 0; // raw
	tty.c_oflag = 0;
	tty.c_lflag = 0;

	tcsetattr(fd, TCSANOW, &tty);			// apply config immediately

	test_host(fd);

	return 0;
}

void test_host(int fd) {

	uint8_t tx_buffer[] = { 'H', 'e', 'l', 'l', 'o', '\r', '\n' };
	uint8_t rx_buffer[sizeof(tx_buffer)];

	for (uint32_t i = 0; i < sizeof(rx_buffer); i++) {
		rx_buffer[i] = 0;
	}

	write(fd, tx_buffer, sizeof(tx_buffer));			// send all bytes to target

	for (uint32_t i = 0; i < sizeof(rx_buffer); i++) {
		read(fd, &rx_buffer[i], 1);						// read echoed bytes back one at a time
	}

	printf("Sent:     ");
	for (uint32_t i = 0; i < sizeof(tx_buffer); i++) {
		printf("%c", tx_buffer[i]);
	}
	printf("\n");

	printf("Received: ");
	for (uint32_t i = 0; i < sizeof(rx_buffer); i++) {
		printf("%c", rx_buffer[i]);
	}
	printf("\n");

	close(fd);
}