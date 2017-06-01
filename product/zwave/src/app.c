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


static void *wait_action_init(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_init(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_class(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_class(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_include(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_include(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_exclude(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_exclude(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_command(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_command(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void *wait_action_init_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_init_over(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void *wait_action_class_step(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_class_step(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void *wait_action_include_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_include_over(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void *wait_action_exclude_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_exclude_over(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void *wait_action_command_over(stStateMachine_t *sm, stEvent_t *event);
static int		wait_transition_command_over(stStateMachine_t *sm, stEvent_t *event, void *acret);


stStateMachine_t smApp = {
	6,  S_IDLEING, S_IDLEING, {
		{S_IDLEING, 5, NULL, {
				{E_INIT,		wait_action_init,			wait_transition_init},
				{E_CLASS,		wait_action_class,		wait_transition_class},
				{E_INCLUDE, wait_action_include,	wait_transition_include},
				{E_EXCLUDE, wait_action_exclude,	wait_transition_exclude},
				{E_COMMAND, wait_action_command,	wait_transition_command},
			},
		},
		{S_INITTING, 1, NULL, {
				{E_INIT_OVER, wait_action_init_over, wait_transition_init_over},
			},
		},
		{S_CLASSING, 1, NULL, {
				{E_CLASS_STEP, wait_action_class_step, wait_transition_class_step},
			},
		},
		{S_INCLUDING, 1, NULL, {
				{E_INCLUDE_OVER, wait_action_include_over, wait_transition_include_over},
			},
		},
		{S_EXCLUDING, 1, NULL, {
				{E_EXCLUDE_OVER, wait_action_exclude_over, wait_transition_exclude_over},
			},
		},
		{S_COMMANDING, 1, NULL, {
				{E_COMMAND_OVER, wait_action_command_over, wait_transition_command_over},
			},
		},
	},
};


static stAppEnv_t ae;

int app_init(void *_th, void *_fet) {
	ae.th = _th;
	ae.fet = _fet;
	
	timer_init(&ae.step_timer, app_run);
	lockqueue_init(&ae.eq);

	ae.dev_num = 0;
	memset(ae.devs, 0, sizeof(ae.devs));

	return 0;
}

int app_step() {
	timer_cancel(ae.th, &ae.step_timer);
	timer_set(ae.th, &ae.step_timer, 10);
	return 0;
}

int app_push(stEvent_t *e) {
	lockqueue_push(&ae.eq, e);
	app_step();
	return 0;
}

void app_in(void *arg, int fd) {
}

void app_run(struct timer *timer) {
	stEvent_t *e;

	if (!lockqueue_pop(&ae.eq, (void**)&e)) {
		return;
	}

	if (e == NULL) {
		return;
	}

	state_machine_step(&smApp, e);

	FREE(e);
	
	app_step();
}

void app_util_push(int eid, void *param) {
	stEvent_t *e = MALLOC(sizeof(stEvent_t));
	if (e == NULL) {
		return;
	}
	e->eid = eid;
	e->param = param;
	app_push(e);
}

stAppEnv_t *app_util_getae() {
	return &ae;
}

//////////////////////////////////////////////////////////////////
static void *wait_action_init(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
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
	log_debug("----------[%s]-..----------", __func__);

	return S_INITTING;
}


static void *wait_action_class(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	
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
		stNodeInfoIn_t nii = {id};
		ae.devs[id].id = id;
		ae.lastid = id;
		api_call(CmdZWaveRequestNodeInfo, (stParam_t*)&nii, sizeof(nii));
	}
	return NULL;
}
static int		wait_transition_class(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_CLASSING;
}

static void *wait_action_include(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int		wait_transition_include(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_INITTING;
}

static void *wait_action_exclude(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int		wait_transition_exclude(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_EXCLUDING;
}

static void *wait_action_command(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int		wait_transition_command(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_COMMANDING;
}


static void *wait_action_init_over(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	
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

	app_util_push(E_CLASS, NULL);
	return NULL;
}
static int		wait_transition_init_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_IDLEING;
}

static void *wait_action_class_step(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	stNodeInfo_t *ni = event->param;

	int id = ni->bNodeID&0xff;
	ae.devs[id].id = id;
	ae.devs[id].basic = ni->basic;
	ae.devs[id].generic = ni->generic;
	ae.devs[id].specific = ni->specific;
	ae.devs[id].clen = ni->len - 3;
	memcpy(ae.devs[id].class, ni->commandclasses, ae.devs[id].clen);

	log_debug("id is %d, ae.lastid:%d, clen:%d", id, ae.lastid, ni->len-3);
	if (id != ae.lastid) {
		return (void*)S_CLASSING;
	}

	do_cmd_list();
	
	return (void*)S_IDLEING;
}
static int		wait_transition_class_step(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------2", __func__);
	return (int)acret;
}

static void *wait_action_include_over(stStateMachine_t *sm, stEvent_t *event) {
	return NULL;
}
static int		wait_transition_include_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_IDLEING;
}

static void *wait_action_exclude_over(stStateMachine_t *sm, stEvent_t *event) {
	return NULL;
}
static int		wait_transition_exclude_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_IDLEING;
}

static void *wait_action_command_over(stStateMachine_t *sm, stEvent_t *event) {
	return NULL;
}
static int		wait_transition_command_over(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_IDLEING;
}




