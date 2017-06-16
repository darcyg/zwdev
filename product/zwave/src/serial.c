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

#if 0
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
#else 
static void _setTermios(struct termios * _pNewtio, int _buat, char _nbits, char _parity, char _stop) {
	int uBaudRate;
	switch(_buat) {
		case 1200:
			uBaudRate = B1200;
			break;
		case 2400:
			uBaudRate = B2400;
			break;
		case 4800:
			uBaudRate = B4800;
			break;
		case 9600:
			uBaudRate = B9600;
			break;
		case 19200:
			uBaudRate = B19200;
			break;
		case 38400:
			uBaudRate = B38400;
			break;
		case 57600:
			uBaudRate = B57600;
			break;
		case 115200:
			uBaudRate = B115200;
			break;
		default:
			uBaudRate = B115200;
			break;
	}
	bzero(_pNewtio, sizeof(struct termios));

	log_debug("buat : %d, nbits: %d, parity: %c, stop: %d\n", _buat, _nbits, _parity, _stop);
	_pNewtio->c_cflag 	 		 = CLOCAL | CREAD;
	_pNewtio->c_cflag	 		|= uBaudRate;

	switch (_nbits) {
		case 8:
			_pNewtio->c_cflag 	|= CS8;
			break;
		case 7: 
			_pNewtio->c_cflag	|= CS7;
			break;
		case 6: 
			_pNewtio->c_cflag	|= CS6;
			break;
		case 5: 
			_pNewtio->c_cflag	|= CS5;
			break;
	}

	if (_stop == 1) {
		;
	} else {
		_pNewtio->c_cflag		|= CSTOPB;
	}

	if (_parity == 'N') {
		;
	} else if (_parity == 'E') {
		_pNewtio->c_cflag		|= PARENB;
	} else if (_parity == 'O') {
		_pNewtio->c_cflag		|= PARENB | PARODD;
	} else {
		;
	}

	//pNewtio->c_iflag = IGNPAR;
	_pNewtio->c_iflag = IGNPAR;
	//pNewtio->c_iflag = 0;

	_pNewtio->c_oflag = 0;

	_pNewtio->c_lflag = 0; //non ICANON

	_pNewtio->c_cc[VINTR] = 0; 
	_pNewtio->c_cc[VQUIT] = 0;
	_pNewtio->c_cc[VERASE] = 0;
	_pNewtio->c_cc[VKILL] = 0;
	_pNewtio->c_cc[VEOF] = 0;
	_pNewtio->c_cc[VTIME] = 5; //5 *100ms
	_pNewtio->c_cc[VMIN] = 0; //0  ×îÐ¡µÈ´ý×Ö½ÚÊý
	_pNewtio->c_cc[VSWTC] = 0;
	_pNewtio->c_cc[VSTART] = 0;
	_pNewtio->c_cc[VSTOP] = 0;
	_pNewtio->c_cc[VSUSP] = 0;
	_pNewtio->c_cc[VEOL] = 0;
	_pNewtio->c_cc[VREPRINT] = 0;
	_pNewtio->c_cc[VDISCARD] = 0;
	_pNewtio->c_cc[VWERASE] = 0;
	_pNewtio->c_cc[VLNEXT] = 0;
	_pNewtio->c_cc[VEOL2] = 0;
}



static int setfmt(int fd, int _buad, char _nbits, char _parity, char _stop) {
	struct termios newtio;
	//struct termios oldtio;
	int	ret;

	if (fd < 0) {
		return 0;
	}

	//tcgetattr(fd, &oldtio);
	_setTermios(&newtio, _buad, _nbits, _parity, _stop);
	tcflush(fd, TCIFLUSH);
	tcflush(fd, TCOFLUSH);
	ret = tcsetattr(fd, TCSANOW, &newtio);
	if (ret < 0) {
		return 0;
	}
	return 1;
}


