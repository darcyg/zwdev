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
#include "memory.h"
#include "flash.h"
#include "jansson.h"
#include "json_parser.h"
#include "uproto.h"


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

static void *wait_action_attr_new(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_attr_new(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void *wait_action_fresh_nodemap(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_fresh_nodemap(stStateMachine_t *sm, stEvent_t *event, void *acret);



extern stClass_t classes[];

static stStateMachine_t smApp = {
	2,  aS_IDLEING, aS_IDLEING, {
		{aS_IDLEING, 11, NULL, -1, {
				{aE_INIT,					wait_action_init,						wait_transition_init},
				{aE_CLASS,					wait_action_class,					wait_transition_class},
				{aE_ATTR,					wait_action_attr,						wait_transition_attr},
				{aE_INCLUDE,				wait_action_include,				wait_transition_include},
				{aE_EXCLUDE,				wait_action_exclude,				wait_transition_exclude},
				{aE_SET,						wait_action_set,						wait_transition_set},
				{aE_GET,						wait_action_get,						wait_transition_get},
				{aE_ONLINE_CHECK,	wait_action_online_check,		wait_transition_online_check},
				{aE_ATTR_NEW,			wait_action_attr_new,				wait_transition_attr_new},
				{aE_FRESH_NODEMAP, wait_action_fresh_nodemap, wait_transition_fresh_nodemap},
			},
		},
		{aS_WORKING, 1, NULL, -1, {
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
	lockqueue_init(&ae.eqMsg);

	ae.inventory.dev_num = 0;
	memset(ae.inventory.devs, 0, sizeof(ae.inventory.devs));

	state_machine_reset(&smApp);

	timer_set(ae.th, &ae.online_timer, 1000 * 15);

	memory_init(&ae.inventory.hmattrs);
	flash_init("/etc/config/dusun/zwave");
	
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
#if 1
	stInventory_t *inv = app_get_inventory();
	
	int i = 0;
	int online_check_inteval = 5*60;

	for (i = 0; i < sizeof(inv->devs)/sizeof(inv->devs[0]); i++) {
		stDevice_t *dev = &inv->devs[i];
		if (dev->id == 0) {
			continue;
		}
		if (dev->id == 0x01) {
			continue;
		}

		if (dev->online > 0) {
			dev->online -= online_check_inteval;
			if (dev->online <= 0) {
				dev->online = 0;
				log_debug("dev %d online status change: offline", dev->id);
			} else {
				if (!app_util_is_battery_device(dev->id)) {
					log_debug("send heart data to dev %02x", dev->id&0xff);
					const char *type = class_cmd_specific2str(dev->generic, dev->specific);
					if (strcmp(type, "1212") == 0) {
						log_debug("send on_off get cmd to  dev %02x", dev->id&0xff);
						app_zclass_cmd_get(dev->id, "on_off", "");
					} else {
						log_debug("send generic data send cmd to dev %02x", dev->id&0xff);
						app_zclass_cmd_get(dev->id, "version", "");
					}
				}
			}
		} 

	}
#endif

	timer_set(ae.th, &ae.online_timer, 1000 * 60 * 5);
}
void app_run(struct timer *timer) {
	stEvent_t *e;
	if (state_machine_get_state(&smApp) == aS_IDLEING) {
		if (lockqueue_pop(&ae.eqMsg, (void **)&e) && e != NULL) {
			log_debug("app handler event (idle)-------------------------->e->eid:%d", e->eid);
			state_machine_step(&smApp, e);
			FREE(e);
			app_step();
		}


		if (lockqueue_pop(&ae.eq, (void**)&e) && e != NULL) {
			log_debug("app handler event (idle)-------------------------->e->eid:%d", e->eid);
			state_machine_step(&smApp, e);
			FREE(e);
			app_step();
		}
		
	} else if (state_machine_get_state(&smApp) == aS_WORKING) {
		if (lockqueue_pop(&ae.eqMsg, (void **)&e) && e != NULL) {
			log_debug("app handler event (working)-------------------------->e->eid:%d", e->eid);
			state_machine_step(&smApp, e);
			FREE(e);
			app_step();
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


	lockqueue_push(&ae.eq, e);
	app_step();
}

void app_push_msg(int eid, void *param, int len) {
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
	lockqueue_push(&ae.eqMsg, e);
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
	return aS_WORKING;
}




static void *wait_action_class(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);


	/* read all attr from flash */

	
	stInventory_t *inv = app_get_inventory();
	
#if 0
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
	return (void *)flag;
#else
	int i = 0;
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			flash_remove_device(id);
			continue;
		}
		if (id == 0x01) {
			flash_remove_device(id);
			continue;
		}
		inv->devs[id].id = id;


		char cls[64];
		int cnt;
		char b, g, s;

		if (flash_load_basic_generic_specific(inv->devs[id].id, &b, &g, &s) == 0) {
			inv->devs[id].basic = b;	
			inv->devs[id].generic = g;
			inv->devs[id].specific = s;
		
			inv->devs[id].online = 15 * 60;
		}

		flash_load_class(inv->devs[id].id, cls, &cnt);
		inv->devs[id].clen = cnt;
		if (inv->devs[id].clen > 0) {
			memcpy(inv->devs[id].class, cls, inv->devs[id].clen);
		}
	}
	return 0;
#endif

}
static int		wait_transition_class(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if ((int)acret > 0)  
		return aS_WORKING;
	return aS_IDLEING;
}




static void *wait_action_attr(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);

	stInventory_t *inv = app_get_inventory();

#if 0
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
			if ((cls->cid&0xff) != cid) {
				continue;
			}
			if (cls->attrs_cnt == 0) {
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
					api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
					flag++;
				}
			}
		}
	}
	return (void*)flag;
#else
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
			if ((cls->cid&0xff) != cid) {
				continue;
			}
			if (cls->attrs_cnt == 0) {
				continue;
			}
	
			int j = 0;
			for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
				stAttr_t *attr = &cls->attrs[j];
				if (attr->name == NULL || attr->name[0] == 0) {
					continue;
				}
				if (attr->usenick == 0) {
					continue;
				}
			
				char buf[32];
				if (flash_load_attr(inv->devs[id].id, attr->name, buf) == 0) {
					memory_set_attr(inv->devs[id].id, attr->name, buf);
				}
			}
		}
	}
	return 0;
