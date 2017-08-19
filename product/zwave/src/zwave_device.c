#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "zwave_device_storage.h"


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
	memcpy(zc->data, data, len);
	return 0;
}



stZWaveClass_t *device_get_class(stZWaveDevice_t *zd, char epid, char classid) {
	stZWaveEndPoint_t *ep = NULL;
	
	if (epid == 0) {
		ep = &zd->root;
	} else {
		ep = device_get_subep(zd, ep);
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
