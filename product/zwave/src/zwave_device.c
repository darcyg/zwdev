#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "hex.h"

#include "zwave_device_storage.h"
#include "zwave_class_cmd.h"



stZWaveCache_t zc;

static stZWaveDevice_t* device_malloc() {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 1) {
			continue;
		}
	
		memset(&devs[i], 0, sizeof(devs[i]));
		devs[i].nbit = -1;
		devs[i].nbit_subeps_head = -1;
		devs[i].used = 1;
		return &devs[i];
	}
	return NULL;
}
static void device_free(stZWaveDevice_t* zd) {
	zd->used = 0;
}

static void device_clear_cmd(stZWaveCommand_t *cmd) {
	cmd->cmdid = -1;
}

static void device_clear_class(stZWaveClass_t *class) {
	class->classid = -1;
	if (class->cmdcnt > 0) {
		int i = 0;
		for (i = 0; i < class->cmdcnt; i++) {
			device_clear_cmd(&class->cmds[i]);
		}
		free(class->cmds);
		class->cmds =NULL;
		class->cmdcnt = 0;
	}
	return ;
}

static void device_clear_endpoint(stZWaveEndPoint_t *ep) {
	ep->ep = -1;
	if (ep->classcnt > 0) {
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			device_clear_class(&ep->classes[i]);
		}
		free(ep->classes);
		ep->classes = NULL;
		ep->classcnt = 0;
	}
}

char *device_get_extaddr(stZWaveDevice_t *zd) {
	char mac[8];

	memset(mac, zd->bNodeID, 8);

	stZWaveClass_t *class = device_get_class(zd, 0, 0x72);
	if (class != NULL && class->version >= 2) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			memcpy(mac, cmd->data + 2, 8);
		} else {
			log_warn("no manufacturer id(no cmd 0x07), use zwave node id as mac!!!");
		}
	} else {
		log_warn("no manufacturer id(no class 0x72), use zwave node id as mac!!!");
	}

	memcpy(zd->mac, mac, 8);

	return zd->mac;
}

int device_add(char bNodeID, char basic, char generic, 
		char specific, char security, char capability,
		int classcnt, char *classes) {
	stZWaveDevice_t *zd = device_get_by_nodeid(bNodeID);
	if (zd != NULL) {
		return 0;
	}
	
	zd = device_malloc();
	if (zd == NULL) {
		return -1;
	}

	zd->bNodeID = bNodeID;
	zd->security = security;
	zd->capability = capability;

	device_fill_ep(&zd->root, 0, basic, generic, specific, classcnt, classes);
	
	return 0;
}

int device_del(char bNodeID) {
	stZWaveDevice_t *zd = device_get_by_nodeid(bNodeID);
	if (zd == NULL) {
		return 0;
	}
	
	device_clear_endpoint(&zd->root);
	if (zd->subepcnt > 0) {
		int i = 0;
		for (i = 0; i < zd->subepcnt; i++) {
			device_clear_endpoint(&zd->subeps[i]);
		}
		free(zd->subeps);
		zd->subeps = NULL;
		zd->subepcnt = 0;
	}

	device_free(zd);

	return 0;
}

stZWaveDevice_t *device_get_by_nodeid(char bNodeID) {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}
	
		if (devs[i].bNodeID == bNodeID) {
			return &devs[i];
		}
	}
	
	return NULL;
}

stZWaveDevice_t *device_get_by_extaddr(char extaddr[8]) {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}
	
		if (memcmp(device_get_extaddr(&devs[i]), extaddr, 8) == 0) {
			return &devs[i];
		}
	}
	
	return NULL;
}

