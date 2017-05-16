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
#include "api.h"


///////////////////////////////////////////////////////////////
//test module
void api_test();

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
		api_test();
	}
	return 0;
}

////////////////////////////////////////////////////////////////
void api_call_callback(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error) {
	//log_debug("api [%s] call over with ret : %d", api_name(api), error);
	if (param != NULL) {
		FREE(param);
	}
	return;
}
void api_return_callback(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error) {
	//log_debug("api [%s] return with ret : %d", api_name(api), error);

	if (param != NULL) {
		//api_param_view(api, param, state);
	}
	
	if (param != NULL) {
		FREE(param);
	}
	return;
}

void timerout_cb(struct timer *t) {
	log_info("========================api test==================");
	timer_set(&th, t, 8000);
	
	api_call(CmdZWaveGetVersion, NULL, 0);

	api_call(CmdSerialApiGetInitData, NULL, 0);

	stNodeProtoInfoIn_t npii = { 0x01};
	api_call(CmdZWaveGetNodeProtoInfo, (stParam_t*)&npii, sizeof(stNodeProtoInfoIn_t));

	api_call(CmdSerialApiGetCapabilities, NULL, 0);

	/*
	api_exec(CmdZWaveGetControllerCapabilities,NULL);

	api_exec(CmdZWaveGetControllerCapabilities,NULL);

	api_exec(CmdMemoryGetId, NULL);

	api_exec(CmdZWaveGetSucNodeId, NULL);
	
	api_exec(CmdSerialApiApplNodeInformation, NULL);

	static int funcID = 0x1;
	stAddNodeToNetworkIn_t antni = {0x81, funcID++};
	api_exec(CmdZWaveAddNodeToNetwork, &antni);
	*/
}

void api_in(void *arg, int fd) {
	api_step();
}


void api_test() {

	struct timer tr;
	timer_init(&tr, timerout_cb);
	timer_set(&th, &tr, 1000);

	struct file_event_table fet;
	file_event_init(&fet);


	int ret = -1;
	ret = api_init(&th, api_call_callback, api_return_callback);
	if (ret != 0) {
		log_debug("session init failed!");
		exit(-1);
	}
	file_event_reg(&fet, api_getfd(), api_in, NULL, NULL);

	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}
}