#endif

}
static int		wait_transition_attr(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if ((int)acret > 0)  
		return aS_WORKING;
	return aS_IDLEING;	
}

static void *wait_action_attr_new(stStateMachine_t *sm, stEvent_t *event) {
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

		if (inv->devs[id].fnew != 1) {
			continue;
		}
		inv->devs[id].fnew = 0;

		log_debug_hex("devs[id].class:", inv->devs[id].class, inv->devs[id].clen);
		int j;
		for (j = 0; j < inv->devs[id].clen; j++) {
			int cid = inv->devs[id].class[j]&0xff;

			stClass_t *cls = &classes[cid];
			if ((cls->cid&0xff) != cid) {
				continue;
			}
			if (cls->attrs_cnt == 0) {
				continue;
			}
	
			int j = 0;
			for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
				stAttr_t *attr = &cls->attrs[j];
				if (attr->name == NULL || attr->name[0] == 0) {
					continue;
				}
				if (attr->usenick == 0) {
					continue;
				}

				if ((cls->cid&0xff) == 0x84 && attr->aid == WAKE_UP_INTERNAL && attr->set != NULL) {
					stSendDataIn_t sdi;
					int len = 0;
					attr->set(id, cls->cid, attr->aid, NULL, 0, &sdi, &len);
					api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
					flag++;
				}
				if ((cls->cid&0xff) == 0x71 && attr->aid == NOTIFICATION && attr->set != NULL) {
					stSendDataIn_t sdi;
					int len = 0;
					attr->set(id, cls->cid, attr->aid, NULL, 0, &sdi, &len);
					api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
					flag++;
				}
				if ((cls->cid&0xff) == 0x85 && attr->aid == ASSOCIATION && attr->set != NULL) {
					stSendDataIn_t sdi;
					int len = 0;
					attr->set(id, cls->cid, attr->aid, NULL, 0, &sdi, &len);
					api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
					flag++;
				}

				if (attr->get != NULL) {
					stSendDataIn_t sdi;
					int len = 0;
					attr->get(id, cls->cid, attr->aid, NULL, 0, &sdi, &len);
					api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
					flag++;
				}
			}
		}
	}

	return (void*)flag;

}
static int		wait_transition_attr_new(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if ((int)acret > 0)  
		return aS_WORKING;
	return aS_IDLEING;	

}



static void *wait_action_fresh_nodemap(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	api_call(CmdSerialApiGetInitData, NULL, 0);
	return NULL;
}
static int		wait_transition_fresh_nodemap(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return aS_WORKING;
}