static int _read(int fd, char *_c, int _s, int _u) {
	int 	ret;
	int 	en;
	char    es[256];
	char	ch;


	fd_set	fds;
	struct	timeval tv;

	if (_c == NULL || _s < 0 || _u < 0 || fd < 0) {
		return -1;
	}
_read_select_tag:	
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = _s;
	tv.tv_usec = _u;
	//log_debug("s:%d, u:%d", tv.tv_sec, tv.tv_usec);
	ret = select(fd + 1, &fds, NULL, NULL, &tv);
	//log_debug("s:%d, u:%d", tv.tv_sec, tv.tv_usec);
	if (ret < 0) {
		en = errno;
		if (en == EAGAIN || en == EINTR) {
			goto _read_select_tag;
		} else { //if (en == EBADF || en == EINVAL || en == ENOMEM) {
			strerror_r(en, es, sizeof(es));
			log_debug("serial _read: %d,%s", en, es);
			return -3;
		}
	} else if (ret == 0) {
			return 0;
	} else if (FD_ISSET(fd, &fds)) {
read_tag:
		ret = read(fd, &ch, 1);
		if (ret < 0) {
			en = errno;
			if (en == EAGAIN || en == EINTR) {
				goto read_tag;
			} else { 
				strerror_r(en, es, sizeof(es));
				log_debug("serial _read:%d,%s", en, es);
				return -4;
			}
		} else if (ret != 1) {
			log_err("serial _read:ret != 1");
			return -5;
		} else {
			;
		} 
	} else {
			log_err("serial _read:unknown error");
			return -6;
	}

	*_c = ch;
	return ret;
}

//int serial_open(const char *dev, int baud, char nbits, char parity, char stop) {
int serial_open(const char *dev, int baud) {
	int ret;
	char nbits = 8;
	char parity = 'N';
	char stop = 1;
	

	log_debug("dev : %s", dev);

	//ret = open(dev, O_RDWR | O_NOCTTY);
	ret = open(dev, O_RDWR);
	if (ret < 0) {
		log_debug("open file %s failed", dev);
		return -1;
	}

	if (!setfmt(ret, baud, nbits, parity, stop)) {
		/**> TODO: check here, if need to close */
		log_debug("set serial fmt failed!");
		close(ret);
		return -2;
	}

	return ret;
}
int serial_close(int fd) {
	if (fd > 0) {
		close(fd);
	}
	return 0;
}
int serial_write(int fd, char *_buf, int _size, int timeout_ms) {
	int 	ret;
	int 	en;
	fd_set	fds;
	struct	timeval tv;
	char    es[256];

	int _s = timeout_ms/1000;
	int _u = (timeout_ms%1000)*1000;

	if (_buf  == NULL || _size <= 0 || _s < 0 || _u < 0 || fd < 0) {
		log_debug("serial write argments error!");
		return -1;
	}
write_select_tag:
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = _s;
	tv.tv_usec = _u;
	ret = select(fd + 1, NULL, &fds, NULL, &tv);
	if (ret < 0) {
		en = errno;
		if (en == EAGAIN || en == EINTR) {
			goto write_select_tag;
		} else { //if (en == EBADF || en == EINVAL || en == ENOMEM) {
			strerror_r(en, es, sizeof(es));
			log_debug("serial write:%d,%s", en, es);
			return -2;
		}
	} else if (ret == 0) {
		return 0;
	} else if (FD_ISSET(fd, &fds)) {
write_tag:
		ret = write(fd, _buf,  _size);
		if (ret < 0) {
			en = errno;
			if (en == EAGAIN || en == EINTR) {
				goto write_tag;
			} else {
				strerror_r(en, es, sizeof(es));
				log_debug("serial write:%d,%s", en, es);
				return -3;
			}
		} else if ((unsigned)ret != _size) {
			log_debug("serial write:ret != size");
			return -4;
		} else {
			;
		} 
	} else {
		log_err("serial write :unknown error");
		return -5;
	}
	return ret;
}
int serial_read(int fd, char *_buf, int _size, int timeout_ms) {
	int ret;
	unsigned int i;

	int _s = timeout_ms/1000;
	int _u = (timeout_ms%1000)*1000;

	if (_buf  == NULL || _size <= 0 || _s < 0 || _u < 0 || fd < 0) {
		log_debug("serial read argments error!");
		return -1;
	}

	for (i = 0; i < _size; i++) {
		if (i == 0) 
			ret = _read(fd, &(_buf[i]), _s, _u);
		else 	
			ret = _read(fd, &(_buf[i]), 0, _u);
		if (ret == 1) {
			;
		} else if (ret == 0) {
			break;
		} else {
			return ret;
		}
	}

	return i;
}
int serial_flush(int fd) {
	tcflush(fd, TCIFLUSH);
	tcflush(fd, TCOFLUSH);

	char ch;
	while (serial_read(fd, &ch, 1, 400) == 1) {
		//log_debug("%02x", ch);
	}
	return 0;
}

#endif
