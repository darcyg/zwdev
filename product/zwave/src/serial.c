#include "serial.h"

#include <stdio.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "log.h"

static struct termios options;			//place for settings for serial port

int serial_open(const char *dev, int baud) {
  int fd;
  switch (baud) {
  case (50):          baud = B50;         break;
  case (75):          baud = B75;         break;
  case (110):         baud = B110;        break;
  case (134):         baud = B134;        break;
  case (150):         baud = B150;        break;
  case (200):         baud = B200;        break;
  case (300):         baud = B300;        break;
  case (600):         baud = B600;        break;
  case (1200):        baud = B1200;       break;
  case (1800):        baud = B1800;       break;
  case (2400):        baud = B2400;       break;
  case (4800):        baud = B4800;       break;
  case (9600):        baud = B9600;       break;
  case (19200):       baud = B19200;      break;
  case (38400):       baud = B38400;      break;
  case (57600):       baud = B57600;      break;
  case (115200):      baud = B115200;     break;
  case (230400):      baud = B230400;     break;
  case (460800):      baud = B460800;     break;
  case (500000):      baud = B500000;     break;
  case (576000):      baud = B576000;     break;
  case (921600):      baud = B921600;     break;
  case (1000000):     baud = B1000000;    break;
  case (1152000):     baud = B1152000;    break;
  case (1500000):     baud = B1500000;    break;
  case (2000000):     baud = B2000000;    break;
  case (2500000):     baud = B2500000;    break;
  case (3000000):     baud = B3000000;    break;
  case (3500000):     baud = B3500000;    break;
  case (4000000):     baud = B4000000;    break;
  default:
    return -1;
  }
  
  //open the device(com port) to be non-blocking (read will return immediately)
  fd = open(dev, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    log_debug("Couldn't open serial device \"%s\"(%s)", dev, strerror(errno));
    return -1;
  }
  
  if (tcgetattr(fd,&options) == -1) {
    log_debug("Error getting port settings (%s)", strerror(errno));
    return -2;
  }

  options.c_iflag &= ~(INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF);
  options.c_iflag = IGNBRK | IGNPAR;
  options.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
  options.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS);
  options.c_cflag |= CS8 | CREAD | HUPCL | CLOCAL;
  options.c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);

  cfsetispeed(&options, baud);
  cfsetospeed(&options, baud);

  if (tcsetattr(fd,TCSAFLUSH,&options) == -1) {
    log_debug("Error setting port settings (%s)", strerror(errno));
    return -3;
  }
  
  fcntl(fd, F_SETFL, O_NONBLOCK);
  
  return fd;
}

int serial_close(int fd) {
  if (fd <= 0) {
    return -1;
  }
  
  close(fd);
  fd = -1;

  return 0;
}

int serial_write(int fd, char *buf, int size, int timeout_ms) {
  if (fd < 0 || buf == NULL || size <= 0) {
    log_debug("error, err argments, %s, %d", __func__, __LINE__);
    return -1;
  }

  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  int ret = select(fd + 1, NULL, &fds, NULL, &tv);
  if (ret < 0) {
    log_debug("error : select, %s, %d", __func__, __LINE__);
    return -2;
  }
  if (ret == 0) {
    return 0;
  }

  int retry_write_cnt = 0;
 retry_write:
  ret = write(fd, buf, size);
  retry_write_cnt++;
  if (ret <= 0) {
    if (errno == EAGAIN && retry_write_cnt < 3) {
      tv.tv_sec = 0;
      tv.tv_usec = 1000;
      select(0, NULL, NULL, NULL, &tv);
      goto retry_write;
    } 
    log_debug("error: read, %s, %d", __func__, __LINE__);
    return -3;
  }

  return ret;
}

int serial_read(int fd, char *buf, int size, int timeout_ms) {
  if (fd < 0 || buf == NULL || size <= 0) {
    log_debug("error, err argments, %s, %d", __func__, __LINE__);
    return -1;
  }
  
  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  int ret = select(fd + 1, &fds, NULL, NULL, &tv);
  if (ret < 0) {
    log_debug("error : select, %s, %d", __func__, __LINE__);
    return -2;
  }
  if (ret == 0) {
    return 0;
  }

  if (!FD_ISSET(fd, &fds)) {
    log_debug("error : select, %s, %d", __func__, __LINE__);
    return -3;
  }

  ret = read(fd, buf, size);
  if (ret <= 0) {
    log_debug("error: read, %s, %d", __func__, __LINE__);
    return -4;
  }

  return ret;
}

int serial_clear(int fd) {
  if (fd < 0) {
    log_debug("error, err argments, %s, %d", __func__, __LINE__);
    return -1;
  }
  if (tcsetattr(fd,TCSAFLUSH,&options) == -1) {
    log_debug("Error setting port settings (%s)", strerror(errno));
    return -2;
  }
  
  return 0;
}
