// AI-Generated for testing

// uart_send.c - send a binary file over a serial port (8N1).
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

static speed_t baud_to_speed(int baud) {
  switch (baud) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    default: return B115200;
  }
}

static int serial_open_cfg(const char *dev, int baud) {
  int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) return -1;

  struct termios tio;
  if (tcgetattr(fd, &tio) != 0) return -1;

  cfmakeraw(&tio);
  tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8; // 8 data bits
  tio.c_cflag |= (CLOCAL | CREAD);            // enable receiver
  tio.c_cflag &= ~(PARENB | PARODD);          // no parity
  tio.c_cflag &= ~CSTOPB;                     // 1 stop bit
  tio.c_cflag &= ~CRTSCTS;                    // no HW flow control

  speed_t spd = baud_to_speed(baud);
  cfsetispeed(&tio, spd);
  cfsetospeed(&tio, spd);

  tio.c_cc[VMIN]  = 0;    // non-blocking-ish reads
  tio.c_cc[VTIME] = 10;   // 1.0s read timeout units (0.1s)

  if (tcsetattr(fd, TCSANOW, &tio) != 0) return -1;
  tcflush(fd, TCIOFLUSH);
  return fd;
}

static int write_all(int fd, const uint8_t *p, size_t n) {
  while (n) {
    ssize_t w = write(fd, p, n);
    if (w <= 0) return -1;
    p += (size_t)w;
    n -= (size_t)w;
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 3) return 2;
  const char *dev = argv[1];
  const char *path = argv[2];
  int baud = 115200;

  int sfd = serial_open_cfg(dev, baud);
  if (sfd < 0) return 3;

  int ffd = open(path, O_RDONLY);
  if (ffd < 0) return 4;

  // Minimal “protocol”: send 4-byte little-endian length, then raw bytes.
  uint8_t buf[1024];
  uint32_t total = 0;

  // First pass: get file size by reading.
  for (;;) {
    ssize_t r = read(ffd, buf, sizeof(buf));
    if (r < 0) return 5;
    if (r == 0) break;
    total += (uint32_t)r;
  }
  lseek(ffd, 0, SEEK_SET);

  uint8_t len_le[4] = {
    (uint8_t)(total >> 0),
    (uint8_t)(total >> 8),
    (uint8_t)(total >> 16),
    (uint8_t)(total >> 24),
  };
  if (write_all(sfd, len_le, 4) != 0) return 6;

  uint32_t sent = 0;
  while (sent < total) {
    ssize_t r = read(ffd, buf, sizeof(buf));
    if (r < 0) return 7;
    if (r == 0) break;
    if (write_all(sfd, buf, (size_t)r) != 0) return 8;
    sent += (uint32_t)r;
  }

  close(ffd);
  close(sfd);
  return 0;
}