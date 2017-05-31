#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "app.h"
#include "log.h"
#include "parse.h"
#include "file_event.h"
#include "cmd.h"
#include "api.h"
#include "classcmd.h"
#include "jansson.h"


void do_cmd_exit(char *argv[], int argc);
void do_cmd_quit(char *argv[], int argc);
void do_cmd_init(char *argv[], int argc);
void do_cmd_list(char *argv[], int argc);
void do_cmd_include(char *argv[], int argc);
void do_cmd_exclude(char *argv[], int argc);
void do_cmd_onoff(char *argv[], int argc);
void do_cmd_help(char *argv[], int argc);

static stCmd_t cmds[] = {
	{"exit", do_cmd_exit, "exit the programe!"},
	{"init", do_cmd_init, "init zwave controller"},
	{"list", do_cmd_list, "list all zwave devices"},
	{"include", do_cmd_include, "include a zwave device"},
	{"exclude", do_cmd_exclude, "exclude a zwave device"},
	{"onoff", do_cmd_onoff, "onff light"},
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
		return;
	}

	//state_machine_step(&smApp, e);

	FREE(e);
	
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

		//int i = 0;
		//for (i = 0; i < argc; i++) {
			//log_debug("argv[%d] is %s", i, argv[i]);
		//}

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
	app_util_push(E_INIT, NULL);
}
void do_cmd_list(char *argv[], int argc) {
	stAppEnv_t *ae = app_util_getae();
	
	int i = 0;

	for (i = 0; i < sizeof(ae->devs)/sizeof(ae->devs[0]); i++) {
		stDevice_t *dev = &ae->devs[i];
		if (dev->id == 0) {
			continue;
		}
	
		log_debug_hex("class:", dev->class, dev->clen);
		json_t *jattrs = json_object();
		if (jattrs != NULL) {
			emClass_t class[32];
			int j;
			for (j = 0; j < dev->clen; j++) {
				class[j] = dev->class[j];
			}
			class_cmd_to_attrs(dev->id, class, dev->clen, jattrs);
			
			char *jattr_str = json_dumps(jattrs, 0);
			if (jattr_str != NULL) {
				log_debug("id: %d, attrs is [%s]", dev->id, jattr_str);
				free(jattr_str);
			}
			json_decref(jattrs);
		}
	}
}
void do_cmd_include(char *argv[], int argc) {
	log_debug("not implement!");
}
void do_cmd_exclude(char *argv[], int argc) {
	log_debug("not implement!");
}
void do_cmd_onoff(char *argv[], int argc) {
	log_debug("not implement!");
}

void do_cmd_help(char *argv[], int argc) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		log_debug("%-12s\t-\t%s", cmds[i].name, cmds[i].desc);
	}
}



