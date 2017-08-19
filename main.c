#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>



#include "common.h"
#include "log.h"
#include "timer.h"
#include "file_event.h"
#include "jansson.h"
#include "json_parser.h"

#include "uproto.h"
#include "cmd.h"
#include "zwave.h"
#include "zwave_iface.h"
#include "zwave_device.h"
#include "zwave_device_storage.h"
/////////////////////////////////////////////////////////////////////////////////////////////////
void				run_main();
static void ds_child_exit_handler(int s);
static void ds_sig_exit_handler(int s);
static void ds_sigpipe_handler(int s);
static void ds_exit_handler(void);
static void sig_set();
static void timerout_cb(struct timer *t);



/////////////////////////////////////////////////////////////////////////////////////////////////
static int ds_child_died = 0;
struct timer_head th = {.first = NULL};
static int use_cmd = 0;
static char *uart_dev = "/dev/ttyS1";
static int uart_buad = 115200;


extern stZWaveCache_t zc;

int parse_args(int argc, char *argv[]);
int usage();
int write_pid();

/////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
	sig_set();

	log_init(argv[0], LOG_OPT_DEBUG | LOG_OPT_CONSOLE_OUT | LOG_OPT_TIMESTAMPS | LOG_OPT_FUNC_NAMES);

	if (parse_args(argc, argv) != 0) {
		usage();
		return -1;
	}

	if (write_pid() != 0) {
		log_err("zwdevd has startted!");
		return -2;
	}

	run_main();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void timerout_cb(struct timer *t) {
	log_info("[%s] %d : %p", __func__, __LINE__, t);
}

void run_main() {
	log_info("[%s] %d", __func__, __LINE__);


	struct timer tr;
	timer_init(&tr, timerout_cb);

	struct file_event_table fet;
	file_event_init(&fet);

	
	if (use_cmd) {
		cmd_init(&th, &fet);
	}

	//uproto_init(&th, &fet);

	ds_init("/etc/config/dusun/zwave/zwave.db");
	ds_load_alldevs(zc.devs);

	zwave_init(&th, &fet, uart_dev, uart_buad);

	timer_set(&th, &tr, 10);
	log_info("[%s] %d : goto main loop", __func__, __LINE__);
	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}
}

static void ds_child_exit_handler(int s) {
	log_info("[%s] %d", __func__, __LINE__);
	ds_child_died = 1;
}
static void ds_sig_exit_handler(int s) {
	log_info("[%s] %d : %d", __func__, __LINE__, s);
	exit(1);
}
static void ds_sigpipe_handler(int s) {
	log_info("[%s] %d : %d", __func__, __LINE__, s);
}
static void ds_exit_handler(void) {
	log_info("[%s] %d", __func__, __LINE__);
}

static void sig_set() {
	log_info("[%s] %d", __func__, __LINE__);

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


int parse_args(int argc, char *argv[]) {
	int ch = 0;
	while((ch = getopt(argc,argv,"Cd:b:"))!= -1){
		switch(ch){
			case 'C': 
				use_cmd = 1;
				break;
			case 'd':
				uart_dev = optarg;
				break;
			case 'b':
				uart_buad = atoi(optarg);
				break;
			default:
				return -1;
				break;
		}
	}
	return 0;
}

int usage() {
	printf(	"Usage: zwave [options] ...\n"
					"Options:\n"
					"  -C                       Start program with command line for debug.\n"
					"  -d /dev/ttyS1						Use which uart to communicate with zwave. \n"
					"  -b 115200                set uart buadrate.\n"
					"For more infomation, please mail to dlauciende@gmail.com\n");
	return 0;
}

int write_pid() {
	int fd = -1;
	if (access("/var/run/zwdevd.pid", F_OK) != 0) {
		fd = open("/var/run/zwdevd.pid", O_WRONLY | O_CREAT, 0644);
	} else {
		fd = open("/var/run/zwdevd.pid", O_WRONLY);
	}
	
	if (fd < 0) {
		return -1;
	}
	
	if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
		return -2;
	}
	
	char buf[64];
	
	sprintf(buf, "%d\n", (int)getpid());
	if (write(fd, buf, strlen(buf)) != strlen(buf)) {
		return -3;
	}
	
	return 0;
}
