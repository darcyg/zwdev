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

static void *wait_action_set(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_set(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_get(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_get(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_online_check(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_online_check(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_over(stStateMachine_t *sm, stEvent_t *event, void *acret);


static stStateMachine_t smApp = {
	1,  S_IDLEING, S_IDLEING, {
		{S_IDLEING, 6, NULL, {
				{aE_INIT,					wait_action_init,						wait_transition_init},
				{aE_CLASS,					wait_action_class,					wait_transition_class},
				{aE_ATTR,					wait_action_attr,						wait_transition_attr},
				{aE_INCLUDE,				wait_action_include,				wait_transition_include},
				{aE_EXCLUDE,				wait_action_exclude,				wait_transition_exclude},
				{aE_SET,						wait_action_set,						wait_transition_set},
				{aE_GET,						wait_action_get,						wait_transition_get},
				{aE_ONLINE_CHECK,	wait_action_online_check,		wait_transition_online_check},
			},
		},
		{S_WORKING, 6, NULL, {
				{aE_OVER,					wait_action_over,						wait_transition_over},
			},
		},
	},
};

static stAppEnv_t ae;

int app_init(void *_th, void *_fet) {
	ae.th = _th;
	ae.fet = _fet;
	
	timer_init(&ae.step_timer, app_run);
	timer_init(&ae.online_timer, app_online_check);
	lockqueue_init(&ae.eq);

	ae.inventory.dev_num = 0;
	memset(ae.inventory.devs, 0, sizeof(ae.inventory.devs));

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
#if 0
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
#endif

	timer_set(ae.th, &ae.online_timer, 1000 * 15);
}
void app_run(struct timer *timer) {
	stEvent_t *e;
	if (lockqueue_pop(&ae.eq, (void**)&e) && e != NULL) {
		state_machine_step(&smApp, e);
		FREE(e);
		app_step();
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
	lockqueue_push(&ae.eq, e);
	app_step();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void *wait_action_init(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	api_call(CmdZWaveGetVersion, NULL, 0);
	api_call(CmdSerialApiGetInitData, NULL, 0);
	api_call(CmdSerialApiGetCapabilities, NULL, 0);
	api_call(CmdMemoryGetId, NULL, 0);
	api_call(CmdZWaveGetSucNodeId, NULL, 0);
	return NULL;
}
static int		wait_transition_init(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return S_WORKING;
}




static void *wait_action_class(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	
	stInventory_t *inv = app_get_inventory();
	
	int flag = 0;

	int i = 0;
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		}

		if (id == 0x01) {
			continue;
		}
		inv->devs[id].id = id;

		stNodeInfoIn_t nii = {id};
		api_call(CmdZWaveRequestNodeInfo, (stParam_t*)&nii, sizeof(nii));
		flag++;
	}
	return flag;
}
static int		wait_transition_class(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if (flag > 0)  
		return S_WORKING;
	return S_IDLEING;
}




static void *wait_action_attr(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);

	stInventory_t *inv = app_get_inventory();

	int flag = 0;
	int i = 0;
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		}

		if (id == 0x01) {
			continue;
		}

		log_debug_hex("devs[id].class:", inv->devs[id].class, inv->devs[id].clen);
		int j;
		for (j = 0; j < inv->devs[id].clen; j++) {
			int cid = inv->devs[id].class[j]&0xff;

			stClass_t *cls = &classes[cid];
			if ((class->cid&0xff) != cid) {
				continue;
			}
			if (class->attrs_cnt == 0) {
				continue;
			}
	
			int j = 0;
			for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
				stAttr_t *attr = &cls->attrs[j];
				if (attr->name == NULL || attr->name[0] == 0) {
					continue;
				}
				if (attr->usenick == 0 || attr->nick[0] == 0) {
					continue;
				}

				if (attr->get != NULL) {
					stSendDataIn_t sdi;
					int len = 0;
					attr->get(id, cls->cid, attr->aid, NULL, 0, &sdi, &len);
					api_call(CmdZWaveSendData, (stParam_t*)&sid, len);
					flag++;
				}
			}
		}
	}

	return NULL;
}
static int		wait_transition_attr(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if (flag > 0) 
		return S_WORKING;
	return S_IDLEING;	
}




static void *wait_action_include(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAddNodeToNetworkIn_t antni = {0x81, 0x02, 0x00, 0x00};
	api_call(CmdZWaveAddNodeToNetwork, (stParam_t*)&antni, sizeof(antni));
	return NULL;
}
static int		wait_transition_include(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return S_WORKING;
}


static void *wait_action_exclude(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	int did = *(int*)cmd->param;
	

	stRemoveNodeFromNetworkIn_t rnfn = {0x01, did&0xff};
	api_call(CmdZWaveRemoveNodeFromNetwork, (stParam_t*)&rnfn, sizeof(rnfn));
	return NULL;
}
static int		wait_transition_exclude(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return S_WORKING;
}

static void *wait_action_set(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	
	stInventory_t *inv = app_get_inventory();

	stSetParam_t *p = cmd->param;

	int flag = 0;
	
	for (j = 0; j < inv->devs[p->did].clen; j++) {
		int cid = inv->devs[p->did].class[j]&0xff;

		stClass_t *cls = &classes[cid];
		if ((class->cid&0xff) != cid) {
			continue;
		}
		if (class->attrs_cnt == 0) {
			continue;
		}

		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &cls->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				continue;
			}
			if (attr->usenick == 0 || attr->nick[0] == 0) {
				continue;
			}
			if (strcmp(attr->nick, p->attr) != 0) {
				continue;
			}

			if (attr->set != NULL) {
				stSendDataIn_t sdi;
				int len = 0;
				char argv[1] = {p->value};
				attr->set(p->did, cls->cid, attr->aid, argv, 1, &sdi, &len);
				api_call(CmdZWaveSendData, (stParam_t*)&sid, len);
			}
		}
	}

	return NULL;
}
static int		wait_transition_set(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if (flag > 0) 
		return S_WORKING;
	return S_IDLEING;
}

