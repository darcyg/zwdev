/*
 * Serial.h
 *
 *  Created on: 2016-5-17
 *      Author: root
 */
#include <stdint.h>
#ifndef SERIAL_H_
#define SERIAL_H_

int serial_open(char *name, uint32_t baud);
int serial_read(const int fd, unsigned char *data);
int serial_write(const int fd, const unsigned char data);

int serial_read_buffer(const int fd, unsigned char *data, uint32_t *count);
int serial_write_buffer(const int fd, unsigned char *data, uint32_t count);

void serial_flush(const int fd);
#endif /* SERIAL_H_ */