static void *wait_action_include(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAddNodeToNetworkIn_t antni = {0x81, 0x02, 0x00, 0x00};
	api_call(CmdZWaveAddNodeToNetwork, (stParam_t*)&antni, sizeof(antni));
	return NULL;
}
static int		wait_transition_include(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return aS_WORKING;
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
	return aS_WORKING;
}

static void *wait_action_set(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	
	stInventory_t *inv = app_get_inventory();

	stSetParam_t *p = cmd->param;
	p->attr		= (char *)(p+1);
	p->value	= p->attr + strlen(p->attr) + 1;
	log_debug("p->attr: %s, p->value:%s", p->attr, p->value);

	int flag = 0;
	
	int j;
	for (j = 0; j < inv->devs[p->did].clen; j++) {
		int cid = inv->devs[p->did].class[j]&0xff;

		stClass_t *cls = &classes[cid];
		if ((cls->cid&0xff) != cid) {
			continue;
		}
		if (cls->attrs_cnt == 0) {
			continue;
		}

		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &cls->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				continue;
			}
			if (attr->usenick == 0) {
				continue;
			}
			if (strcmp(attr->nick, p->attr) != 0) {
				continue;
			}

			if (attr->set != NULL) {
				stSendDataIn_t sdi;
				int len = 0;
				char *argv[1] = {p->value};
				attr->set(p->did, cls->cid, attr->aid, argv, 1, &sdi, &len);
				api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
				flag++;
			}
		}
	}

	return (void*)flag;
}
static int		wait_transition_set(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if ((int)acret > 0)  
		return aS_WORKING;
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return aS_IDLEING;
}

static void *wait_action_get(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	stAppCmd_t *cmd = (stAppCmd_t*)event->param;
	
	stInventory_t *inv = app_get_inventory();

	stGetParam_t *p = cmd->param;
	p->attr		= (char *)(p+1);
	p->value	= p->attr + strlen(p->attr) + 1;
	log_debug("p->attr: %s, p->value:%s", p->attr, p->value);

	int flag = 0;
	
	int j;
	for (j = 0; j < inv->devs[p->did].clen; j++) {
		int cid = inv->devs[p->did].class[j]&0xff;

		stClass_t *cls = &classes[cid];
		if ((cls->cid&0xff) != cid) {
			continue;
		}
		if (cls->attrs_cnt == 0) {
			continue;
		}

		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &cls->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				continue;
			}
			if (attr->usenick == 0) {
				continue;
			}
			if (strcmp(attr->nick, p->attr) != 0) {
				continue;
			}

			if (attr->get != NULL) {
				stSendDataIn_t sdi;
				int len = 0;
				char *argv[1] = {p->value};
				attr->get(p->did, cls->cid, attr->aid, argv, 1, &sdi, &len);
				api_call(CmdZWaveSendData, (stParam_t*)&sdi, len);
				flag++;
			}
		}
	}


	return (void*)flag;
}
static int		wait_transition_get(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	if ((int)acret > 0) 
		return aS_WORKING;
	return aS_IDLEING;
}

static void *wait_action_online_check(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return NULL;
}
static int		wait_transition_online_check(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return aS_WORKING;
}

static void *wait_action_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);

	if (state_machine_get_state_trigger(sm, state_machine_get_state(sm)) == aE_ATTR) {
		stInventory_t *inv = app_get_inventory();

		log_debug("version---->ver:%s, type:%02x", inv->ver.ver, inv->ver.type);

		log_debug("init data---->:");
		log_debug("ver:%02x, capabilities:%02x,nodes_map_size:%02x, chip_type:%02x, chip_version:%02x",
				inv->initdata.ver, inv->initdata.capabilities, inv->initdata.nodes_map_size,
				inv->initdata.chip_type, inv->initdata.chip_version);
		log_debug_hex("nodes_map:",inv->initdata.nodes_map, inv->initdata.nodes_map_size);

		log_debug("Capabilities--->:");
		log_debug("AppVersion:%02x, AppRevisioin:%02x, ManufacturerId:%04x, ManufactureProductType:%04x, ManufactureProductId:%04x",
				inv->caps.AppVersion, inv->caps.AppRevisioin, inv->caps.ManufacturerId, 
				inv->caps.ManufactureProductType, inv->caps.ManufactureProductId);
		log_debug("Id-->:");
		log_debug("HomeId:%08X, NodeId:%02x", inv->id.HomeID, inv->id.NodeID);
		log_debug("SucId--->:sudid:%02x", inv->sucid.SUCNodeID);
	}

	return NULL;
}
static int		wait_transition_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s] [%s]-..----------", __FILE__, __func__);
	return aS_IDLEING;
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
			if (attr->usenick == 0 || attr->nick[0] == '\0') {
				continue;
			}

			char value[32];
			if (memory_get_attr(did, attr->name, value) == 0)  {
				json_object_set_new(jattrs, attr->nick, json_string(value));
			} else if (flash_load_attr(did, attr->name, value) == 0) {
				memory_set_attr(did, attr->name, value);
				json_object_set_new(jattrs, attr->nick, json_string(value));
			} else {
				if (attr->get != NULL && !app_util_is_battery_device(did)) {
					app_zclass_cmd_get(did, attr->nick, "");
				}
				json_object_set_new(jattrs, attr->nick, json_string(""));
			}
		}
	}
	return 0;
}

