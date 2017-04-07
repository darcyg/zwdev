#ifndef _SERIAL_H_
#define _SERIAL_H_


int serial_open(const char *dev, int buadrate);

int serial_close(int fd);

int serial_write(int fd, char *buf, int size, int timeout_ms);

int serial_read(int fd, char *buf, int size, int timeout_ms);

int serial_clear(int fd);

#endif
