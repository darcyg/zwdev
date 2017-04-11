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


#include "common.h"
#include "log.h"
#include "frame.h"
#include "timer.h"
#include "file_event.h"
#include "session.h"


///////////////////////////////////////////////////////////////
//test module
void print_hex_buffer(char *buf, int size);		
void session_test();

///////////////////////////////////////////////////////////////
static int ds_child_died = 0;

struct timer_head th = {
	.first = NULL,
};
///////////////////////////////////////////////////////////////
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
		session_test();
	}
	return 0;
}

////////////////////////////////////////////////////////////////
void print_hex_buffer(char *buf, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		printf("[%02X] ", buf[i]&0xff);
		
		if ( (i+1) % 20 == 0) {
			printf("\n");
		}
	}	
	printf("\n");
}

void send_callback(stDataFrame_t *sf) {
	log_info("-");
	if (sf != NULL) {
		if (sf->error == FE_NONE) {
			log_debug("ok frame");
		} else if (sf->error == FE_SEND_TIMEOUT) {
			log_debug("frame send time out");
		} else if (sf->error == FE_SEND_ACK) {
			log_debug("frame send ok");
		} else if (sf->error == FE_SEND_NAK) {
			log_debug("frame send nak");
		} else if (sf->error == FE_SEND_CAN) {
			log_debug("frame send can");
		} else if (sf->error == FE_RECV_CHECKSUM) {
			log_debug("frame recv checksum error");
		} else if (sf->error == FE_RECV_TIMEOUT) {
			log_debug("frame recv timeout");
		}
		log_info("size is %02x, %02x, retrycnt:%d", sf->size, sf->len, sf->trycnt);
		print_hex_buffer(sf->payload, sf->size);
		//FREE(sf);
	}
	return;
}

void recv_callback(stDataFrame_t *sf) {
	log_info("-");

	if (sf != NULL) {
		if (sf->error == FE_NONE) {
			log_debug("ok frame");
		} else if (sf->error == FE_SEND_TIMEOUT) {
			log_debug("frame send time out");
		} else if (sf->error == FE_SEND_ACK) {
			log_debug("frame send ok");
		} else if (sf->error == FE_SEND_NAK) {
			log_debug("frame send nak");
		} else if (sf->error == FE_SEND_CAN) {
			log_debug("frame send can");
		} else if (sf->error == FE_RECV_CHECKSUM) {
			log_debug("frame recv checksum error: %02x(correct:%02x)", sf->checksum, sf->checksum_cal);
		} else if (sf->error == FE_RECV_TIMEOUT) {
			log_debug("frame recv timeout");
		}

		print_hex_buffer(sf->payload, sf->size);
		FREE(sf);
	}
	return;
}

stDataFrame_t df = {
	.sof = SOF_CHAR,
	.len = 0,
	.type = 0x00,
	.cmd = 0x15,
	.payload = "\x33\x33\x33",
	.size = 0,
	.checksum = 0,
	.timestamp = 0,
	.trycnt = 0,
	.error = 0,
	.do_checksum = 0,
	.checksum_cal = 0,
};


void timerout_cb(struct timer *t) {
	log_debug("timer out!");
	timer_set(&th, t, 15000);
	df.trycnt= 0;
	session_send(&df);
}

void session_in(void *arg, int fd) {
	session_receive_step();
}


void session_test() {

	struct timer tr;
	timer_init(&tr, timerout_cb);
	timer_set(&th, &tr, 3000);

	struct file_event_table fet;
	file_event_init(&fet);


	int ret = -1;
	ret = session_init(&th, (void*)send_callback, (void*)recv_callback);
	if (ret != 0) {
		log_debug("session init failed!");
		exit(-1);
	}
	file_event_reg(&fet, session_getfd(), session_in, NULL, NULL);

	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}
}