int	app_zinit() {
	app_push(aE_INIT, NULL, 0);
	return 0;
}
int	app_zclass() {
	app_push(aE_CLASS, NULL, 0);
	return 0;
}
int	app_zattr() {
	app_push(aE_ATTR, NULL, 0);
	return 0;
}

int	app_zattr_new() {
	app_push(aE_ATTR_NEW, NULL, 0);
	return 0;
}

int app_zfresh_nodemap() {
	app_push(aE_FRESH_NODEMAP, NULL, 0);
	return 0;
}


json_t *	app_zlist() {
	json_t *jdevs = json_array();

	stInventory_t *inv = app_get_inventory();

	int i = 0;
	for (i = 0; i < sizeof(inv->devs)/sizeof(inv->devs[0]); i++) {
		stDevice_t *dev = &inv->devs[i];
		if (dev->id == 0 || dev->id == 1) {
			continue;
		}

		json_t *jdev = json_object();
		json_object_set_new(jdev,	"mac",			json_integer(dev->id));
		json_object_set_new(jdev,	"type",			json_string(class_cmd_specific2str(dev->generic, dev->specific)));
		json_object_set_new(jdev,	"model",		json_string(class_cmd_generic2str(dev->generic)));
		json_object_set_new(jdev,	"online",		json_integer(!!dev->online));
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
	app_push(aE_INCLUDE, NULL, 0);
	return 0;
}

int	app_zexclude(int did) {
	app_push(aE_EXCLUDE, &did, sizeof(did));
	return 0;
}

int				app_zexclude_by_mac(const char *mac) {
	int i = 0;
	stInventory_t *inv = app_get_inventory();
	for (i = 0; i < sizeof(inv->devs)/sizeof(inv->devs[0]); i++) {
		stDevice_t *dev = &inv->devs[i];
		if (dev->id == 0) {
			continue;
		}

		char devmac[32];
		memory_get_attr(dev->id, "manufacturer_specific", devmac);
		if (strcmp(devmac, mac) != 0) {
			continue;
		}

		app_zexclude(dev->id);
		app_zfresh_nodemap();
	}
		
	return 0;

}

int	app_zclass_cmd_set(int did, char *attr, char *value) {
	int aLen = strlen(attr);
	int vLen = strlen(value);
	stSetParam_t *p = (stSetParam_t*)MALLOC(aLen + vLen + 2 + sizeof(stSetParam_t));

	p->did = did;
	p->attr = (char*)(p + 1);
	p->value = (char*)(p->attr + aLen + 1);
	strcpy(p->attr, attr);
	strcpy(p->value, value);

	app_push(aE_SET, p, aLen + vLen + 2 + sizeof(stSetParam_t));
	FREE(p);
	return 0;
}

int	app_zclass_cmd_get(int did, char *attr, char *value) {
	int aLen = strlen(attr);
	int vLen = strlen(value);
	stGetParam_t *p = (stGetParam_t*)MALLOC(aLen + vLen + 2 + sizeof(stGetParam_t));

	p->did = did;
	p->attr = (char*)(p + 1);
	p->value = (char*)(p->attr + aLen + 1);
	strcpy(p->attr, attr);
	strcpy(p->value, value);
	
	app_push(aE_GET, p, aLen + vLen + 2 + sizeof(stGetParam_t));
	FREE(p);
	return 0;
}

int	app_zclass_cmd_set_by_mac(const char *mac, char *attr, char *value) {
	int i = 0;
	stInventory_t *inv = app_get_inventory();
	for (i = 0; i < sizeof(inv->devs)/sizeof(inv->devs[0]); i++) {
		stDevice_t *dev = &inv->devs[i];
		if (dev->id == 0) {
			continue;
		}

		char devmac[32];
		memory_get_attr(dev->id, "manufacturer_specific", devmac);
		if (strcmp(devmac, mac) != 0) {
			continue;
		}

		app_zclass_cmd_set(dev->id, attr, value);
		app_zclass_cmd_get(dev->id, attr, "");
	}
		
	return 0;
}

int	app_zclass_cmd_get_by_mac(const char *mac, char *attr, char *value) {
	int i = 0;
	stInventory_t *inv = app_get_inventory();
	for (i = 0; i < sizeof(inv->devs)/sizeof(inv->devs[0]); i++) {
		stDevice_t *dev = &inv->devs[i];
		if (dev->id == 0) {
			continue;
		}

		char devmac[32] = {0};
		memory_get_attr(dev->id, "manufacturer_specific", devmac);
		
		if (strcmp(devmac, mac) != 0) {
			continue;
		}

		log_debug("get dev->id : %d, mac:%s, attr:%s", dev->id, devmac, attr);
		app_zclass_cmd_get(dev->id, attr, value);
	}
	
	return 0;
}


int	app_zclass_cmd_rpt(int did, int cid, int aid, char *value, int value_len) {
	log_debug("[%s] ----->0x%02x, 0x%02x, 0x%02x", __func__, did&0xff, cid&0xff, aid&0xff);
	log_debug_hex("value:", value, value_len);

	if ((cid&0xff) == 0x84 && (aid&0xff) == 0x07) {
		app_zclass_cmd_set(did, "nomore", "");
	}

	stInventory_t *inv = app_get_inventory();
	inv->devs[did].online = 15*60;

	stClass_t *class = &classes[cid&0xff];
	if ((class->cid&0xff) != (cid&0xff)) {
		return -1;
	}

	if (class->attrs_cnt == 0) {
		return -1;
	}

	stAttr_t *attr = &class->attrs[aid&0xff];
	if (attr->name == NULL || attr->name[0] == 0) {
		return -1;
	}
	if ((attr->aid&0xff) != aid) {
		return -1;
	}

	char buf[32] = {0};
	class_cmd_rpt_attr(did, cid, aid, buf, value, value_len);
	if (strcmp(buf, "") != 0) {
		log_debug("memory save : did:%d, attr:%s, value:%s", did, attr->nick, buf);
#if 0
		memory_set_attr(did, attr->name, buf);
		flash_save_attr(did, attr->name, buf);
#else
		char oldbuf[32];
		memory_get_attr(did, attr->name, oldbuf);
		if (strcmp(oldbuf, buf) != 0) {
			memory_set_attr(did, attr->name, buf);
			flash_save_attr(did, attr->name, buf);
		}

			char submac[32];
			memory_get_attr(did, "manufacturer_specific", submac);
			log_debug("uproto report %s, %s", attr->nick, buf);
			const char *type = class_cmd_specific2str(inv->devs[did].generic, inv->devs[did].specific);
			if (attr->nick[0] != '\0') {
				uproto_report_dev_attr(submac, type, attr->nick, buf);
			}
#endif
	}
	return 0;
}

json_t *	app_zinfo() {
	stInventory_t *inv = app_get_inventory();
	
	json_t *jinfo = json_object();
	if (jinfo != NULL) {
		char buf[32];
		sprintf(buf, "%08X", inv->id.HomeID);
		json_object_set_new(jinfo, "HomeID",		json_string(buf));

		sprintf(buf, "%02X", inv->id.NodeID);
		json_object_set_new(jinfo, "NodeID",		json_string(buf));

		sprintf(buf, "%02X", inv->sucid.SUCNodeID);
		json_object_set_new(jinfo, "SUCNOdeID", json_string(buf));
	}
	return jinfo;
}

stInventory_t *app_get_inventory() {
	return &ae.inventory;
}

int app_util_is_battery_device(int did) {
	stInventory_t *inv = app_get_inventory();
	int i = 0;
	//log_debug_hex("clses:", inv->devs[did].class, inv->devs[did].clen);
	for (i = 0; i < inv->devs[did].clen; i++) {
		if ((inv->devs[did].class[i]&0xff) == 0x80 || (inv->devs[did].class[i]&0xff) == 0x84) {
			//log_debug("dev %d is battery device", did);
			return 1;
		}
	}
	return 0;
}



