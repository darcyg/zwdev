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
#include "json_parser.h"


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

static stCmd_t cmds[] = {
	{"exit", do_cmd_exit, "exit the programe!"},
	{"init", do_cmd_init, "init zwave controller"},
	{"list", do_cmd_list, "list all zwave devices"},
	{"include", do_cmd_include, "include a zwave device"},
	{"exclude", do_cmd_exclude, "exclude a zwave device"},
	{"get", do_cmd_get, "get device class/attr : get 0x3B 0x25 0x03"},
	{"set", do_cmd_set, "set device class/attr : set 0x3B 0x25 0x03 0x00/0x01"},
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
	app_util_push_cmd(E_INIT, NULL, 0);
	app_util_push_cmd(E_CLASS, NULL, 0);
	app_util_push_cmd(E_ATTR, NULL, 0);
}
void do_cmd_list(char *argv[], int argc) {
	stAppEnv_t *ae = app_util_getae();
	
	json_t *jdevs = json_array();

	int i = 0;
	for (i = 0; i < sizeof(ae->devs)/sizeof(ae->devs[0]); i++) {
		stDevice_t *dev = &ae->devs[i];
		if (dev->id == 0) {
			continue;
		}
		json_t *jdev = json_object();
		json_object_set_new(jdev,	"mac",			json_integer(dev->id));
		json_object_set_new(jdev,	"type",			json_string(specific2str(dev->generic, dev->specific)));
		//json_object_set_new(jdev,	"version",	json_string(json_get_string(jattrs, "version")));
		json_object_set_new(jdev,	"model",		json_string(generic2str(dev->generic)));
		json_object_set_new(jdev,	"online",		json_integer(dev->online));
		json_object_set_new(jdev,	"battery",	json_integer(100));
		//json_object_set_new(jdev, "basic",    json_integer(dev->basic&0xff));
		//json_object_set_new(jdev, "generic",  json_integer(dev->generic&0xff));
		//json_object_set_new(jdev, "specific", json_integer(dev->specific&0xff));
	

		log_debug("dev %d", dev->id);
		log_debug_hex("class:", dev->class, dev->clen);
		json_t *jattrs = json_object();
		if (jattrs != NULL) {
			emClass_t class[32];
			int j;
			for (j = 0; j < dev->clen; j++) {
				class[j] = dev->class[j];
			}
			class_cmd_to_attrs(dev->id, class, dev->clen, jattrs);
			
			const char *version = json_get_string(jattrs, "version");
			if (version != NULL) {
				json_object_set_new(jdev,	"version",	json_string(version));
				json_object_del(jattrs, "version");
			}

			const char *battery = json_get_string(jattrs, "battery");
			if (battery != NULL) {
				int battery_value;
				sscanf(battery, "%d", &battery_value);
				json_object_set_new(jdev,	"battery",	json_integer(battery_value));
			} else {
				json_object_set_new(jdev,	"battery",	json_integer(100));
			}
	
	
			json_object_set_new(jdev, "attrs", jattrs);
		}

		json_array_append_new(jdevs, jdev);
	}

	char *jdevs_str = json_dumps(jdevs, 0);
	if (jdevs_str != NULL) {
		log_debug("devss is [%s]", jdevs_str);
		free(jdevs_str);
	}

	json_decref(jdevs);
}
void do_cmd_include(char *argv[], int argc) {
	log_debug("not implement!");
}
void do_cmd_exclude(char *argv[], int argc) {
	log_debug("not implement!");
}

void do_cmd_help(char *argv[], int argc) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		log_debug("%-12s\t-\t%s", cmds[i].name, cmds[i].desc);
	}
}

void do_cmd_get(char *argv[], int argc) {
	int did, cid, aid;
	if (argc != 4) {
		log_debug("error argments!");
		return;
	}
	if (sscanf(argv[1], "0x%02x", &did) != 1) {
		log_debug("error argments!");
		return;
	}
	if (sscanf(argv[2], "0x%02x", &cid) != 1) {
		log_debug("error argments!");
		return;
	}
	if (sscanf(argv[3], "0x%02x", &aid) != 1) {
		log_debug("error argments!");
		return;
	}
	
	log_debug("get did:%02x,cid:%02x,aid:%02x", did&0xff, cid&0xff, aid&0xff);
	app_util_push_cmd(E_COMMAND, NULL, 0);
	class_cmd_get_attr(did, cid, aid, argv+4, argc-4);
}

void do_cmd_set(char *argv[], int argc) {
	int did, cid, aid;
	if (argc != 5) {
		log_debug("error argments!");
		return;
	}
	if (sscanf(argv[1], "0x%02x", &did) != 1) {
		log_debug("error argments!");
		return;
	}
	if (sscanf(argv[2], "0x%02x", &cid) != 1) {
		log_debug("error argments!");
		return;
	}
	if (sscanf(argv[3], "0x%02x", &aid) != 1) {
		log_debug("error argments!");
		return;
	}
	
	log_debug("get did:%02x,cid:%02x,aid:%02x", did&0xff, cid&0xff, aid&0xff);

	app_util_push_cmd(E_COMMAND, NULL, 0);
	class_cmd_set_attr(did, cid, aid, argv+4, argc-4);
}


void do_cmd_info(char *argv[], int argc) {
	stAppEnv_t *ae = app_util_getae();
	json_t *jinfo = json_object();
	if (jinfo != NULL) {
		char buf[32];
		sprintf(buf, "%08X", ae->id.HomeID);
		json_object_set_new(jinfo, "HomeID",		json_string(buf));

		sprintf(buf, "%02X", ae->id.NodeID);
		json_object_set_new(jinfo, "NodeID",		json_string(buf));

		sprintf(buf, "%02X", ae->sucid.SUCNodeID);
		json_object_set_new(jinfo, "SUCNOdeID", json_string(buf));

		char *jinfo_str = json_dumps(jinfo, 0);
		if (jinfo_str != NULL) {
				log_debug("jinfo [%s]",jinfo_str);
				free(jinfo_str);
		}
		json_decref(jinfo);
	}
}