static void *wait_action_get(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	
	stInventory_t *inv = app_get_inventory();

	stSetParam_t *p = cmd->param;

	int flag = 0;
	
	for (j = 0; j < inv->devs[p->did].clen; j++) {
		int cid = inv->devs[p->did].class[j]&0xff;

		stClass_t *cls = &classes[cid];
		if ((class->cid&0xff) != cid) {
			continue;
		}
		if (class->attrs_cnt == 0) {
			continue;
		}

		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &cls->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				continue;
			}
			if (attr->usenick == 0 || attr->nick[0] == 0) {
				continue;
			}
			if (strcmp(attr->nick, p->attr) != 0) {
				continue;
			}

			if (attr->set != NULL) {
				stSendDataIn_t sdi;
				int len = 0;
				char argv[1] = {p->value};
				attr->set(p->did, cls->cid, attr->aid, argv, 1, &sdi, &len);
				api_call(CmdZWaveSendData, (stParam_t*)&sid, len);
			}
		}
	}


	return NULL;
}
static int		wait_transition_get(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if (flag > 0) 
		return S_WORKING;
	return S_IDLEING;
}

static void *wait_action_online_check(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_online_check(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return S_INITTING;
}

static void *wait_action_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return S_IDLEING;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/*
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
*/
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

int	app_zinit() {
	app_push(E_INIT, NULL, 0);
	return 0;
}
int	app_zclass() {
	app_push(E_CLASS, NULL, 0);
	return 0;
}
int	app_zattr() {
	app_push(E_ATTR, NULL, 0);
	return 0;
}

json_t *	app_zlist() {
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

int	app_zinclude() {
	app_push(E_INCLUDE, NULL, 0);
	return 0;
}

int	app_zexclude(int did) {
	app_push(E_EXCLUDE, &did, sizeof(did));
	return 0;
}

int	app_zclass_cmd_set(int did, char *attr, char *value) {
	int aLen = strlen(attr);
	int vLen = strlen(value);
	stSetParam_t *p = (stSetParam_t*)MALLOC(aLen + vLen + 2);

	p->attr = p;
	p->value = p + aLen + 1;
	p->did = did;
	strcpy(p->attr, attr);
	strcpy(p->value, value);

	app_push(E_SET, p, aLen + vLen + 2);
	FREE(p);
	return 0;
}

int	app_zclass_cmd_get(int did, char *attr, char *value) {
	int aLen = strlen(attr);
	int vLen = strlen(value);
	stGetParam_t *p = (stGetParam_t*)MALLOC(aLen + vLen + 2);

	p->attr = p;
	p->value = p + aLen + 1;
	p->did = did;
	strcpy(p->attr, attr);
	strcpy(p->value, value);

	app_push(E_GET, p, aLen + vLen + 2);
	FREE(p);
	return 0;
}

int	app_zclass_cmd_rpt(int did, int cid, int aid, char *value, int value_lep) {
	log_debug("[%s] ----->%02x, %02x", __func__, did&0xff, cid&0xff);
	log_debug_hex("value:", value, value_len);

	stClass_t *class = &classes[cid&0xff];
	if ((class->cid&0xff) != (cid&0xff)) {
		return;
	}

	if (class->attrs_cnt == 0) {
		return;
	}

	stAttr_t *attr = &class->attrs[op&0xff];
	if (attr->name == NULL || attr->name[0] == 0) {
		return;
	}
	if ((attr->aid&0xff) != op) {
		return;
	}

	char buf[32] = {0};
	class_cmd_rpt_attr(did, cid, aid, buf, value, value_len);
	if (strcmp(buf, "") != 0) {
		memory_set_attr(did, attr->name, value_buf);
	}
}

stInventory_t *app_get_inventory() {
	return &ae.inventory;
}



