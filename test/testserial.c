#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#include "log.h"

#include "serial.h"
#include "transport.h"

///////////////////////////////////////////////////////////////
//test module
void print_hex_buffer(char *buf, int size);		
void serial_test();
void transport_test();

///////////////////////////////////////////////////////////////
static int ds_child_died = 0;
static void ds_child_exit_handler(int s) {
	ds_child_died = 1;
}
static void ds_sig_exit_handler(int s) {
	log_debug("Caught signal %d", s);
	exit(1);
}
static void ds_sigpipe_handler(int s) {
	log_warn("Caught SIGPIPE");
}
static void ds_exit_handler(void) {
	log_debug("inside exit handler");
}

static void sig_set() {
	struct sigaction sigHandler;

	memset(&sigHandler, 0, sizeof(sigHandler));

	sigHandler.sa_handler = ds_sig_exit_handler;
	sigemptyset(&sigHandler.sa_mask);
	sigaction(SIGINT, &sigHandler, NULL);
	sigaction(SIGTERM, &sigHandler, NULL);

	sigHandler.sa_handler = ds_child_exit_handler;
	sigaction(SIGCHLD, &sigHandler, NULL);

	sigHandler.sa_handler = ds_sigpipe_handler;
	sigaction(SIGPIPE, &sigHandler, NULL);

	atexit(ds_exit_handler);
}

int main(int argc, char *argv[]) {
	sig_set();

	log_init(argv[0], LOG_OPT_DEBUG | LOG_OPT_CONSOLE_OUT | LOG_OPT_TIMESTAMPS | LOG_OPT_FUNC_NAMES);

	while (1) {
		serial_test();
		//transport_test();
	}
	return 0;
}

////////////////////////////////////////////////////////////////
void serial_test() {
	//const char *dev = "/dev/pts/5";
	const char *dev = "/dev/ttyUSB0";
	int baud = 115200;
	int fd = serial_open(dev, baud);
	if (fd < 0) {
		log_debug("open serial port failed : %s, %d", dev, baud);
		exit(-1);
	}
	serial_flush(fd);

	log_info("start test serial--------------->:");

	int test_cnt = 0;
	int test_cnt_err = 0;
	while (1) {
		char buf[256];
		int size;
		int ret;
	
		const char *msg = "helloworld, helloworld";
		size = strlen(msg);
		memcpy(buf, msg, size);

		log_info("write data to serial...");
		ret = serial_write(fd, buf, size, 80);
		if (ret < 0) {
			log_debug("serial write failed");
			exit(-1);
		} else if (ret == 0) {
			log_debug("serial write timeout");
			exit(-1);
		} else if (ret != size) {
			log_debug("serial write error!");
			exit(-1);
		}

		log_debug("read data from serial...");
		ret = serial_read(fd, buf, sizeof(buf), 4080);
		if (ret < 0) {
			log_debug("serial read error!");	
			exit(-1);
		} else if (ret == 0) {
			;
			continue;
		}
		log_debug_hex("Read:", buf, ret);

		test_cnt++;
		if (memcmp(buf, msg, strlen(msg)) != 0) {
			test_cnt_err++;
		}
		if (test_cnt >= 200) {
			break;
		}
	}

	log_info("test over : success : %d, error : %d", test_cnt - test_cnt_err, test_cnt_err);

	exit(0);
}

void transport_test() {
	//const char *dev = "/dev/pts/2";
	const char *dev = "/dev/ttyUSB0";
	int baud = 115200;

	log_info("transport open..");
	int fd = transport_open(dev, baud);
	if (fd < 0) {
		log_debug("transport_open failed: %d", fd);
		exit(-1);
	}
	
	log_info("test transport start----------->:");
	int test_cnt = 0;
	int test_cnt_err = 0;
	while (1) {
		char buf[256];
		int size;
		int ret;

		const char *msg = "helloworld, helloworld";
		size = strlen(msg);
		memcpy(buf, msg, size);

		log_info("write data to transport...");
		ret = transport_write(buf, size, 80);
		if (ret < 0) {
			log_debug("transport_write failed");
			exit(-1);
		} else if (ret == 0) {
			log_debug("transport_write timeout");
			exit(-1);
		} else if (ret != size) {
			log_debug("transport_write error!");
			exit(-1);
		}


		log_debug("read data from transport...");
		ret = transport_read(buf, sizeof(buf), 4080);
		if (ret < 0) {
			log_debug("transport_read read error!");	
			exit(-1);
		} else if (ret == 0) {
			;
			continue;
		}
		log_debug_hex("Read:", buf, ret);		

		test_cnt++;
		if (memcmp(buf, msg, strlen(msg)) != 0) {
			test_cnt_err++;
		}
		if (test_cnt >= 200) {
			break;
		}
	}
	log_info("test over : success : %d, error : %d", test_cnt - test_cnt_err, test_cnt_err);

	exit(0);
}

