#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "parse.h"
#include "file_event.h"
#include "jansson.h"
#include "json_parser.h"

#include "cmd.h"
#include "zwave_iface.h"


void do_cmd_exit(char *argv[], int argc);
void do_cmd_quit(char *argv[], int argc);
void do_cmd_init(char *argv[], int argc);
void do_cmd_list(char *argv[], int argc);
void do_cmd_help(char *argv[], int argc);
void do_cmd_info(char *argv[], int argc);

void do_cmd_get(char *argv[], int argc);
void do_cmd_set(char *argv[], int argc);
void do_cmd_include(char *argv[], int argc);
void do_cmd_exclude(char *argv[], int argc);
void do_cmd_onoff(char *argv[], int argc);

static stCmd_t cmds[] = {
	{"exit", do_cmd_exit, "exit the programe!"},
	{"init", do_cmd_init, "init zwave controller"},
	{"list", do_cmd_list, "list all zwave devices"},
	{"include", do_cmd_include, "include a zwave device"},
	{"exclude", do_cmd_exclude, "exclude a zwave device : exclude <mac>"},
	{"get", do_cmd_get, "get device class/attr : get <mac> <attr name> [value]"},
	{"set", do_cmd_set, "set device class/attr : set <mac> <attr name> [value]"},
	{"onoff", do_cmd_onoff, "onoff binary switch: onoff <mac> <onoff>"},
	{"info", do_cmd_info, "get zwave network info"},
	{"help", do_cmd_help, "help info"},
};

static stCmdEnv_t ce;

int cmd_init(void *_th, void *_fet) {
	ce.th = _th;
	ce.fet = _fet;
	
	timer_init(&ce.step_timer, cmd_run);
	lockqueue_init(&ce.eq);
	
	ce.fd = 0;
	file_event_reg(ce.fet, ce.fd, cmd_in, NULL, NULL);

	return 0;
}
int cmd_step() {
	timer_cancel(ce.th, &ce.step_timer);
	timer_set(ce.th, &ce.step_timer, 10);
	return 0;
}
int cmd_push(stEvent_t *e) {
	lockqueue_push(&ce.eq, e);
	cmd_step();
	return 0;
}
void cmd_run(struct timer *timer) {
	stEvent_t *e;

	if (!lockqueue_pop(&ce.eq, (void**)&e)) {
		return;
	}

	if (e == NULL) {
		FREE(e);
	}

	cmd_step();
}
void cmd_in(void *arg, int fd) {
	char buf[1024];	
	int  size = 0;

	int ret = read(ce.fd, buf, sizeof(buf));
	if (ret < 0) {
		log_debug("what happend?");
		return;
	}

	if (ret == 0) {
		log_debug("error!");
		return;
	}

	size = ret;
	buf[size] = 0;
	if (buf[size-1] == '\n') {
		buf[size-1] = 0;
		size--;
	}

	if (strcmp(buf, "") != 0) {
		log_debug("console input:[%s]", buf);
		char* argv[20];
		int argc;
		argc = parse_argv(argv, sizeof(argv), buf);

		stCmd_t *cmd = cmd_search(argv[0]);
		if (cmd == NULL) {
			log_debug("invalid cmd!");
		} else {
			cmd->func(argv, argc);
		}
	}
	log_debug("$");
}

stCmd_t *cmd_search(const char *cmd) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		if (strcmp(cmds[i].name, cmd) == 0) {
			return &cmds[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////
void do_cmd_exit(char *argv[], int argc) {
	exit(0);
}
void do_cmd_init(char *argv[], int argc) {
}
void do_cmd_list(char *argv[], int argc) {
	json_t *jdevs = zwave_iface_list();
	if (jdevs != NULL) {
		char *jdevs_str = json_dumps(jdevs, 0);
		if (jdevs_str != NULL) {
			log_debug("devss is [%s]", jdevs_str);
			free(jdevs_str);
		}
		json_decref(jdevs);
	}
}
void do_cmd_include(char *argv[], int argc) {
	int ret = zwave_iface_include();
	if (ret == 0) {
		log_info("include ok");
	} else {
		log_warn("include timeout!");
	}
}
void do_cmd_exclude(char *argv[], int argc) {
	if (argc != 2) {
		log_debug("exclude must has one argment as <mac>");
		return;
	}
	zwave_iface_exclude(argv[1]);
}

void do_cmd_help(char *argv[], int argc) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		log_debug("%-12s\t-\t%s", cmds[i].name, cmds[i].desc);
	}
}

void do_cmd_get(char *argv[], int argc) {
	if (argc != 4) {
		log_debug("error argments!");
		return;
	}
		
	log_debug("get mac:%s,attr:%s, arg:%s", argv[1], argv[2], argv[3]);

	zwave_iface_get(argv[1], argv[2], argv[3]);
}

void do_cmd_set(char *argv[], int argc) {
	if (argc != 4) {
		log_debug("error argments!");
		return;
	}
	log_debug("set mac:%s,attr:%s, arg:%s", argv[1], argv[2], argv[3]);

	zwave_iface_set(argv[1], argv[2], argv[3]);
}


void do_cmd_info(char *argv[], int argc) {
	json_t *jinfo = zwave_iface_info();
	if (jinfo != NULL) {
		char *jinfo_str = json_dumps(jinfo, 0);
		if (jinfo_str != NULL) {
				log_debug("jinfo [%s]",jinfo_str);
				free(jinfo_str);
		}
		json_decref(jinfo);
	}
}


void do_cmd_onoff(char *argv[], int argc) {
	if (argc != 3) {
		log_debug("error argments!");
		return;
	}
	log_debug("onoff:%s, value:%s", argv[1], argv[2]);

	zwave_iface_device_light_onoff(argv[1], atoi(argv[2]));
}


