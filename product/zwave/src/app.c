#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "log.h"

#include "lockqueue.h"
#include "statemachine.h"
#include "app.h"
#include "api.h"
#include "frame.h"
#include "classcmd.h"


static void *wait_action_init(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_init(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_class(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_class(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_attr(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_attr(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_include(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_include(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_exclude(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_exclude(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_command(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_command(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_init_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_init_over(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_sub_class(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_sub_class(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void *wait_action_class_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_class_over(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void *wait_action_sub_attr(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_sub_attr(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void *wait_action_attr_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_attr_over(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_include_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_include_over(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_exclude_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_exclude_over(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_sub_command(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_sub_command(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_command_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_command_over(stStateMachine_t *sm, stEvent_t *event, void *acret);



static stStateMachine_t smApp = {
	1,  S_IDLEING, S_IDLEING, {
		{S_IDLEING, 6, NULL, {
				{E_INIT,		wait_action_init,			wait_transition_init},
				{E_CLASS,		wait_action_class,		wait_transition_class},
				{E_ATTR,		wait_action_attr,			wait_transition_attr},
				{E_INCLUDE, wait_action_include,	wait_transition_include},
				{E_EXCLUDE, wait_action_exclude,	wait_transition_exclude},
				{E_COMMAND, wait_action_command,  wait_transition_command},
			},
		},
	},
};

static stStateMachine_t	smCommand = {
	2, S_IDLEING, S_IDLEING, {
		{S_IDLEING, 1, NULL, {
				{E_SUB_COMMAND, wait_action_sub_command, wait_transition_sub_command},
			},
		},
		{S_COMMANDING, 1, NULL, {
				{E_COMMAND_OVER, wait_action_command_over, wait_transition_command_over},
			},
		},
	},
};



static stStateMachine_t	smInit = {
	2, S_INITTING, S_INITTING, {
		{S_IDLEING, 0, NULL, {
			},
		},
		{S_INITTING, 1, NULL, {
				{E_INIT_OVER, wait_action_init_over, wait_transition_init_over},
			},
		},
	},
};

static stStateMachine_t smClass = {
	2, S_IDLEING, S_IDLEING, {
		{S_IDLEING, 1, NULL, {
				{E_SUB_CLASS, wait_action_sub_class, wait_transition_sub_class},
			},
		},
		{S_CLASSING, 1, NULL, {
				{E_CLASS_OVER, wait_action_class_over, wait_transition_class_over},
			},
		},
	},
};

static stStateMachine_t smAttr = {
	2, S_IDLEING, S_IDLEING, {
		{S_IDLEING, 1, NULL, {
				{E_SUB_ATTR, wait_action_sub_attr, wait_transition_sub_attr}, 
			},
		},
		{S_ATTRING, 1, NULL, {
				{E_ATTR_OVER, wait_action_attr_over, wait_transition_attr_over},
			},
		},
	}
};

static stStateMachine_t smInclude = {
	2, S_INCLUDING, S_INCLUDING, {
		{S_IDLEING, 0, NULL, {
			},
		},
		{S_INCLUDING, 1, NULL, {
				{E_INCLUDE_OVER, wait_action_include_over, wait_transition_include_over},
			},
		},
	},
};

static stStateMachine_t smExclude = {
	2, S_EXCLUDING, S_EXCLUDING, {
		{S_IDLEING, 0, NULL, {
			},
		},
		{S_EXCLUDING, 1, NULL, {
				{E_EXCLUDE_OVER, wait_action_exclude_over, wait_transition_exclude_over},
			},
		},
	},
};


static stAppEnv_t ae = {
	.subsm = NULL,
};

int app_init(void *_th, void *_fet) {
	ae.th = _th;
	ae.fet = _fet;
	
	timer_init(&ae.step_timer, app_run);
	timer_init(&ae.online_timer, app_online_check);
	lockqueue_init(&ae.cmdq);
	lockqueue_init(&ae.subcmdq);
	lockqueue_init(&ae.msgq);

	ae.dev_num = 0;
	memset(ae.devs, 0, sizeof(ae.devs));


	state_machine_reset(&smApp);

	timer_set(ae.th, &ae.online_timer, 1000 * 60);
	return 0;
}

int app_step() {
	timer_cancel(ae.th, &ae.step_timer);
	timer_set(ae.th, &ae.step_timer, 10);
	return 0;
}


void app_in(void *arg, int fd) {
}

void app_online_check(struct timer *timer) {
	int i = 0;
	for (i = 0; i < sizeof(ae.devs)/sizeof(ae.devs[0]); i++) {
		stDevice_t *dev = &ae.devs[i];
		if (dev->id == 0) {
			continue;
		}

		int j = 0;
		int fBattery = 0;
		for (j = 0; j < dev->clen; j++) {
			if (dev->class[j] == 0x80) {
				fBattery = 1;
				break;
			}
		}

		long curr = time(NULL);
		if (curr - dev->lasttime > 15 * 60) {
			if (dev->online != 0) {
				dev->online = 0;
				log_debug("dev %d online status change: offline", dev->id);
			}
		}

		/*
		if (dev->online != 0 && fBattery != 1) {
			if (curr - dev->lasttime > (dev->online_checknum + 1) * 5) {
				if (state_machine_get_state(&smApp) == S_IDLEING) {
					app_util_push_cmd(E_ATTR, NULL, 0);
					class_cmd_get_attr(dev->id, COMMAND_CLASS_BASIC_V1, BASIC, NULL, 0);
					dev->online_checknum++;
					break;
				}
			}
		}
		*/
	}

	timer_set(ae.th, &ae.online_timer, 1000 * 15);
}
void app_run(struct timer *timer) {
	stEvent_t *e;
	if (lockqueue_pop(&ae.msgq, (void**)&e) && e != NULL) {
		if (ae.subsm != NULL) {
			state_machine_step(ae.subsm, e);
		} else {
			state_machine_step(&smApp, e);
		}
		FREE(e);
		app_step();
		return;
	}

	if (ae.subsm != NULL) {	
		int s = state_machine_get_state(ae.subsm);
		if (s != S_IDLEING) {	
			return;
		}
		if (lockqueue_pop(&ae.subcmdq, (void**)&e) && e != NULL) {
			state_machine_step(ae.subsm, e);
			FREE(e);
			app_step();
			return;
		}
		ae.subsm = NULL;
		state_machine_set_state(&smApp, S_IDLEING);
		app_step();
		return;
	} else {
		if (lockqueue_pop(&ae.cmdq, (void**)&e) && e != NULL) {
			state_machine_step(&smApp, e);
			FREE(e);
			app_step();
			return;
		}
	}
}

void app_push(int eid, void *param, int len) {
	stEvent_t *e = MALLOC(sizeof(stEvent_t) + sizeof(stAppCmd_t) + len);
	if (e == NULL) {
		return;
	}
	e->eid = eid;
	e->param = e+1;
	stAppCmd_t *cmd = (stAppCmd_t*)e->param;

	if (len > 0) {
		cmd->len = len;
		cmd->param = cmd + 1;

		memcpy(cmd->param, param, len);
	} else {
		cmd->len = 0;
		cmd->param = 0;
	}
	lockqueue_push(&ae.subcmdq, e);
	app_step();
}

void app_util_push_cmd(int eid, void *param, int len) {
	stEvent_t *e = MALLOC(sizeof(stEvent_t) + sizeof(stAppCmd_t) + len);
	if (e == NULL) {
		return;
	}
	e->eid = eid;
	e->param = e+1;
	stAppCmd_t *cmd = (stAppCmd_t*)e->param;

	if (len > 0) {
		cmd->len = len;
		cmd->param = cmd + 1;

		memcpy(cmd->param, param, len);
	} else {
		cmd->len = 0;
		cmd->param = 0;
	}
	lockqueue_push(&ae.cmdq, e);
	app_step();
}

void app_util_push_msg(int eid, void *param, int len) {
	stEvent_t *e = MALLOC(sizeof(stEvent_t) + sizeof(stAppCmd_t) + len);
	if (e == NULL) {
		return;
	}
	e->eid = eid;
	e->param = e+1;
	stAppCmd_t *cmd = (stAppCmd_t*)e->param;

	if (len > 0) {
		cmd->len = len;
		cmd->param = cmd + 1;

		memcpy(cmd->param, param, len);
	} else {
		cmd->len = 0;
		cmd->param = 0;
	}
	
	lockqueue_push(&ae.msgq, e);
	app_step();
}


stAppEnv_t *app_util_getae() {
	return &ae;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void *wait_action_init(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	api_call(CmdZWaveGetVersion, NULL, 0);
	api_call(CmdSerialApiGetInitData, NULL, 0);
	api_call(CmdSerialApiGetCapabilities, NULL, 0);
	api_call(CmdMemoryGetId, NULL, 0);
	api_call(CmdZWaveGetSucNodeId, NULL, 0);

	//stNodeProtoInfoIn_t npii = { 0x01};
	//api_call(CmdZWaveGetNodeProtoInfo, (stParam_t*)&npii, sizeof(stNodeProtoInfoIn_t));
	/*
	api_call(CmdZWaveGetProtocolVersion, NULL, 0);
	api_call(CmdZWaveRfPowerLevelGet, NULL, 0);
	api_call(CmdZWaveTypeLibrary, NULL, 0);
	api_call(CmdZWaveGetProtocolStatus, NULL, 0);
	*/

	return NULL;
}
static int		wait_transition_init(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	ae.subsm = &smInit;
	state_machine_reset(ae.subsm);
	return S_INITTING;
}

static void *wait_action_class(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	
	int i = 0;
	for (i = 0; i < ae.initdata.nodes_map_size * 8; i++) {
		int id			= i+1;
		int id_bit	= (ae.initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		}
		if (id == 0x01) {
			continue;
		}
		ae.devs[id].id = id;
		ae.lastid = id;

		stNodeInfoIn_t nii = {id};
		app_push(E_SUB_CLASS, &nii, sizeof(nii));
	}
	return NULL;
}
static int		wait_transition_class(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	ae.subsm = &smClass;
	state_machine_reset(ae.subsm);
	return S_CLASSING;
}


static void *wait_action_attr(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	int i = 0;
	for (i = 0; i < ae.initdata.nodes_map_size * 8; i++) {
		int id			= i+1;
		int id_bit	= (ae.initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		}
		if (id == 0x01) {
			continue;
		}

		log_debug_hex("ae.devs[id].class:", ae.devs[id].class, ae.devs[id].clen);
		emClass_t class[32];
		int j;
		for (j = 0; j < ae.devs[id].clen; j++) {
			class[j] = ae.devs[id].class[j];
		}

		class_cmd_get_dev_attr(id, class, ae.devs[id].clen);
	}

	return NULL;
}
static int		wait_transition_attr(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	ae.subsm = &smAttr;
	state_machine_reset(ae.subsm);
	return S_ATTRING;
}

static void *wait_action_include(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	//api_call(CmdZWaveGetSucNodeId, NULL, 0);
	return NULL;
}
static int		wait_transition_include(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	ae.subsm = &smInclude;
	state_machine_reset(ae.subsm);
	return S_INCLUDING;
}

static void *wait_action_exclude(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	//api_call(CmdZWaveIn, NULL, 0);
	return NULL;
}
static int		wait_transition_exclude(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	ae.subsm = &smExclude;
	state_machine_reset(ae.subsm);
	return S_EXCLUDING;
}

static void *wait_action_command(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_command(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	ae.subsm = &smCommand;
	state_machine_reset(ae.subsm);
	return S_COMMANDING;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
static void *wait_action_init_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	
	api_app_init_over_fetch(&ae);
	

	log_debug("version---->ver:%s, type:%02x", ae.ver.ver, ae.ver.type);

	log_debug("init data---->:");
	log_debug("ver:%02x, capabilities:%02x,nodes_map_size:%02x, chip_type:%02x, chip_version:%02x",
						ae.initdata.ver, ae.initdata.capabilities, ae.initdata.nodes_map_size,
						ae.initdata.chip_type, ae.initdata.chip_version);
	log_debug_hex("nodes_map:",ae.initdata.nodes_map, ae.initdata.nodes_map_size);

	log_debug("Capabilities--->:");
	log_debug("AppVersion:%02x, AppRevisioin:%02x, ManufacturerId:%04x, ManufactureProductType:%04x, ManufactureProductId:%04x",
						ae.caps.AppVersion, ae.caps.AppRevisioin, ae.caps.ManufacturerId, 
						ae.caps.ManufactureProductType, ae.caps.ManufactureProductId);
	log_debug("Id-->:");
	log_debug("HomeId:%08X, NodeId:%02x", ae.id.HomeID, ae.id.NodeID);
	log_debug("SucId--->:sudid:%02x", ae.sucid.SUCNodeID);

	return NULL;

}
static int		wait_transition_init_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	app_step();
	return S_IDLEING;
}

static void *wait_action_sub_class(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	stNodeInfoIn_t *nii = cmd->param;
	api_call(CmdZWaveRequestNodeInfo, (stParam_t*)nii, sizeof(*nii));
	return NULL;
}
static int		wait_transition_sub_class(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	app_step();
	return (int)S_CLASSING;
}
static void *wait_action_class_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	stNodeInfo_t *ni = cmd->param;

	int id = ni->bNodeID&0xff;
	ae.devs[id].id = id;
	ae.devs[id].basic = ni->basic;
	ae.devs[id].generic = ni->generic;
	ae.devs[id].specific = ni->specific;
	ae.devs[id].clen = ni->len - 3;
	ae.devs[id].lasttime = time(NULL);
	ae.devs[id].online = 1;
	ae.devs[id].online_checknum = 0;
	memcpy(ae.devs[id].class, ni->commandclasses, ae.devs[id].clen);

	return NULL;

}
static int		wait_transition_class_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	app_step();
	return (int)S_IDLEING;
}



static void *wait_action_sub_attr(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);

	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	stSendDataIn_t *sdi = cmd->param;
	api_call(CmdZWaveSendData, (stParam_t*)sdi,  cmd->len);

	return NULL;
}
static int		wait_transition_sub_attr(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	app_step();
	return S_ATTRING;
}
static void *wait_action_attr_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);

	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	if (cmd != NULL && cmd->param != NULL) {
		char src = ((char *)cmd->param)[0];
		int did = src&0xff;
		int i;
		for (i = 0; i < sizeof(ae.devs)/sizeof(ae.devs[0]); i++) {
			stDevice_t *dev = &ae.devs[i];
			if (dev->id == 0 || dev->id != did) {
				continue;
			}
			dev->lasttime = time(NULL);
			if (dev->online == 0) {
				dev->online = 1;
				log_debug("dev %d online status change: online", dev->id);
			}
		}
	}
	
	return NULL;
}
static int		wait_transition_attr_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	app_step();
	return (int)S_IDLEING;
}


static void *wait_action_include_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_include_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return 0;
}

static void *wait_action_exclude_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_exclude_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return 0;
}

static void *wait_action_sub_command(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	stSendDataIn_t *sdi = cmd->param;
	api_call(CmdZWaveSendData, (stParam_t*)sdi, cmd->len);
	return NULL;
}

static int		wait_transition_sub_command(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return S_COMMANDING;
}


static void *wait_action_command_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_command_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	app_step();
	return (int)S_IDLEING;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
static int app_class_cmd_to_attrs(int did, emClass_t *class_array, int class_cnt, void *jattrs) {
	int i = 0;
	for (i = 0; i < class_cnt; i++) {
		int cid = class_array[i]&0xff;
		stClass_t *class = &classes[cid];

		if ((class->cid&0xff) != cid) {
			continue;
		}
	
		if (class->attrs_cnt == 0) {
			continue;
		}
	
		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &class->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				continue;
			}
			if (attr->usenick == 0 || attr->nick[0] == 0) {
				continue;
			}

			char value[32];
			if (memory_get_attr(did, attr->name, value) == 0)  {
				json_object_set_new(jattrs, attr->nick, json_string(value));
			} else if (flash_load_attr(did, attr->name, value) == 0) {
				memory_set_attr(did, attr->name, value);
				json_object_set_new(jattrs, attr->nick, json_string(value));
			} else {
				if (attr->get != NULL) {
					app_class_cmd_get(did, attr->nick, "");
				}
				json_object_set_new(jattrs, attr->nick, json_string(""));
			}
		}
	}
	return 0;
}

json_t *	app_list() {
	json_t *jdevs = json_array();

	int i = 0;
	for (i = 0; i < sizeof(ae.devs)/sizeof(ae.devs[0]); i++) {
		stDevice_t *dev = &ae.devs[i];
		if (dev->id == 0) {
			continue;
		}

		json_t *jdev = json_object();
		json_object_set_new(jdev,	"mac",			json_integer(dev->id));
		json_object_set_new(jdev,	"type",			json_string(specific2str(dev->generic, dev->specific)));
		json_object_set_new(jdev,	"model",		json_string(generic2str(dev->generic)));
		json_object_set_new(jdev,	"online",		json_integer(dev->online));
		json_object_set_new(jdev,	"version",	json_string("unknow"));
		json_object_set_new(jdev,	"battery",	json_integer(100));
	
		log_debug("dev %d", dev->id);
		log_debug_hex("class:", dev->class, dev->clen);

		json_t *jattrs = json_object();
		if (jattrs != NULL) {
			emClass_t class[32];
			int j;
			for (j = 0; j < dev->clen; j++) {
				class[j] = dev->class[j];
			}
			app_class_cmd_to_attrs(dev->id, class, dev->clen, jattrs);
			
			const char *version = json_get_string(jattrs, "version");
			if (version != NULL) {
				json_object_del(jdev, "version");
				json_object_set_new(jdev,	"version",	json_string(version));

				json_object_del(jattrs, "version");
			}

			const char *battery = json_get_string(jattrs, "battery");
			if (battery != NULL) {
				int battery_value;
				sscanf(battery, "%d", &battery_value);

				json_object_del(jdev, "battery");
				json_object_set_new(jdev,	"battery",	json_integer(battery_value));

				json_object_del(jattrs, "battery");
			} 

			const char *mac = json_get_string(jattrs, "mac");
			if (mac != NULL) {
				json_object_del(jdev, "mac");
				json_object_set_new(jdev,	"mac",	json_string(mac));

				json_object_del(jattrs, "mac");
			}
	
	
			json_object_set_new(jdev, "attrs", jattrs);
		}

		json_array_append_new(jdevs, jdev);
	}

	return jdevs;
}

int				app_include() {
}

int				app_exclude() {
}

int				app_class_cmd_set(int did, char *attr, char *value) {
}

int				app_class_cmd_get(int did, char *attr, char *value) {
}