int device_add_subeps(stZWaveDevice_t *zd, int epcnt, char *eps) {
	if (epcnt <= 0 || eps == NULL || zd == NULL) {
		return -1;
	}
	
	zd->subepcnt = epcnt;
	int i = 0;
	zd->subeps = (stZWaveEndPoint_t *)malloc(sizeof(stZWaveEndPoint_t)*zd->subepcnt);
	for (i = 0; i < epcnt; i++) {
		memset(&zd->subeps[i], 0, sizeof(stZWaveEndPoint_t));
		zd->subeps[i].ep = eps[i]&0xff;
		zd->subeps[i].nbit = -1;
		zd->subeps[i].nbit_next = -1;
		zd->subeps[i].nbit_classes_head = -1;
	}
	
	return 0;
}

int device_fill_ep(stZWaveEndPoint_t *ze, char ep, char basic, char generic,
		char specific, int classcnt, char *classes) {
	ze->ep = 0;
	ze->basic = basic;
	ze->generic = generic;
	ze->specific = specific;
	ze->classcnt = 0;
	ze->classes = NULL;
	if (classcnt > 0) {
		ze->classcnt = classcnt;
		ze->classes = (stZWaveClass_t*)malloc(sizeof(stZWaveClass_t)*ze->classcnt);
		int i = 0;
		for (i = 0; i < ze->classcnt; i++) {
			ze->classes[i].classid = classes[i]&0xff;
			ze->classes[i].version = -1;
			ze->classes[i].cmdcnt = 0;
			ze->classes[i].cmds = NULL;
			ze->classes[i].nbit = -1;
			ze->classes[i].nbit_next = -1;
			ze->classes[i].nbit_cmds_head = -1;

			/*
			stZWClass_t *c = zcc_get_class(ze->classes[i].classid, ze->classes[i].version);
			char cmds[MAX_CMD_NUM];
			int cmdcnt = zcc_get_class_cmd_rpt(c, cmds);
			if (cmdcnt > 0) {
				device_add_cmds(&ze->classes[i], cmdcnt, cmds);
			}
			*/
		}
	} 

	return 0;
}

stZWaveEndPoint_t *device_get_subep(stZWaveDevice_t *zd, char ep) {
	int i;	
	for (i = 0; i < zd->subepcnt; i++) {
		if (zd->subeps[i].ep == ep) {
			return &zd->subeps[i];
		}
	}
	return 0;
}

int device_add_cmds(stZWaveClass_t *class, int cmdcnt, char *cmds) {
	if (class == NULL || cmdcnt <= 0 || cmds == NULL) {
		return -1;
	}
	
	class->cmdcnt = cmdcnt;
	class->cmds = (stZWaveCommand_t*)malloc(sizeof(stZWaveCommand_t)*class->cmdcnt);
	int i = 0;
	for (i = 0; i < class->cmdcnt; i++) {
		memset(&class->cmds[i], 0, sizeof(stZWaveCommand_t));
		class->cmds[i].cmdid = cmds[i]&0xff;
		class->cmds[i].nbit = -1;
		class->cmds[i].nbit_next = -1;
	}

	return 0;
}

stZWaveCommand_t *device_get_cmd(stZWaveClass_t *class, char cmd) {
	int i = 0; 
	for (i = 0; i < class->cmdcnt; i++) {	
		if (class->cmds[i].cmdid == cmd) {
			return &class->cmds[i];
		}
	}

	return NULL;
}

int device_update_cmds_data(stZWaveCommand_t *zc, char *data, int len) {
	len = len > sizeof(zc->data) ? sizeof(zc->data) : len;
	zc->len = len;
	memcpy(zc->data, data, len);
	return 0;
}



stZWaveClass_t *device_get_class(stZWaveDevice_t *zd, char epid, char classid) {
	stZWaveEndPoint_t *ep = NULL;
	
	if (epid == 0) {
		ep = &zd->root;
	} else {
		ep = device_get_subep(zd, epid);
	}

	if (ep == NULL) {
		return NULL;
	}


	int i = 0;
	for (i = 0; i < ep->classcnt; i++) {
		stZWaveClass_t *class = &ep->classes[i];
		if (class->classid == classid) {
			return class;
		}
	}

	return NULL;
}

