#ifndef _TRANSPORT_H_
#endif _TRANSPORT_H_

#define DEV_NAME_MAX_LEN    32

typedef struct stTransport {
  char dev[DEV_NAME_MAX_LEN];
  int buadrate;
  int fd;
} stTransport_t;

int transport_open(const char *dev, int buadrate);
int transport_close();
int transport_getfd();
int transport_is_open();
int transport_read(char *buf, int size, int timeout_ms);
int transport_write(char *buf, int size, int timeout_ms);

#endif
