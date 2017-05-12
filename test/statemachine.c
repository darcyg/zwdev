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

#include "statemachine.h"

///////////////////////////////////////////////////////////////
//test module
void test_statemachine();

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
		test_statemachine();
	}
	return 0;
}

////////////////////////////////////////////////////////////////
enum {
	STATE_1 = 0,
	STATE_2 = 1,
	STATE_3 = 2,
};
enum {
	E_LOW_100 = 1,
	E_HIGH_100_LOW_300 = 3,
	E_HIGH_300_LOW_500 = 5,
	E_HIGH_500 = 7,
};
void *s1_a_low_100(stStateMachine_t *sm, stEvent_t *event);
void *s1_a_high_100_low_300(stStateMachine_t *sm, stEvent_t *event);
int s1_t_high_100_low_300(stStateMachine_t *sm, stEvent_t *event);

void *s2_a_high_100_low_300(stStateMachine_t *sm, stEvent_t *event);
void *s2_a_high_300_low_500(stStateMachine_t *sm, stEvent_t *event);
int s2_t_high_300_low_500(stStateMachine_t *sm, stEvent_t *event);

void *s3_a_high_300_low_500(stStateMachine_t *sm, stEvent_t *event);
void *s3_a_high_500(stStateMachine_t *sm, stEvent_t *event);
int s3_t_high_500(stStateMachine_t *sm, stEvent_t *event);

stStateMachine_t testSm = {
	3, STATE_1, STATE_1, {
		{STATE_1, 2, NULL, {
				{E_LOW_100, s1_a_low_100, NULL},
				{E_HIGH_100_LOW_300, s1_a_high_100_low_300, s1_t_high_100_low_300},
			} 
		}, //state1
		{STATE_2, 2, NULL, {	
				{E_HIGH_100_LOW_300, s2_a_high_100_low_300, NULL},
				{E_HIGH_300_LOW_500, s2_a_high_300_low_500, s2_t_high_300_low_500},
			} 
		}, //state 2
		{STATE_3, 2, NULL, {
				{E_HIGH_300_LOW_500, s3_a_high_300_low_500, NULL},
				{E_HIGH_500, s3_a_high_500, s3_t_high_500},
			}
		}
	},
};

void test_statemachine() {
	state_machine_init(&testSm);
	while (1) {
		int r = rand()%1000;
		stEvent_t event = {0, 0};
		if (r < 100) {
			//log_debug("r is %d", r);
			event.eid = E_LOW_100;
		} else if (r > 100 && r < 300) {
			//log_debug("r is %d", r);
			event.eid = E_HIGH_100_LOW_300;
		} else if (r > 300 && r < 500) {
			//log_debug("r is %d", r);
			event.eid = E_HIGH_300_LOW_500;
		} else if (r > 500) {
			//log_debug("r is %d", r);
			event.eid = E_HIGH_500;
		}
		state_machine_step(&testSm, &event);
		usleep(10000);
	}
	state_machine_free(&testSm);
}


void *s1_a_low_100(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("--------------");
	log_debug("[%s]", __func__);
	return NULL;
}
void *s1_a_high_100_low_300(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("--------------");
	log_debug("[%s]", __func__);
	return NULL;
}
int s1_t_high_100_low_300(stStateMachine_t *sm, stEvent_t *event) {
	int new_state = STATE_2;
	log_debug("[%s] state from %d to %d", __func__, sm->state, new_state);
	return new_state;
}

void *s2_a_high_100_low_300(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("--------------");
	log_debug("[%s]", __func__);
	return NULL;
}
void *s2_a_high_300_low_500(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("--------------");
	log_debug("[%s]", __func__);
	return NULL;
}
int s2_t_high_300_low_500(stStateMachine_t *sm, stEvent_t *event) {
	int new_state = STATE_3;
	log_debug("[%s] state from %d to %d", __func__, sm->state, new_state);
	return new_state;
}

void *s3_a_high_300_low_500(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("--------------");
	log_debug("[%s]", __func__);
	return NULL;
}
void *s3_a_high_500(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("--------------");
	log_debug("[%s]", __func__);
	return NULL;
}
int s3_t_high_500(stStateMachine_t *sm, stEvent_t *event) {
	int new_state = STATE_1;
	log_debug("[%s] state from %d to %d", __func__, sm->state, new_state);
	return new_state;
}

