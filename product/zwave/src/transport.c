#include "transport.h"

static stTransport_t tp;

int transport_open(const char *dev, int buadrate) {

  if (!valid_dev(dev)) {
    log_debug("invalid dev: %s", dev);
    return -1;
  }
  
  if (valid_buadrate(buadrate)) {
    log_debug("invalid buadrate: %d", buadrate);
    return -2;
  }

  if (tp.fd != -1) {
    log_debug("transport has been openned! close it first.");
    if (serial_close(tp.fd) != 0) {
      log_debug("serial close failed!");
      return -3;
    } 
    tp.fd = -1;
  }
  
  tp.buadrate = buadrate;
  strcpy(tp.dev, dev);

  tp.fd = serial_open(tp.dev, tp.buadrate);
  if (tp.fd < 0) {
    tp.fd = -1;
    log_debug("serial open %s(%d) failed!", tp.dev, tp.buadrate);
    return -4;
  }

  log_debug("serial open success : %d", tp.fd);
  return 0;
}
int transport_close() {
  if (tp.fd == -1) {
    log_debug("serial has not been openned!");
    return -1;
  }
  
  if (serial_close(tp.fd) != 0) {
    log_debug("seiral close failed !");
    return -2;
  }
  
  tp.fd  = -1;

  return 0;
}

int transport_getfd() {
  return tp.fd;
}

int transport_is_open() {
  return (tp.fd == -1 ? 0 : 1);
}

int transport_read(char *buf, int size, int timeout_ms) {
  if (tp.fd == -1) {
    log_debug("transport not readded");
    return -1;
  }

  int ret = serial_read(tp.fd, buf, size, timeout_ms);
  if (ret < 0) {
    log_debug("serial_read failed!");
    return -2;
  }

  return ret;
}

int transport_write(char *buf, int size, int timeout_ms) {
  if (tp.fd == -1) {
    log_debug("transport not readded");
    return -1;
  }

  int ret = serial_write(tp.fd, buf, size, timeout_ms);
  if (ret < 0) {
    log_debug("serial_write failed!");
    return -2;
  }

  return ret;
}