const char *device_make_macstr(stZWaveDevice_t *zd) {
	char mac[8];

	memset(mac, zd->bNodeID, 8);

	stZWaveClass_t *class = device_get_class(zd, 0, 0x72);
	if (class != NULL && class->version >= 2) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			memcpy(mac, cmd->data + 2, 8);
		} else {
			log_warn("no manufacturer id(no cmd 0x07), use zwave node id as mac!!!");
		}
	}  else {
		log_warn("class:%p, version:%d\n", class, class != NULL ? class->version : -1);
		log_warn("no manufacturer id(no class 0x72), use zwave node id as mac!!!");
	}

	static char macstr[32];
	hex_string(macstr, sizeof(macstr), (u8*)mac, sizeof(mac), 1, 0);

	return macstr;
}
int device_get_battery(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x80);
	if (class != NULL) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x03);
		if (cmd != NULL) {
			char *buf = cmd->data;
			int battery = buf[0]&0xff;
			return battery;
		}
	}
	return 100;
}
int device_get_online(stZWaveDevice_t *zd) {
	int online = zd->online;
	return !!online;
}
const char *device_make_modelstr(stZWaveDevice_t *zd) {
	return "unknow";
}
int device_is_lowpower(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x80);
	if (class != NULL) {
		return 1;
	}
	return 0;
}
const char *device_make_typestr(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x25);
	if (class != NULL) {
		return "1213";
	}
	
	class = device_get_class(zd, 0, 0x71);
	if (class != NULL) {
		return "1209";
	}

	return "unkonw";
}
const char *device_make_versionstr(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x86);
	if (class != NULL) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			char *buf = cmd->data;
			static char version[32];
			sprintf(version, "%02X-%02X.%02X-%02X.%02X", buf[0]&0xff, buf[1]&0xff,buf[2]&0xff, buf[3]&0xff, buf[4]&0xff);
			return version;
		}
	}
	return "unknow";
}


char device_get_nodeid_by_mac(const char *mac) {
	stZWaveDevice_t *zd = device_get_by_extaddr((char *)mac);
	if (zd == NULL) {
		return -1;
	}
	
	return zd->bNodeID;
}

static void device_view_command(stZWaveClass_t *class, stZWaveCommand_t *command) {
	char buf[sizeof(command->data)*3 + 4];
	int i = 0; 
	int len = 0;
	buf[0] = 0;
	for (i = 0; i < command->len; i++) {
		len += sprintf(buf + len, "%02X ", command->data[i]&0xff);
	}
	const char *name = zcc_get_cmd_name(class->classid, class->version, command->cmdid);
	printf("      cmdid:%02X(%s), len: %d, data:[ %s]\n", command->cmdid&0xff, name,  command->len, buf);
}

static void device_view_class(stZWaveClass_t *class) {
	const char *name = zcc_get_class_name(class->classid, class->version);
	printf("    classid:%02X(%s), version:%d\n", class->classid&0xff, name, class->version);
	int i = 0;
	for (i = 0; i < class->cmdcnt; i++) {
		device_view_command(class, &class->cmds[i]);
	}
}

static void device_view_endpoint(stZWaveEndPoint_t *ep) {
	printf("  ep:%02X, basic:%02X, generic: %02X, specific:%02X \n",
					ep->ep&0xff, ep->basic&0xff, ep->generic&0xff, ep->specific&0xff);
	int i = 0;
	for (i = 0; i < ep->classcnt; i++) {
		device_view_class(&ep->classes[i]);
	}
}

static void device_view_device(stZWaveDevice_t *dev) {
	printf("mac:%s, nodeid:%02X, security:%02X, capacility:%02X, online:%02X\n",
						device_make_macstr(dev), dev->bNodeID&0xff, dev->security&0xff, dev->capability&0xff, 
						dev->online);
	device_view_endpoint(&dev->root);
	int i = 0;
	for (i = 0; i < dev->subepcnt; i++) {
		device_view_endpoint(&dev->subeps[i]);
	}
}


void device_view_all() {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}

		device_view_device(&devs[i]);
	}
}

