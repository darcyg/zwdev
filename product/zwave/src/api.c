#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "frame.h"
#include "api.h"
#include "log.h"
#include "timer.h"
#include "common.h"

#include "session.h"
#include "lockqueue.h"
#include "statemachine.h"
#include "app.h"
#include "classcmd.h"

#if 0
typedef struct stApiEnv {
	stLockQueue_t qSend;
	stApiCall_t *apicall;
	int initFlag;
  struct timer timerSend;
	
	struct timer_head *th;
}stApiEnv_t;

static API_CALL_CALLBACK api_ccb = NULL;
static API_RETURN_CALLBACK api_crb = NULL;


static stApiEnv_t env = {
	.qSend = {},
	.apicall = NULL,
	.initFlag = 0,
	.timerSend = {},
};


/* CmdZWaveGetVersion */
static stParam_t *CmdZWaveGetVersion_parse_input(stDataFrame_t *df) {
	return NULL;
}
static void CmdZWaveGetVersion_step_input() {
}
static stParam_t *CmdZWaveGetVersion_parse_version(stDataFrame_t *df) {
	if (df != NULL) {
		stVersion_t *ver = (stVersion_t *)MALLOC(sizeof(stVersion_t));
		if (ver == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		strcpy(ver->ver, df->payload);
		ver->type = df->payload[strlen(ver->ver) + 1];
		//log_debug(":%p, ver:%s, type:%02x", ver, ver->ver, ver->type);	
		return (stParam_t*)ver;
	}
	return NULL;
}
static void CmdZWaveGetVersion_view_version(stParam_t *param) {
	if (param != NULL) {
		stVersion_t *ver = (stVersion_t*)param;
		log_debug("ver:%s,type:%02x", ver->ver, ver->type);
	}
}
static void CmdZWaveGetVersion_step_version() {
}
/* CmdSerialApiGetInitData */

static stParam_t *CmdSerialApiGetInitData_parse_input(stDataFrame_t *df) {
	return NULL;
}
static void CmdSerialApiGetInitData_step_input() {
}

static stParam_t *CmdSerialApiGetInitData_parse_initdata(stDataFrame_t *df) {
	if (df != NULL) {
		stInitData_t *id= (stInitData_t *)MALLOC(sizeof(stInitData_t));
		if (id == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		id->ver = df->payload[0];
		id->capabilities = df->payload[1];
		id->nodes_map_size = df->payload[2];
		if (id->nodes_map_size > 0) {
			memcpy(id->nodes_map, &df->payload[3], id->nodes_map_size);
		}
		id->chip_type = df->payload[id->nodes_map_size +  3];
		id->chip_version = df->payload[id->nodes_map_size + 4];
		return (stParam_t*)id;
	}
	return NULL;
}
static void CmdSerialApiGetInitData_parse_initdata_view(stParam_t *param) {
	if (param != NULL) {
		stInitData_t *id = (stInitData_t*)param;
		log_debug("ver:%02x, capabilities:%02x, nodes_map_size:%02x, chip_type:%02x, chip_version:%02x",
			id->ver, id->capabilities, id->nodes_map_size, id->chip_type, id->chip_version	
		);
		log_debug_hex("nodes:", id->nodes_map, id->nodes_map_size);
	}
}
static void CmdSerialApiGetInitData_step_initdata() {
}

/* nodeprotoinfo */
static stParam_t* CmdZWaveGetNodeProtoInfo_parse_input(stDataFrame_t *df) {
	if (df != NULL) {
		stNodeProtoInfoIn_t *npii = (stNodeProtoInfoIn_t *)MALLOC(sizeof(stNodeProtoInfoIn_t));
		if (npii == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		npii->bNodeID = df->payload[0];
		return (stParam_t*)npii;
	}
	return NULL;

}
static void CmdZWaveGetNodeProtoInfo_view_input(stParam_t *param) {
	if (param != NULL) {
		stNodeProtoInfoIn_t *npii = (stNodeProtoInfoIn_t*)param;
		log_debug("nodeId is 0x%02x", npii->bNodeID);
	}
}
static void CmdZWaveGetNodeProtoInfo_step_input() {
}
static stParam_t* CmdZWaveGetNodeProtoInfo_parse_nodeprotoinfo(stDataFrame_t *df) {
	if (df != NULL) {
		stNodeProtoInfo_t *npi = (stNodeProtoInfo_t *)MALLOC(sizeof(stNodeProtoInfoIn_t));
		if (npi == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		npi->Capability = df->payload[0];
		npi->Security		= df->payload[1];
		npi->Basic			= df->payload[2];
		npi->Generic		= df->payload[3];
		npi->Specific		= df->payload[4];
		return (stParam_t*)npi;
	}
	return NULL;
}
static void CmdZWaveGetNodeProtoInfo_view_nodeprotoinfo(stParam_t *param) {
	if (param != NULL) {
		stNodeProtoInfo_t *npi = (stNodeProtoInfo_t*)param;
		log_debug("Capability:%02x,Security:%02x,Basic:%02x,Generic:%02x,Specific:%02x", 
			npi->Capability&0xff, npi->Security&0xff, npi->Basic&0xff, npi->Generic&0xff, npi->Specific&0xff
		);
	}
}
static void CmdZWaveGetNodeProtoInfo_step_nodeprotoinfo() {
}

/* capabilities */

static stParam_t *CmdSerialApiGetCapabilities_parse_input(stDataFrame_t *df) {
	return NULL;
}
static void CmdSerialApiGetCapabilities_step_input() {
}
static stParam_t *CmdSerialApiGetCapabilities_parse_capabilities(stDataFrame_t*df) {
	if (df != NULL) {
		stCapabilities_t *capa = (stCapabilities_t *)MALLOC(sizeof(stCapabilities_t));
		if (capa == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		capa->AppVersion = df->payload[0];
		capa->AppRevisioin = df->payload[1];
		capa->ManufacturerId = df->payload[2]*256 + df->payload[3];
		capa->ManufactureProductType = df->payload[4]*256 + df->payload[5];
		capa->ManufactureProductId = df->payload[6]*256 + df->payload[7];
		memcpy(capa->SupportedFuncIds_map, &df->payload[8], sizeof(capa->SupportedFuncIds_map));
		//capa->SupportedFuncIds_slots[];
		return (stParam_t*)capa;
	}
	return NULL;
}
static void  CmdSerialApiGetCapabilities_view_capabilities(stParam_t *param) {
	if (param != NULL) {
		stCapabilities_t *capa = (stCapabilities_t*)param;
		log_debug("AppVersion:%02x, AppRevisioin:%02x, ManufacturerId:%04x, ManufactureProductType:%04x, ManufactureProductId:%04x",
		capa->AppVersion, capa->AppRevisioin, capa->ManufacturerId, capa->ManufactureProductType, 
		capa->ManufactureProductId);
		log_debug_hex("SupporttedFuncIds_Map:", capa->SupportedFuncIds_map, sizeof(capa->SupportedFuncIds_map));
	}
}
static void  CmdSerialApiGetCapabilities_step_capabilities() {
}

/* */

static stParam_t *CmdZWaveGetControllerCapabilities_parse_input(stDataFrame_t *df) {
	return NULL;
}
static void CmdZWaveGetControllerCapabilities_step_input(stDataFrame_t *df) {
}
static stParam_t *CmdZWaveGetControllerCapabilities_parse_controllercapalities(stDataFrame_t *df) {
	if (df != NULL) {
		stControllerCapabilities_t *cc = (stControllerCapabilities_t *)MALLOC(sizeof(stControllerCapabilities_t));
		if (cc == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		cc->RetVal = df->payload[0];
		return (stParam_t*)cc;
	}
	return NULL;

}
static void CmdZWaveGetControllerCapabilities_parse_controllercapalities_view(stParam_t *param) {
	if (param != NULL) {
		stControllerCapabilities_t *cc =(stControllerCapabilities_t*)param;
		log_debug("RetVal : %02x", cc->RetVal);
	}
}
static void CmdZWaveGetControllerCapabilities_step_controllercapalities() {
}

/* */
static stParam_t *CmdMemoryGetId_parse_input(stDataFrame_t *df) {
	return NULL;
}
static void CmdMemoryGetId_step_input() {
}
static stParam_t *CmdMemoryGetId_parse_id(stDataFrame_t *df) {
	if (df != NULL) {
		stId_t *i = (stId_t *)MALLOC(sizeof(stId_t));
		if (i == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		i->HomeID = *(int*)&df->payload[0];
		i->NodeID = df->payload[4];
		return (stParam_t*)i;
	}
	return NULL;

}
static void CmdMemoryGetId_view_id(stParam_t *param) {
	if (param != NULL) {
		stId_t *i = (stId_t*)param;
		log_debug("HomeId : %08x, NodeId:%02x", i->HomeID, i->NodeID);
	}
}
static void CmdMemoryGetId_step_id() {
}


/* */
static stParam_t *CmdZWaveGetSucNodeId_parse_input(stDataFrame_t *df) {
	return NULL;
}
static void CmdZWaveGetSucNodeId_step_input() {
}
static stParam_t *CmdZWaveGetSucNodeId_parse_sucnodeid(stDataFrame_t *df) {
	if (df != NULL) {
		stSucNodeId_t *sni = (stSucNodeId_t *)MALLOC(sizeof(stSucNodeId_t));
		if (sni == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		sni->SUCNodeID = df->payload[0];
		return (stParam_t*)sni;
	}
	return NULL;
}
static void CmdZWaveGetSucNodeId_view_sucnodeid(stParam_t *param) {
	if (param != NULL) {
		stSucNodeId_t *sni = (stSucNodeId_t*)param;
		log_debug("SUCNodeID:%02x", sni->SUCNodeID);
	}
}
static void CmdZWaveGetSucNodeId_step_sucnodeid() {
}


static stParam_t *CmdSerialApiApplNodeInformation_parse_applnodeinformation(stDataFrame_t *df) {
	if (df != NULL) {
		stApplNodeInformationIn_t *anii = (stApplNodeInformationIn_t *)MALLOC(sizeof(stApplNodeInformationIn_t));
		if (anii == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		anii->deviceOptionsMask = df->payload[0];
		anii->generic = df->payload[1];
		anii->specific = df->payload[2];
		memcpy(anii->nodeParm, &df->payload[3] , sizeof(anii->nodeParm));
		return (stParam_t*)anii;
	}
	return NULL;
} 
static void CmdSerialApiApplNodeInformation_view_applnodeinformation(stParam_t *param) {
	if (param != NULL) {
		stApplNodeInformationIn_t *anii = (stApplNodeInformationIn_t*)param;
		log_debug("deviceOptionsMask:%02x,generic:%02x,specific:%02x", 
			anii->deviceOptionsMask,anii->generic,anii->specific
		);
		log_debug_hex("nodeParm:", anii->nodeParm, sizeof(anii->nodeParm));
	}
}
static void CmdSerialApiApplNodeInformation_step_applnodeinformation() {
}

/* Add Node To Network*/
static stParam_t* CmdZWaveAddNodeToNetwork_parse_in(stDataFrame_t *df) {
	if (df != NULL) {
		stAddNodeToNetworkIn_t *in = (stAddNodeToNetworkIn_t *)MALLOC(sizeof(stAddNodeToNetworkIn_t));
		if (in == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		in->mode = df->payload[0];
		in->funcID = df->payload[1];
		return (stParam_t*)in;
	}
	return NULL;
}
static void CmdZWaveAddNodeToNetwork_view_in(stParam_t *param) {
	if (param != NULL) {
		stAddNodeToNetworkIn_t *in = (stAddNodeToNetworkIn_t*)param;
		log_debug("mode:%02x,funcID:%02x", in->mode, in->funcID);
	}
}
static void CmdZWaveAddNodeToNetwork_step_in() {
}
static stParam_t* CmdZWaveAddNodeToNetwork_parse_wait(stDataFrame_t *df) {
	if (df != NULL) {
		stAddNodeToNetworkWait_t *w = (stAddNodeToNetworkWait_t *)MALLOC(sizeof(stAddNodeToNetworkWait_t));
		if (w == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		w->funcID = df->payload[0];
		w->dummy1 = df->payload[1];
		w->dummy2 = df->payload[2];
		w->dummy3 = df->payload[3];
		return (stParam_t*)w;
	}
	return NULL;
}
static void CmdZWaveAddNodeToNetwork_view_wait(stParam_t *param) {
	if (param != NULL) {
		stAddNodeToNetworkWait_t *w = (stAddNodeToNetworkWait_t*)param;
		log_debug("funcID:%02x", w->funcID);
	}
}
static void CmdZWaveAddNodeToNetwork_step_wait() {
}
static stParam_t* CmdZWaveAddNodeToNetwork_parse_back(stDataFrame_t *df) {
	if (df != NULL) {
		stAddNodeToNetworkBack_t *b = (stAddNodeToNetworkBack_t *)MALLOC(sizeof(stAddNodeToNetworkBack_t));
		if (b == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		b->funcID = df->payload[0];
		b->dummy1 = df->payload[1];
		b->dummy2 = df->payload[2];
		b->dummy3 = df->payload[3];
		return (stParam_t*)b;
	}
	return NULL;

}
static void CmdZWaveAddNodeToNetwork_view_back(stDataFrame_t *param) {
	if (param != NULL) {
		stAddNodeToNetworkBack_t *b = (stAddNodeToNetworkBack_t*)param;
		log_debug("funcID:%02x", b->funcID);
	}
}
static void CmdZWaveAddNodeToNetwork_step_back() {
}
static stParam_t* CmdZWaveAddNodeToNetwork_parse_addnodetonetwork(stDataFrame_t *df) {
	if (df != NULL) {
		stAddNodeToNetwork_t *x = (stAddNodeToNetwork_t *)MALLOC(sizeof(stAddNodeToNetwork_t));
		if (x == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		x->funcID = df->payload[0];
		x->bStatus = df->payload[1];
		x->bSource = df->payload[2];
		x->len = df->payload[3];
		x->basic = df->payload[4];
		x->generic = df->payload[5];
		x->specific = df->payload[6];
		memcpy(x->commandclasses, &df->payload[7], x->len - 3);
		
		return (stParam_t*)x;
	}
	return NULL;
}

static void CmdZWaveAddNodeToNetwork_view_addnodetonetwork(stParam_t *param) {
	if (param != NULL) {
		stAddNodeToNetwork_t *x = (stAddNodeToNetwork_t*)param;
		log_debug("funcID:%02x, bStatus:%02x, bSource:%02x, len:%02x, basic:%02x, generic:%02x, specific:%02x",
			x->funcID, x->bStatus, x->bSource, x->len, x->basic, x->generic, x->specific
		);
		log_debug_hex("commandclass:", x->commandclasses, x->len - 3);
	}
}
static void CmdZWaveAddNodeToNetwork_step_addnodetonetwork() {
}
static stParam_t* CmdZWaveAddNodeToNetwork_parse_comp(stDataFrame_t *df) {
	if (df != NULL) {
		stAddNodeToNetworkComp_t *c = (stAddNodeToNetworkComp_t *)MALLOC(sizeof(stAddNodeToNetworkComp_t));
		if (c == NULL) {	
			log_debug("no enough memory!");
			return NULL;
		}
		c->funcID = df->payload[0];
		c->dummy1 = df->payload[1];
		c->dummy2 = df->payload[2];
		c->dummy3 = df->payload[3];
		return (stParam_t*)c;
	}
	return NULL;


}
static void CmdZWaveAddNodeToNetwork_view_comp(stParam_t *param) {
	if (param != NULL) {
		stAddNodeToNetworkComp_t *c = (stAddNodeToNetworkComp_t*)param;
		log_debug("funcID:%02x", c->funcID);
	}
}
static void CmdZWaveAddNodeToNetwork_step_comp() {
}




static stApiStateMachine_t asms[] = {
	[CmdZWaveGetVersion] = {
		CmdZWaveGetVersion, "CmdZWaveGetVersion", 0, 2, {
			{CmdZWaveGetVersion_parse_input, NULL, CmdZWaveGetVersion_step_input},
			{CmdZWaveGetVersion_parse_version, CmdZWaveGetVersion_view_version, CmdZWaveGetVersion_step_version},
		},
	},
	[CmdSerialApiGetInitData] = {
		CmdSerialApiGetInitData, "CmdSerialApiGetInitData", 0, 2, {
			{CmdSerialApiGetInitData_parse_input, NULL, CmdSerialApiGetInitData_step_input},
			{CmdSerialApiGetInitData_parse_initdata, CmdSerialApiGetInitData_parse_initdata_view, CmdSerialApiGetInitData_step_initdata},
		},
	},
	[CmdZWaveGetNodeProtoInfo] = {
		CmdZWaveGetNodeProtoInfo, "CmdZWaveGetNodeProtoInfo", sizeof(stNodeProtoInfoIn_t), 2, {
			{CmdZWaveGetNodeProtoInfo_parse_input, CmdZWaveGetNodeProtoInfo_view_input, CmdZWaveGetNodeProtoInfo_step_input},
			{CmdZWaveGetNodeProtoInfo_parse_nodeprotoinfo, CmdZWaveGetNodeProtoInfo_view_nodeprotoinfo, CmdZWaveGetNodeProtoInfo_step_nodeprotoinfo},
		},
	},

	[CmdSerialApiGetCapabilities] = {
		CmdSerialApiGetCapabilities ,"CmdSerialApiGetCapabilities", 0, 2, {
			{CmdSerialApiGetCapabilities_parse_input, NULL, CmdSerialApiGetCapabilities_step_input},
			{CmdSerialApiGetCapabilities_parse_capabilities, CmdSerialApiGetCapabilities_view_capabilities, CmdSerialApiGetCapabilities_step_capabilities},
		},
	},

	[CmdZWaveGetControllerCapabilities] = {
			CmdZWaveGetControllerCapabilities, "CmdZWaveGetControllerCapabilities", 0, 2, {
			{CmdZWaveGetControllerCapabilities_parse_input, NULL, CmdZWaveGetControllerCapabilities_step_input},
			{CmdZWaveGetControllerCapabilities_parse_controllercapalities, CmdZWaveGetControllerCapabilities_parse_controllercapalities_view,
					 CmdZWaveGetControllerCapabilities_step_controllercapalities},
		},
	},

	[CmdMemoryGetId] = {
		CmdMemoryGetId, "CmdMemoryGetId", 0, 2, {
			{CmdMemoryGetId_parse_input, NULL, CmdMemoryGetId_step_input},
			{CmdMemoryGetId_parse_id, CmdMemoryGetId_view_id, CmdMemoryGetId_step_id},
		},
	},
	[CmdZWaveGetSucNodeId] ={
		CmdZWaveGetSucNodeId, "CmdZWaveGetSucNodeId", 0, 2, {
			{CmdZWaveGetSucNodeId_parse_input, NULL, CmdZWaveGetSucNodeId_step_input},
			{CmdZWaveGetSucNodeId_parse_sucnodeid, CmdZWaveGetSucNodeId_view_sucnodeid, CmdZWaveGetSucNodeId_step_sucnodeid},
		},
	},
	[CmdSerialApiApplNodeInformation] = {
		CmdSerialApiApplNodeInformation, "CmdSerialApiApplNodeInformation", 0, 1, {
			{CmdSerialApiApplNodeInformation_parse_applnodeinformation, CmdSerialApiApplNodeInformation_view_applnodeinformation,
				CmdSerialApiApplNodeInformation_step_applnodeinformation},
		},
	},

	[CmdZWaveAddNodeToNetwork] = {
		CmdZWaveAddNodeToNetwork, "CmdZWaveAddNodeToNetwork", sizeof(stAddNodeToNetworkIn_t), 5, {
			{CmdZWaveAddNodeToNetwork_parse_in, CmdZWaveAddNodeToNetwork_view_in, CmdZWaveAddNodeToNetwork_step_in},
			{CmdZWaveAddNodeToNetwork_parse_wait, CmdZWaveAddNodeToNetwork_view_wait, CmdZWaveAddNodeToNetwork_step_wait},
			{CmdZWaveAddNodeToNetwork_parse_back, CmdZWaveAddNodeToNetwork_view_back, CmdZWaveAddNodeToNetwork_step_back},
			{CmdZWaveAddNodeToNetwork_parse_addnodetonetwork, CmdZWaveAddNodeToNetwork_view_addnodetonetwork, CmdZWaveAddNodeToNetwork_step_addnodetonetwork},
			{CmdZWaveAddNodeToNetwork_parse_comp, CmdZWaveAddNodeToNetwork_view_comp, CmdZWaveAddNodeToNetwork_step_comp},
		},
	},
};

///////////////////////////////////////////////////////////////////////////////////////////

static stDataFrame_t * make_frame(emApi_t api, stParam_t *param) {
	//static int seq = 0;

	int paramSize = asms[api].param_size;
	int size = sizeof(stDataFrame_t) + paramSize;

	stDataFrame_t *df = MALLOC(size);
	if (df == NULL) {
		return NULL;
	}

	df->sof			= SOF_CHAR;
	df->len			= paramSize+3;
	df->type		= 0x00,
	df->cmd			= api;
	df->payload = (char *)(df + 1);
	df->size		= paramSize;
	if (df->size > 0) {
		memcpy(df->payload, param, paramSize);
	}
	df->checksum = 0;
	df->timestamp = time(NULL);
	df->checksum_cal = 0;
	df->do_checksum = 0;
	df->trycnt =0;
	df->error = 0;
	
	return df;
}

static void api_end() {
	if (env.apicall != NULL) {
		FREE(env.apicall);
		env.apicall = NULL;
	}
	timer_cancel(env.th, &env.timerSend);
}

static void api_start() {
	if (env.apicall != NULL) {
		return;
	}

	lockqueue_pop(&env.qSend, (void **)&env.apicall);
	if (env.apicall == NULL) {
		return;
	}

	stDataFrame_t * df = make_frame(env.apicall->api, env.apicall->param);
	if (df == NULL) {
		log_debug("make frame error !");
		return;
	}
	frame_send(df);
	env.apicall->state++;

	timer_set(env.th, &env.timerSend, API_EXEC_TIMEOUT_MS);
}

static void api_call_step() {
	if (env.apicall != NULL) {
		timer_cancel(env.th, &env.timerSend);
		timer_set(env.th, &env.timerSend, API_EXEC_TIMEOUT_MS);
		env.apicall->state++;
	}
}


static void api_send_over(void *_df) {
	stDataFrame_t *df = (stDataFrame_t*)_df;

	log_debug("%s in [%s]!", asms[df->cmd].name, __func__);

	if (df != NULL) {
		log_info("size is %02x, %02x, trycnt:%d", df->size, df->len, df->trycnt);
		log_debug("type : %02X, cmd : %02x", df->type, df->cmd);
		log_debug_hex("send payload:", df->payload, df->size);


		if (df->error == FE_NONE) {
			/* never go here */
			log_debug("never go here: FE_NONE");
		} else if (df->error == FE_SEND_TIMEOUT) {
			/* timeout , send timeout */
			if (api_ccb != NULL) {
				stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
				api_ccb(df->cmd, parse(df), 1, AE_SEND_TIMEOUT);
			}
			api_end();
			api_start();
		} else if (df->error == FE_SEND_ACK) {
			/* ack , send ok*/
			if (api_ccb != NULL) {
				stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
				api_ccb(df->cmd, parse(df), 1, AE_NONE);
			}
			api_call_step();

			if (env.apicall->state > asms[df->cmd].num_state) {
				api_end();
			}

		} else if (df->error == FE_SEND_NAK) {
			/* nan , send nak */
			if (api_ccb != NULL) {
				stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
				api_ccb(df->cmd, parse(df), 1, AE_NAK);
			}
			api_end();
			api_start();
		} else if (df->error == FE_SEND_CAN) {
			/* can , send can */
			if (api_ccb != NULL) {
				stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
				api_ccb(df->cmd, parse(df), 1, AE_CAN);
			}
			api_end();
			api_start();
		} else if (df->error == FE_RECV_CHECKSUM) {
			log_debug("never go here: FE_RECV_CHECKSUM");
		} else if (df->error == FE_RECV_TIMEOUT) {
			log_debug("never go here: FE_RECV_TIMEOUT");
		}
		FREE(df);
	}
}

static void api_recv_over(void *_df) {
	stDataFrame_t *df = (stDataFrame_t*)_df;
	log_debug("%s in [%s] !", asms[df->cmd].name, __func__);

	if (df != NULL) {
		log_debug("type : %02X, cmd : %02x", df->type, df->cmd);
		log_debug_hex("recv payload:", df->payload, df->size);

		if (df->error == FE_NONE) {
			/* receive ok */
			if (api_crb != NULL) {
				if (env.apicall != NULL && df->cmd == env.apicall->api) {
					stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[env.apicall->state-1].parse;
					api_crb(df->cmd, parse(df), env.apicall->state, AE_NONE);

					api_call_step();
				} else {
					stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
					api_crb(df->cmd, parse(df), 1, AE_NONE);
				}
			}
			//log_debug("env.apicall->state:%d, asms[df->cmd].num_state:%d", env.apicall->state, asms[df->cmd].num_state);
			if (env.apicall != NULL && env.apicall->api == df->cmd && 
					env.apicall->state > asms[df->cmd].num_state) {
				api_end();
			}

			if (env.apicall == NULL) {
				api_start();
			}
		} else if (df->error == FE_SEND_TIMEOUT) {
			/* never go here */
			log_debug("never go here: FE_SEND_TIMEOUT");
		} else if (df->error == FE_SEND_ACK) {
			/* never go here */
			log_debug("never go here: FE_SEND_ACK");
		} else if (df->error == FE_SEND_NAK) {
			/* never go here */
			log_debug("never go here: FE_SEND_NAK");
		} else if (df->error == FE_SEND_CAN) {
			/* never go here */
			log_debug("never go here: FE_SEND_CAN");
		} else if (df->error == FE_RECV_CHECKSUM) {
			/* receive checksum error */
			log_debug("frame recv checksum error: %02x(correct:%02x)",
								df->checksum, 
								df->checksum_cal);
			if (api_crb != NULL) {
				if (env.apicall != NULL && df->cmd == env.apicall->api) {
					stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[env.apicall->state-1].parse;
					api_crb(df->cmd, parse(df), env.apicall->state, AE_CHECKSUM);

					api_call_step();
				} else {
					stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
					api_crb(df->cmd, parse(df), 1, AE_NONE);
				}
			}
			if (env.apicall != NULL && env.apicall->api == df->cmd && 
					env.apicall->state > asms[df->cmd].num_state) {
				api_end();
			}

			if (env.apicall == NULL) {
				api_start();
			}
		} else if (df->error == FE_RECV_TIMEOUT) {
			/* receive timeout */
			log_debug("frame recv timeout!");
			if (api_crb != NULL) {
				if (env.apicall != NULL && df->cmd == env.apicall->api) {
					stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[env.apicall->state-1].parse;
					api_crb(df->cmd, parse(df), env.apicall->state, AE_CHECKSUM);

					api_call_step();
				} else {
					stParam_t *(*parse)(stDataFrame_t*) = asms[df->cmd].states[0].parse;
					api_crb(df->cmd, parse(df), 1, AE_NONE);
				}
			}
			if (env.apicall != NULL && env.apicall->api == df->cmd && 
					env.apicall->state > asms[df->cmd].num_state) {
				api_end();
			}

			if (env.apicall == NULL) {
				api_start();
			}
		}

		FREE(df);
	}
}

static void api_send_timer_callback(struct timer *timer) {
	api_end();
	api_start();
}

//////////////////////////////////////////////
int api_init(void *_th, API_CALL_CALLBACK _accb, API_RETURN_CALLBACK _arcb) {
	
	if (_accb != NULL) {
		api_ccb = _accb;
	}
	if (_arcb != NULL) {
		api_crb = _arcb;
	}

	if (session_init(_th, api_send_over, api_recv_over) != 0) {
		log_debug("frame init failed!");
		return -1;
	}

	lockqueue_init(&env.qSend);

	env.th = (struct timer_head *)_th;
  timer_init(&env.timerSend, api_send_timer_callback);

	env.apicall = NULL;
	env.initFlag = 1;

	return 0;
}


int api_exec(emApi_t api, stParam_t *param) {
	stApiCall_t *call = MALLOC(sizeof(stApiCall_t) + asms[api].param_size);
	if (call == NULL) {
		log_debug("no enough memory!");
		return -1;
	}
	call->api = api;
	call->state = AS_READY;
	call->param = (stParam_t*)(call + 1);
	if (asms[api].param_size != 0) {
		memcpy(call->param, param, asms[api].param_size);
	}
	lockqueue_push(&env.qSend, call);

	if (env.apicall == NULL) {
		api_start();
	}

	return 0;

}

int api_free() {
	session_free();

	lockqueue_destroy(&env.qSend, NULL);

	env.initFlag = 0;
	return 0;
}

int api_getfd() {
	return session_getfd();
}

int api_step() {
	session_receive_step();
	return 0;
}


const char *api_name(emApi_t api) {
	return asms[api].name;
}

void api_param_view(emApi_t api, stParam_t *param, emApiState_t state) {
	int num_state = asms[api].num_state;
	//log_debug("view state : %d, num_state: %d", state, num_state);
	if (state > num_state || state < 1) {
		log_debug("api state error!");
		return;
	}

	void (*view)(stParam_t*) = asms[api].states[state-1].view;
	if (view != NULL) {
		view(param);
	}
}

#elif 0

//=======================================================================
struct stApiMachineEnv_t ame;
static stApiMachineEnv_t *get_api_machine_env() {
	return &ame;
}


static int api_state_event_handler(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	switch (e->event) {
		case AME_CALL_API:
			if (env->state == AME_IDLE) {
				am_transition_call_api(env, e);
			} else {
				log_debug("unhandled event : %d when state: %d", e->event, env->state);
			}
			break;
		case AME_ACK:
			if (env->state == AME_RUNNING) {
				if (am_action_running_ack(env, e) == 0) {
					;
				} else {
					log_debug("unhandled event : %d when state: %d", e->event, env->state);
				}
			} else {
				log_debug("unhandled event : %d when state: %d", e->event, env->state);
			}
			break;
		case AME_ERROR:
			if (env->state == AME_RUNNING) {
				if (am_action_running_err(env, e) == 0) {
					;
				} else {
					log_debug("unhandled event : %d when state: %d", e->event, env->state);
				}
			} else {
				log_debug("unhandled event : %d when state: %d", e->event, env->state);
			}
			break;
		case AME_DATA:
			if (env->state == AME_RUNNING) {
				if (am_action_running_data(env, e) == 0) {
					;
				} else {
					log_debug("unhandled event : %d when state: %d", e->event, env->state);
				}
			} else {
				log_debug("unhandled event : %d when state: %d", e->event, env->state);
			}
			break;
		case AME_ASYNC_DATA:
			if (env->state == AME_RUNNING) {
				if (am_action_running_async_data(env, e) == 0) {
					;
				} else {
					log_debug("unhandled event : %d when state: %d", e->event, env->state);
				}
			} else {
				log_debug("unhandled event : %d when state: %d", e->event, env->state);
			}
			break;
		case AME_CALL_EXIT:
			if (env->state == AME_RUNNING) {
				am_transition_call_exit(env, e);
			} else {
				log_debug("unhandled event : %d when state: %d", e->event, env->state);
			}
			break;	
		case AME_FREE_RUN:
			break;
		default:
			log_debug("unknown event : %d", e->event);
			break;
	}
	
	return 0;
}
static int api_state_run() {
	stApiMachineEnv_t *env = api_machine_env_get();
	stApiMachineEvent_t *e = api_machine_event_pop();
	if (e == NULL) {
		return 0;
	}
	do  {
		ret = api_state_event_handler(env, e);
		api_machine_event_free(e);
		stApiMachineEvent_t *e = api_machine_event_pop();
	} while (e != NULL);
	return 0;
}


static int api_call_api(stApiCall_t *ac) {
	stDataFrame_t * df = make_frame(ac->api, ac->param);
	if (df == NULL) {
		return -1;
	}
	frame_send(df);
	return 0;
}






int am_transition_call_api(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	stApiCall_t *ac = (stApiCall_t*)e->event_data;
	if (ac == NULL) {
		return -1;
	}
	/* CALL API */
	if (api_call_api(ac) != 0) {
		log_debug("make frame error !");
		return -2;
	}
	
	env->state = AME_RUNNING;

	return 0;
}
int am_transition_call_exit(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	env->state = AME_IDLE;

	stApiMachineEvent_t event = {AME_FREE_RUN};
	api_machine_event_push(&event);
	return 0;
}
int am_action_idle_async_data(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	log_debug("idel data->event : %d when state: %d", e->event, env->state);
	/* not not support async data */
	stDataFrame_t *df = e->param;
	if (df->cmd == 0x04) { //class get, ApplicationCommandHandler
		char rxStatus = df->payload[0];
		char sourceNode = df->payload[1];
		char len = df->payload[2];
		char cid = df->payload[3];
		char op = df->payload[4];
		char *value = df->payload[5];
		int value_len = len -2;
		if (rxStatus == 0 && cid == COMMAND_CLASS_SWITCH_BINARY_V1) { //binary class report
			class_cmd_save(sourceNode, cid, op, value, value_len);
		}
	}
	return 0;
}
int am_action_running_data(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	if (env->acm != NULL && env->acm->run != NULL) {
		env->acm->run(env, e);
	}
}
int am_action_running_ack(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {	
	if (env->acm != NULL && env->acm->run != NULL) {
		env->acm->run(env, e);
	}
}
int am_transition_running_err(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	log_debug("error event : %d when state: %d", e->event, env->state);
	
	env->state = AME_IDLE;

	stApiMachineEvent_t event = {AME_FREE_RUN};
	api_machine_event_push(&event);

	return 0;
}
int am_action_running_call_async_api(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
#if 0
	stApiCall_t *ac = (stApiCall_t*)e->event_data;
	if (ac == NULL) {
		return -1;
	}
	/* CALL API */
	if (api_call_api(ac) != 0) {
		log_debug("make frame error !");
		return -2;
	}
#endif
	if (env->acm != NULL && env->acm->run != NULL) {
		env->acm->run(env, e);
	}
}
int am_action_running_async_data(stApiMachineEnv_t *env, stApiMachineEvent_t *e) {
	log_debug("async data->event : %d when state: %d", e->event, env->state);
}

#elif 0

//////////////////////////////////////////////////////////////////////////////////////
int api_init() {
	if (_accb != NULL) {
		api_ccb = _accb;
	}
	if (_arcb != NULL) {
		api_crb = _arcb;
	}

	if (session_init(_th, api_send_over, api_recv_over) != 0) {
		log_debug("frame init failed!");
		return -1;
	}

	/*
	lockqueue_init(&env.qSend);

	env.th = (struct timer_head *)_th;
  timer_init(&env.timerSend, api_send_timer_callback);

	env.apicall = NULL;
	env.initFlag = 1;
	*/

	return 0;
}
int api_free() {
	session_free();

	/*
	lockqueue_destroy(&env.qSend, NULL);

	env.initFlag = 0;
	*/

	return 0;
}
int api_getfd() {
	return session_getfd();
}
int api_step() {
	session_receive_step();
	return 0;
}
int api_call() {
	return 0;
}

#else
typedef struct stApiEnv {
	stLockQueue_t qSend;
	stLockQueue_t qSendBack;
	stLockQueue_t qRecv;

	stApiCall_t *apicall;


	struct timer_head *th;
  struct timer timerSend;
	struct timer timerBeat;

	int initFlag;

	stVersion_t ver;
	stInitData_t initdata;
	stCapabilities_t caps;
	stId_t id;
	stSucNodeId_t sucid;
}stApiEnv_t;



static API_CALL_CALLBACK api_ccb = NULL;
static API_RETURN_CALLBACK api_crb = NULL;

static stApiEnv_t env = {
	.qSend = {},
	.qSendBack = {},
	.qRecv = {},
	.apicall = NULL,
	.th = NULL,
	.timerSend = {},
	.timerBeat = {},

	.initFlag = 0,

	.ver = {},
	.initdata = {},
	.caps = {},
	.id = {},
	.sucid = {},
};

/* state machine relative */
enum {
	S_IDLE = 0,
	S_RUNNING = 1,

	S_WAIT_INIT_DATA = 2,
	S_WAIT_VERSION_DATA = 3,
	S_WAIT_NODE_PROTOINFO = 4,
	S_WAIT_CAPABILITIES = 5,
	S_WAIT_CONTROLLER_CAPABILITIES = 6,
	S_WAIT_ID = 7,
	S_WAIT_SUC_NODE_ID = 8,
	S_WAIT_APPL_NODE_INFORMATION = 9,

	S_WAIT_CTR_STATUS = 10,
	S_WAIT_ADDED_OR_CANCLE = 11,
	S_WAIT_ADDED_NODE = 12,
	S_WAIT_ADD_COMP = 13,
	S_WAIT_CANCLE_CONFIRM = 14,
	S_WAIT_CANCLE_COMP = 15,

	S_WAIT_NODE_INFO = 16,

	S_WAIT_REMOVE_RESPONSE = 17,	
	S_WAIT_LEAVE_OR_CANCLE = 18,
	S_WAIT_LEAVED_NODE_S1 = 19,
	S_WAIT_LEAVED_NODE_S2 = 20,
	S_WAIT_LEAVE_COMP = 21,

	S_WAIT_SETSUC_RESPONSE = 22,

	S_WAIT_SENDDATA_RESPONSE = 23,
	S_WAIT_TX_STATUS = 24,

	S_WAIT_ISFAILED_RESPONSE = 25,

	S_WAIT_REMOVE_FAILED_RESPONSE = 26,

	S_WAIT_SOFTRESET_RESPONSE = 27,
	
	S_WAIT_PROTOCOL_VERSION = 28,
		
	S_WAIT_API_STARTED = 29,

	S_WAIT_RFPOWER_LEVEL = 30,

	S_WAIT_NEIGHBOR_COUNT = 31,

	S_WAIT_ARE_NEIGHBORS = 32,

	S_WAIT_TYPE_LIBRARY = 33,
	
	S_WAIT_PROTOCOL_STATUS = 34,
	
	S_WAIT_PORT_STATUS = 35,

	S_WAIT_IO_PORT = 36,

	S_WAIT_NODE_INFO_ACK = 37,


	S_END = 9999,
};

enum {
	E_BEAT = 0,
	E_CALL_API = 1,
	E_ASYNC_DATA = 2,
	E_ERROR = 3,
	E_ACK = 4,
  E_DATA = 5,
	

	E_VERSION_DATA = 6,
	E_INIT_DATA = 7,
	E_NODE_PROTOINFO = 8,
	E_CAPABILITIES = 9,
	E_CONTROLLER_CAPABILITIES = 10,
	E_ID = 11,
	E_SUC_NODE_ID = 12,
	E_APPL_NODE_INFORMATION = 13,

	E_CTR_STATUS = 14,
	E_NEWDEV_ADDED = 15,
	E_CANCLE_ADD = 16,
	E_ADDED_NODE = 17,
	E_ADD_COMP = 18,
	E_CANCLE_CONFIRM = 19,
	E_CANCLE_COMP = 20,

	E_NODE_INFO = 21,


	E_REMOVE_RESPONSE = 22,
	E_LEAVING = 23,
	E_CANCLE_REMOVE = 24,
	E_LEAVED_NODE_S1 = 25,
	E_LEAVED_NODE_S2 = 26,
	E_LEAVE_COMP = 27,

	E_SETSUC_RESPONSE = 28,

	E_SENDDATA_RESPONSE = 29,
	E_TX_STATUS = 30,

	E_ISFAILED_RESPONSE = 31,

	E_REMOVE_FAILED_RESPONSE = 32,
	E_SOFTRESET_RESPONSE = 33,
	E_PROTOCOL_VERSION = 34,
	E_API_STARTED = 35,
	E_RFPOWER_LEVEL = 36,
	E_NEIGHBOR_COUNT = 37,
	E_ARE_NEIGHBORS = 38,
	E_TYPE_LIBRARY = 39,
	E_PROTOCOL_STATUS = 40,
	E_PORT_STATUS = 41,
	E_IO_PORT = 42,

	E_NODE_INFO_ACK = 43,

};


static void * idle_action_beat(stStateMachine_t *sm, stEvent_t *event);

static void * idle_action_call_api(stStateMachine_t *sm, stEvent_t *event);
static int    idle_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * idle_action_async_data(stStateMachine_t *sm, stEvent_t *event);

static void * running_action_error(stStateMachine_t *sm, stEvent_t *event);
static int  running_transition_error(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * running_action_ack(stStateMachine_t *sm, stEvent_t *event);
static int  running_transition_ack(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * running_action_async_data(stStateMachine_t *sm, stEvent_t *event);
static int  running_transition_async_data(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * running_action_data(stStateMachine_t *sm, stEvent_t *event);
static int  running_transition_data(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * running_action_call_api(stStateMachine_t *sm, stEvent_t *event);
static int  running_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret);



static void * wait_action_version_data(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_version_data(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_init_data(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_init_data(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void * wait_action_node_protoinfo(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_node_protoinfo(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void * wait_action_capabilities(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_capabilities(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_controller_capabilities(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_controller_capabilities(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void * wait_action_id(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_id(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void * wait_action_suc_node_id(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_suc_node_id(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_appl_node_information(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_appl_node_information(stStateMachine_t *sm, stEvent_t *event, void *acret);



static void * wait_action_ctr_status(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_ctr_status(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_newdev_added(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_newdev_added(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_cancle_add(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_cancle_add(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_added_node(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_added_node(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_add_comp(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_add_comp(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_cancle_confirm(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_cancle_confirm(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_cancle_comp(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_cancle_comp(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_node_info_ack(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_node_info_ack(stStateMachine_t *sm, stEvent_t *event, void *acret);
static void * wait_action_node_info(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_node_info(stStateMachine_t *sm, stEvent_t *event, void *acret);



static void * wait_action_remove_response(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_remove_response(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_leaving(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_leaving(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_cancle_remove(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_cancle_remove(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_leaved_node_s1(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_leaved_node_s1(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_leaved_node_s2(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_leaved_node_s2(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_leave_comp(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_leave_comp(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void * wait_action_setsuc_response(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_setsuc_response(stStateMachine_t *sm, stEvent_t *event, void *acret);



static void * wait_action_senddata_response(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_senddata_response(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void * wait_action_tx_status(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_tx_status(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void * wait_action_isfailed_response(stStateMachine_t *sm, stEvent_t *event);
static int    wait_transition_isfailed_response(stStateMachine_t *sm, stEvent_t *event, void *acret);


static void *wait_action_remove_failed_response(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_remove_failed_response(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_softreset_response(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_softreset_response(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_protocol_version(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_protocol_version(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_api_started(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_api_started(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_rfpower_level(stStateMachine_t *sm, stEvent_t *event); 
static int wait_transition_rf_power_level(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_neighbor_count(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_neighbor_count(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_are_neighbors(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_are_neighbors(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_type_library(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_type_library(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_protocol_status(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_protocol_status(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_port_status(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_port_status(stStateMachine_t *sm, stEvent_t *event, void *acret);

static void *wait_action_io_port(stStateMachine_t *sm, stEvent_t *event);
static int wait_transition_action_io_port(stStateMachine_t *sm, stEvent_t *event, void *acret);



stStateMachine_t smCmdZWaveGetVersion = {
	1, S_WAIT_VERSION_DATA, S_WAIT_VERSION_DATA, {
		{S_WAIT_VERSION_DATA, 1, NULL, {
				{E_VERSION_DATA, wait_action_version_data, wait_transition_version_data},
			},
		},
	},
};

stStateMachine_t smCmdSerialApiGetInitData = {
	1, S_WAIT_INIT_DATA, S_WAIT_INIT_DATA, {
		{S_WAIT_INIT_DATA, 1, NULL, {
				{E_INIT_DATA, wait_action_init_data, wait_transition_init_data},
			},
		},
	},
};

stStateMachine_t smCmdZWaveGetNodeProtoInfo = {
	1, S_WAIT_NODE_PROTOINFO, S_WAIT_NODE_PROTOINFO, {
		{S_WAIT_NODE_PROTOINFO, 1, NULL, {
				{E_NODE_PROTOINFO, wait_action_node_protoinfo, wait_transition_node_protoinfo},
			},
		},
	},
};

stStateMachine_t smCmdSerialApiGetCapalibities = {
	1, S_WAIT_CAPABILITIES, S_WAIT_CAPABILITIES, {
		{S_WAIT_CAPABILITIES, 1, NULL, {
				{E_CAPABILITIES, wait_action_capabilities, wait_transition_capabilities},
			},
		},
	},
};


stStateMachine_t smCmdZWaveGetControllerCapabilities = {
	1, S_WAIT_CONTROLLER_CAPABILITIES, S_WAIT_CONTROLLER_CAPABILITIES, {
		{S_WAIT_CONTROLLER_CAPABILITIES, 1, NULL, {
				{E_CONTROLLER_CAPABILITIES, wait_action_controller_capabilities, wait_transition_controller_capabilities},
			},
		},
	},
};


stStateMachine_t smCmdMemoryGetId = {
	1, S_WAIT_ID, S_WAIT_ID, {
		{S_WAIT_ID, 1, NULL, {
				{E_ID, wait_action_id,  wait_transition_id},
			},
		},
	},
};


stStateMachine_t smCmdZWaveGetSucNodeId = {
	1, S_WAIT_SUC_NODE_ID, S_WAIT_SUC_NODE_ID, {
		{S_WAIT_SUC_NODE_ID, 1, NULL, {
				{E_SUC_NODE_ID, wait_action_suc_node_id,  wait_transition_suc_node_id},
			},
		},
	},
};


stStateMachine_t smCmdSerialApiApplNodeInformation= {
	1, S_WAIT_APPL_NODE_INFORMATION, S_WAIT_APPL_NODE_INFORMATION, {
		{S_WAIT_APPL_NODE_INFORMATION, 1, NULL, {
				{E_APPL_NODE_INFORMATION, wait_action_appl_node_information,  wait_transition_appl_node_information},
			},
		},
	},
};


stStateMachine_t smCmdZWaveAddNodeToNetWork = {
	6, S_WAIT_CTR_STATUS, S_WAIT_CTR_STATUS, {
		{S_WAIT_CTR_STATUS, 1, NULL, {
				{E_CTR_STATUS, wait_action_ctr_status, wait_transition_ctr_status},
			},
		},
		{S_WAIT_ADDED_OR_CANCLE, 2, NULL, {
				{E_NEWDEV_ADDED, wait_action_newdev_added, wait_transition_newdev_added},
				{E_CANCLE_ADD, wait_action_cancle_add, wait_transition_cancle_add}, /* async cancle api */
			},
		},
		{S_WAIT_ADDED_NODE, 1, NULL, {
				{E_ADDED_NODE, wait_action_added_node, wait_transition_added_node},
			},
		},
		{S_WAIT_ADD_COMP, 1, NULL, {
				{E_ADD_COMP, wait_action_add_comp, wait_transition_add_comp},
			},
		},
		{S_WAIT_CANCLE_CONFIRM, 1, NULL, {
				{E_CANCLE_CONFIRM, wait_action_cancle_confirm, wait_transition_cancle_confirm}, /* async confirm api */
			},
		},
		{S_WAIT_CANCLE_COMP, 1, NULL, {
				{E_CANCLE_COMP, wait_action_cancle_comp, wait_transition_cancle_comp},
			},
		},
	},
};

stStateMachine_t smCmdZWaveRequestNodeInfo = {
	2, S_WAIT_NODE_INFO_ACK, S_WAIT_NODE_INFO_ACK, {
		{S_WAIT_NODE_INFO_ACK, 1, NULL, {
				{E_NODE_INFO_ACK, wait_action_node_info_ack, wait_transition_node_info_ack},
			},
		},
		{S_WAIT_NODE_INFO, 1, NULL, {
				{E_NODE_INFO, wait_action_node_info,  wait_transition_node_info},
			},
		},
	},
};

stStateMachine_t smCmdZWaveRemoveNodeFromNetwork = {
	5, S_WAIT_REMOVE_RESPONSE, S_WAIT_REMOVE_RESPONSE, {
		{S_WAIT_REMOVE_RESPONSE, 1, NULL, {
				{E_REMOVE_RESPONSE, wait_action_remove_response, wait_transition_remove_response},
			},
		},

		{S_WAIT_LEAVE_OR_CANCLE, 2, NULL, {
				{E_LEAVING, wait_action_leaving, wait_transition_leaving},
				{E_CANCLE_REMOVE, wait_action_cancle_remove, wait_transition_cancle_remove},
			},
		},

		{S_WAIT_LEAVED_NODE_S1, 1, NULL, {
				{E_LEAVED_NODE_S1, wait_action_leaved_node_s1, wait_transition_leaved_node_s1},
			},
		},

		{S_WAIT_LEAVED_NODE_S2, 1, NULL, {
				{E_LEAVED_NODE_S2, wait_action_leaved_node_s2, wait_transition_leaved_node_s2},
			},
		},

		{S_WAIT_LEAVE_COMP, 1, NULL, {
				{E_LEAVE_COMP, wait_action_leave_comp, wait_transition_leave_comp},
			},
		},
	},
};

stStateMachine_t smCmdZWaveSetSucNodeId = {
	1, S_WAIT_SETSUC_RESPONSE, S_WAIT_SETSUC_RESPONSE, {
		{S_WAIT_SETSUC_RESPONSE, 1, NULL, {
				{E_SETSUC_RESPONSE, wait_action_setsuc_response, wait_transition_setsuc_response},
			},
		},
	},
};

stStateMachine_t smCmdZWaveSendData = {
	2, S_WAIT_SENDDATA_RESPONSE, S_WAIT_SENDDATA_RESPONSE, {
		{S_WAIT_SENDDATA_RESPONSE, 1, NULL, {
				{E_SENDDATA_RESPONSE, wait_action_senddata_response, wait_transition_senddata_response},
			},
		},
		{S_WAIT_TX_STATUS, 1, NULL, {
				{E_TX_STATUS, wait_action_tx_status, wait_transition_tx_status},
			},
		},
	},
};

stStateMachine_t smCmdZWaveIsFailedNode = {
	1, S_WAIT_ISFAILED_RESPONSE, S_WAIT_ISFAILED_RESPONSE, {
		{S_WAIT_ISFAILED_RESPONSE, 1, NULL, {
				{E_ISFAILED_RESPONSE, wait_action_isfailed_response, wait_transition_isfailed_response},
			},
		},
	},
};





stStateMachine_t smCmdZWaveRemoveFailedNodeId = {
	1, S_WAIT_REMOVE_FAILED_RESPONSE, S_WAIT_REMOVE_FAILED_RESPONSE, {
		{S_WAIT_REMOVE_FAILED_RESPONSE, 1, NULL, {
				{E_REMOVE_FAILED_RESPONSE, wait_action_remove_failed_response, wait_transition_remove_failed_response},
			},
		},
	},
};
stStateMachine_t smCmdSerialApiSoftReset = {
	0, S_WAIT_SOFTRESET_RESPONSE, S_WAIT_SOFTRESET_RESPONSE, {
		{S_WAIT_SOFTRESET_RESPONSE, 1, NULL, {
				{E_SOFTRESET_RESPONSE, wait_action_softreset_response, wait_transition_softreset_response},
			},
		},
	},
};
stStateMachine_t smCmdZWaveGetProtocolVersion = {
	1, S_WAIT_PROTOCOL_VERSION, S_WAIT_PROTOCOL_VERSION, {
		{S_WAIT_PROTOCOL_VERSION, 1, NULL, {
				{E_PROTOCOL_VERSION, wait_action_protocol_version, wait_transition_protocol_version},
			},
		},
	},
};
stStateMachine_t smCmdSerialApiStarted = {
	1, S_WAIT_API_STARTED, S_WAIT_API_STARTED, {
		{S_WAIT_API_STARTED, 1, NULL, {
				{E_API_STARTED, wait_action_api_started, wait_transition_api_started},
			},
		},
	},
};
stStateMachine_t smCmdZWaveRfPowerLevelGet = {
	1, S_WAIT_RFPOWER_LEVEL, S_WAIT_RFPOWER_LEVEL, {
		{S_WAIT_RFPOWER_LEVEL, 1, NULL, {
				{E_RFPOWER_LEVEL, wait_action_rfpower_level, wait_transition_rf_power_level},
			},
		},
	},

};
stStateMachine_t smCmdZWaveGetNeighborCount = {
	1, S_WAIT_NEIGHBOR_COUNT, S_WAIT_NEIGHBOR_COUNT, {
		{S_WAIT_NEIGHBOR_COUNT, 1, NULL, {
				{E_NEIGHBOR_COUNT, wait_action_neighbor_count, wait_transition_neighbor_count},
			},
		},
	},

};
stStateMachine_t smCmdZWaveAreNodesNeighbours = {
	1, S_WAIT_ARE_NEIGHBORS, S_WAIT_ARE_NEIGHBORS, {
		{S_WAIT_ARE_NEIGHBORS, 1, NULL, {
				{E_ARE_NEIGHBORS, wait_action_are_neighbors, wait_transition_are_neighbors},
			},
		},
	},

};
stStateMachine_t smCmdZWaveTypeLibrary = {
	1, S_WAIT_TYPE_LIBRARY, S_WAIT_TYPE_LIBRARY, {
		{S_WAIT_TYPE_LIBRARY, 1, NULL, {
				{E_TYPE_LIBRARY, wait_action_type_library, wait_transition_type_library},
			},
		},
	},

};
stStateMachine_t smCmdZWaveGetProtocolStatus = {
	1, S_WAIT_PROTOCOL_STATUS, S_WAIT_PROTOCOL_STATUS, {
		{S_WAIT_PROTOCOL_STATUS, 1, NULL, {
				{E_PROTOCOL_STATUS, wait_action_protocol_status, wait_transition_protocol_status},
			},
		},
	},

};
stStateMachine_t smCmdIoPortStatus = {
	1, S_WAIT_PORT_STATUS, S_WAIT_PORT_STATUS, {
		{S_WAIT_PORT_STATUS, 1, NULL, {
				{E_PORT_STATUS, wait_action_port_status, wait_transition_port_status},
			},
		},
	},

};
stStateMachine_t smCmdIoPort = {
	1, S_WAIT_IO_PORT, S_WAIT_IO_PORT, {
		{S_WAIT_IO_PORT, 1, NULL, {
				{E_IO_PORT, wait_action_io_port, wait_transition_action_io_port},
			},
		},
	},

};


stStateMachine_t smApi = {
	2, S_IDLE, S_IDLE, {

		{S_IDLE, 3, NULL, {
				{E_BEAT, idle_action_beat, NULL},
				{E_CALL_API, idle_action_call_api, idle_transition_call_api},
				{E_ASYNC_DATA, idle_action_async_data, NULL},
			},
		},

		{S_RUNNING, 5, NULL, {	
				{E_ERROR, running_action_error, running_transition_error}, //nak can timeout
				{E_ACK, running_action_ack, running_transition_ack},
				{E_ASYNC_DATA, running_action_async_data, running_transition_async_data},
				{E_DATA, running_action_data, running_transition_data},
				{E_CALL_API, running_action_call_api, running_transition_call_api},
			},
		}, 

	},
};

static struct stApiMachinePair {
	emApi_t api;
	stStateMachine_t *machine;
} amps[] = {
	{CmdZWaveGetVersion, &smCmdZWaveGetVersion},
	{CmdSerialApiGetInitData, &smCmdSerialApiGetInitData},
	{CmdZWaveGetNodeProtoInfo, &smCmdZWaveGetNodeProtoInfo},
	{CmdSerialApiGetCapabilities, &smCmdSerialApiGetCapalibities},
	{CmdZWaveGetControllerCapabilities, &smCmdZWaveGetControllerCapabilities},
	{CmdMemoryGetId, &smCmdMemoryGetId},
	{CmdZWaveGetSucNodeId, &smCmdZWaveGetSucNodeId},
	{CmdSerialApiApplNodeInformation, &smCmdSerialApiApplNodeInformation},
	{CmdZWaveAddNodeToNetwork, &smCmdZWaveAddNodeToNetWork},
	{CmdZWaveRequestNodeInfo, &smCmdZWaveRequestNodeInfo},
	{CmdZWaveRemoveNodeFromNetwork, &smCmdZWaveRemoveNodeFromNetwork},
	{CmdZWaveSetSucNodeId, &smCmdZWaveSetSucNodeId},
	{CmdZWaveSendData, &smCmdZWaveSendData},
	{CmdZWaveIsFailedNode, &smCmdZWaveIsFailedNode},

	{CmdZWaveRemoveFailedNodeId, &smCmdZWaveRemoveFailedNodeId},
	{CmdSerialApiSoftReset, &smCmdSerialApiSoftReset},
	{CmdZWaveGetProtocolVersion, &smCmdZWaveGetProtocolVersion},
	{CmdSerialApiStarted, &smCmdSerialApiStarted},
	{CmdZWaveRfPowerLevelGet, &smCmdZWaveRfPowerLevelGet},
	{CmdZWaveGetNeighborCount, &smCmdZWaveGetNeighborCount},
	{CmdZWaveAreNodesNeighbours, &smCmdZWaveAreNodesNeighbours},
	{CmdZWaveTypeLibrary, &smCmdZWaveTypeLibrary},
	{CmdZWaveGetProtocolStatus, &smCmdZWaveGetProtocolStatus},
	{CmdIoPortStatus, &smCmdIoPortStatus},
	{CmdIoPort, &smCmdIoPort},
};

static stStateMachine_t* api_id_to_state_machine(emApi_t api) {
#if 0
	if (api == CmdZWaveGetVersion) {
		return &smCmdZWaveGetVersion;
	} else if (api == CmdSerialApiGetInitData) {
		return &smCmdSerialApiGetInitData;
	} else if (api == CmdZWaveGetNodeProtoInfo) {
		return &smCmdZWaveGetNodeProtoInfo;
	} else if (api == CmdSerialApiGetCapabilities) {
		return &smCmdSerialApiGetCapalibities;
	} else if (api == CmdZWaveGetControllerCapabilities) {
		return &smCmdZWaveGetControllerCapabilities;
	} else if (api == CmdMemoryGetId) {
		return &smCmdMemoryGetId;
	} else if (api == CmdZWaveGetSucNodeId) {
		return &smCmdZWaveGetSucNodeId;
	} else if (api == CmdSerialApiApplNodeInformation) {
		return &smCmdSerialApiApplNodeInformation;
	} else if (api == CmdZWaveAddNodeToNetwork) {
		return &smCmdZWaveAddNodeToNetWork;
	} else if (api == CmdZWaveRequestNodeInfo) {
		return &smCmdZWaveRequestNodeInfo;
	} else if (api == CmdZWaveRemoveNodeFromNetwork) {
		return &smCmdZWaveRemoveNodeFromNetwork;
	} else if (api == CmdZWaveSetSucNodeId) {
		return &smCmdZWaveSetSucNodeId;
	} else if (api == CmdZWaveSendData) {
		return &smCmdZWaveSendData;
	} else if (api == CmdZWaveIsFailedNode) {
		return &smCmdZWaveIsFailedNode;
	}
#else
	int i = 0;
	for (i = 0; i < sizeof(amps)/sizeof(amps[0]); i++) {
		if (api == amps[i].api) {
			return amps[i].machine;
		}
	}
#endif
	return NULL;
}
static int api_state_machine_to_id(void *sm) {
#if 0
	if (sm == &smCmdZWaveGetVersion) {
		return CmdZWaveGetVersion;
	} else if (sm == &smCmdSerialApiGetInitData) {
		return CmdSerialApiGetInitData;
	} else if (sm == &smCmdZWaveGetNodeProtoInfo) {
		return CmdZWaveGetNodeProtoInfo;
	} else if (sm == &smCmdSerialApiGetCapalibities) {
		return CmdSerialApiGetCapabilities;
	} else if (sm == &smCmdZWaveGetControllerCapabilities) {
		return CmdZWaveGetControllerCapabilities;
	} else if (sm == &smCmdMemoryGetId) {
		return CmdMemoryGetId;
	} else if (sm == &smCmdZWaveGetSucNodeId) {
		return CmdZWaveGetSucNodeId;
	} else if (sm == &smCmdSerialApiApplNodeInformation) {
		return CmdSerialApiApplNodeInformation;
	} else if (sm == &smCmdZWaveAddNodeToNetWork) {
		return CmdZWaveAddNodeToNetwork;
	} else if (sm == &smCmdZWaveRequestNodeInfo) {
		return CmdZWaveRequestNodeInfo;
	} else if (sm == &smCmdZWaveRemoveNodeFromNetwork) {
		return CmdZWaveRemoveNodeFromNetwork;
	} else if (sm == &smCmdZWaveSetSucNodeId) {
		return CmdZWaveSetSucNodeId;
	} else if (sm == &smCmdZWaveSendData) {
		return CmdZWaveSendData;
	} else if (sm == &smCmdZWaveIsFailedNode) {
		return CmdZWaveIsFailedNode;
	}
#else
	int i = 0;
	for (i = 0; i < sizeof(amps)/sizeof(amps[0]); i++) {
		if (sm == amps[i].machine) {
			return amps[i].api;
		}
	}
#endif
	return -1;
}
static bool api_async_call_api(stStateMachine_t *sm, stEvent_t *event, int *sid) {
	stState_t * state = state_machine_search_state(&smApi, *sid);
	if (state != NULL) {
		stStateMachine_t *sm = (stStateMachine_t*)state->param;
		if (sm != NULL) {
			int api = api_state_machine_to_id(sm);
			switch (api) {
				case CmdZWaveGetVersion:
				break;
				case CmdSerialApiGetInitData:
				break;
				case CmdZWaveGetNodeProtoInfo:
				break;
				case CmdSerialApiGetCapabilities:
				break;
				case CmdZWaveGetControllerCapabilities:
				break;
				case CmdMemoryGetId:
				break;
				case CmdZWaveGetSucNodeId:
				break;
				case CmdSerialApiApplNodeInformation:
				break;
				case CmdZWaveAddNodeToNetwork:
				{
					//stDataFrame_t *df = (stDataFrame_t *)event->param;
					stApiCall_t * ac = (stApiCall_t*)event->param;
					if (ac->api == CmdZWaveAddNodeToNetwork && (
								sm->state == S_WAIT_ADDED_OR_CANCLE ||
								sm->state == S_WAIT_CANCLE_CONFIRM  ||
								sm->state == S_WAIT_CANCLE_COMP)) {
						return  true;
					}
				}
				break;
				case CmdZWaveRequestNodeInfo:
				break;
				case CmdZWaveRemoveNodeFromNetwork:
				{
					//stDataFrame_t *df = (stDataFrame_t *)event->param;
					stApiCall_t * ac = (stApiCall_t*)event->param;
					if (ac->api == CmdZWaveRemoveNodeFromNetwork && (
								sm->state == S_WAIT_LEAVE_OR_CANCLE ||
								sm->state == S_WAIT_LEAVE_COMP) ) {
						return  true;
					}
				}
				break;
				case CmdZWaveSetSucNodeId:
				break;
				case CmdZWaveSendData:
				break;
				case CmdZWaveIsFailedNode:
				break;

				case CmdZWaveRemoveFailedNodeId:
				break;
				case	CmdSerialApiSoftReset:
				break;
				case CmdZWaveGetProtocolVersion:
				break;
				case CmdSerialApiStarted:
				break;
				case CmdZWaveRfPowerLevelGet:
				break;
				case CmdZWaveGetNeighborCount:
				break;
				case CmdZWaveAreNodesNeighbours:
				break;
				case CmdZWaveTypeLibrary:
				break;
				case CmdZWaveGetProtocolStatus:
				break;
				case CmdIoPortStatus:
				break;
				case CmdIoPort:
				break;

			}
		}
	}
	return false;
}

static int api_data_event_id_step(stStateMachine_t *sm, int id) {
	int sid = state_machine_get_state(&smApi);
	stState_t * state = state_machine_search_state(&smApi, sid);
	if (state != NULL) {
		stStateMachine_t *sm = (stStateMachine_t*)state->param;
		if (sm != NULL) {
			int api = api_state_machine_to_id(sm);
			switch (api) {
				case CmdZWaveGetVersion:
				if (sm->state == S_WAIT_VERSION_DATA && id == E_DATA) {
					return E_VERSION_DATA;
				}
				break;
				case CmdSerialApiGetInitData:
				if (sm->state == S_WAIT_INIT_DATA && id == E_DATA) {
					return E_INIT_DATA;
				}
				break;
				case CmdZWaveGetNodeProtoInfo:
				if (sm->state == S_WAIT_NODE_PROTOINFO && id == E_DATA) {
					return E_NODE_PROTOINFO;
				}
				break;
				case CmdSerialApiGetCapabilities:
				if (sm->state == S_WAIT_CAPABILITIES && id == E_DATA) {
					return E_CAPABILITIES;
				}
				break;
				case CmdZWaveGetControllerCapabilities:
				if (sm->state == S_WAIT_CONTROLLER_CAPABILITIES && id == E_DATA) {
					return E_CONTROLLER_CAPABILITIES;
				}
				break;
				case CmdMemoryGetId:
				if (sm->state == S_WAIT_ID && id == E_DATA) {
					return E_ID;
				}
				break;
				case CmdZWaveGetSucNodeId:
				if (sm->state == S_WAIT_SUC_NODE_ID && id == E_DATA) {
					return E_SUC_NODE_ID;
				}
				break;
				case CmdSerialApiApplNodeInformation:
				if (sm->state == S_WAIT_APPL_NODE_INFORMATION && id == E_DATA) {
					return E_APPL_NODE_INFORMATION;
				}
				break;
				case CmdZWaveAddNodeToNetwork:
				log_debug("-- 1");
				if (sm->state == S_WAIT_CTR_STATUS && id == E_DATA) {
				log_debug("-- 2");
					return E_CTR_STATUS;
				} else if (sm->state == S_WAIT_ADDED_OR_CANCLE && id == E_DATA) {
				log_debug("-- 3");
					return E_NEWDEV_ADDED;
				} else if (sm->state == S_WAIT_ADDED_NODE && id == E_DATA) {
				log_debug("-- 4");
					return E_ADDED_NODE;
				} else if (sm->state == S_WAIT_ADD_COMP && id == E_DATA) {
				log_debug("-- 5");
					return E_ADD_COMP;
				}
				break;
				case CmdZWaveRequestNodeInfo:
				if (sm->state == S_WAIT_NODE_INFO_ACK && id == E_DATA) {
					return E_NODE_INFO_ACK;
				} else if (sm->state == S_WAIT_NODE_INFO && id == E_DATA) {
					return E_NODE_INFO;
				}
				break;
				case CmdZWaveRemoveNodeFromNetwork:
				if (sm->state == S_WAIT_REMOVE_RESPONSE && id == E_DATA) {
					return E_REMOVE_RESPONSE;
				} else if (sm->state == S_WAIT_LEAVE_OR_CANCLE && id == E_DATA) {
					return E_LEAVING;
				} else if (sm->state == S_WAIT_LEAVED_NODE_S1 && id == E_DATA) {
					return E_LEAVED_NODE_S1;
				} else if (sm->state == S_WAIT_LEAVED_NODE_S2 && id == E_DATA) {
					return E_LEAVED_NODE_S2;
				}
				break;
				case CmdZWaveSetSucNodeId:
				if (sm->state == S_WAIT_SETSUC_RESPONSE && id == E_DATA) {
					return E_SETSUC_RESPONSE;
				}
				break;
				case CmdZWaveSendData:
				if (sm->state == S_WAIT_SENDDATA_RESPONSE && id == E_DATA) {
					return E_SENDDATA_RESPONSE;
				} else if (sm->state == S_WAIT_TX_STATUS && id == E_DATA) {
					return E_TX_STATUS;
				}
				break;
				case CmdZWaveIsFailedNode:
				if (sm->state == S_WAIT_ISFAILED_RESPONSE && id == E_DATA) {
					return E_ISFAILED_RESPONSE;
				}
				break;

				case CmdZWaveRemoveFailedNodeId:
				if (sm->state == S_WAIT_REMOVE_FAILED_RESPONSE && id == E_DATA) {
					return E_REMOVE_FAILED_RESPONSE;
				}
	
				break;
				case	CmdSerialApiSoftReset:
				if (sm->state == S_WAIT_SOFTRESET_RESPONSE && id == E_DATA) {
					return E_SOFTRESET_RESPONSE;
				}
	
				break;
				case CmdZWaveGetProtocolVersion:
				if (sm->state == S_WAIT_PROTOCOL_VERSION && id == E_DATA) {
					return E_PROTOCOL_VERSION;
				}
	
				break;
				case CmdSerialApiStarted:
				if (sm->state == S_WAIT_API_STARTED && id == E_DATA) {
					return E_API_STARTED;
				}
	
				break;
				case CmdZWaveRfPowerLevelGet:
				if (sm->state == S_WAIT_RFPOWER_LEVEL && id == E_DATA) {
					return E_RFPOWER_LEVEL;
				}
	
				break;
				case CmdZWaveGetNeighborCount:
				if (sm->state == S_WAIT_NEIGHBOR_COUNT && id == E_DATA) {
					return E_NEIGHBOR_COUNT;
				}
	
				break;
				case CmdZWaveAreNodesNeighbours:
				if (sm->state == S_WAIT_ARE_NEIGHBORS && id == E_DATA) {
					return E_ARE_NEIGHBORS;
				}
	
				break;
				case CmdZWaveTypeLibrary:
				if (sm->state == S_WAIT_TYPE_LIBRARY && id == E_DATA) {
					return E_TYPE_LIBRARY;
				}
	
				break;
				case CmdZWaveGetProtocolStatus:
				if (sm->state == S_WAIT_TYPE_LIBRARY && id == E_DATA) {
					return E_TYPE_LIBRARY;
				}
	
				break;
				case CmdIoPortStatus:
				if (sm->state == S_WAIT_PORT_STATUS && id == E_DATA) {
					return E_PORT_STATUS;
				}
	
				break;
				case CmdIoPort:
				if (sm->state == S_WAIT_IO_PORT && id == E_DATA) {
					return E_IO_PORT;
				}
	
				break;
			}
		}
	}
	return -1;
}

static int api_ack_event_id_step(stStateMachine_t *sm, int id) {
	int sid = state_machine_get_state(&smApi);
	stState_t * state = state_machine_search_state(&smApi, sid);
	if (state != NULL) {
		stStateMachine_t *sm = (stStateMachine_t*)state->param;
		if (sm != NULL) {
			int api = api_state_machine_to_id(sm);
			switch (api) {
				case CmdZWaveGetVersion:
				case CmdSerialApiGetInitData:
				case CmdZWaveGetNodeProtoInfo:
				case CmdSerialApiGetCapabilities:
				case CmdZWaveGetControllerCapabilities:
				case CmdMemoryGetId:
				case CmdZWaveGetSucNodeId:
				case CmdSerialApiApplNodeInformation:
				break;
				case CmdZWaveAddNodeToNetwork:
				switch (sm->state) {
					case S_WAIT_ADDED_OR_CANCLE:
						return E_CANCLE_ADD;
					break;
					case S_WAIT_CANCLE_CONFIRM:
						return E_CANCLE_CONFIRM;
					break;
					case S_WAIT_CANCLE_COMP:
						return E_CANCLE_COMP;
					break;
					default:
					break;
				}
				break;
				case CmdZWaveRequestNodeInfo:
				break;
				case CmdZWaveRemoveNodeFromNetwork:
				switch (sm->state) {
					case S_WAIT_LEAVE_COMP:
						return E_LEAVE_COMP;
					break;
				}
				break;
				case CmdZWaveSetSucNodeId:
				break;
				case CmdZWaveSendData:
				break;
				case CmdZWaveIsFailedNode:
				break;

				case CmdZWaveRemoveFailedNodeId:
				break;
				case	CmdSerialApiSoftReset:
				break;
				case CmdZWaveGetProtocolVersion:
				break;
				case CmdSerialApiStarted:
				break;
				case CmdZWaveRfPowerLevelGet:
				break;
				case CmdZWaveGetNeighborCount:
				break;
				case CmdZWaveAreNodesNeighbours:
				break;
				case CmdZWaveTypeLibrary:
				break;
				case CmdZWaveGetProtocolStatus:
				break;
				case CmdIoPortStatus:
				break;
				case CmdIoPort:
				break;
			}
		}
	}
	return -1;
}



static bool api_is_async_data(stDataFrame_t *df) {
	int sid = state_machine_get_state(&smApi);
	stState_t * state = state_machine_search_state(&smApi, sid);

	if (sid == S_RUNNING && state != NULL) {
		stStateMachine_t *sm = (stStateMachine_t*)state->param;
		if (sm != NULL) {
			if (api_state_machine_to_id(sm) == df->cmd /* && seq == seq */) {
				log_debug_hex("sync data :",df->payload, df->size);
				return false;
			} else if (api_state_machine_to_id(sm) == CmdZWaveRequestNodeInfo && df->cmd == CmdApplicationControllerUpdate) {
				log_debug_hex("sync data :",df->payload, df->size);
				return false;
			}
		}
	}

	//log_debug("async data(state api id:%d, frame api id:%d)", api_state_machine_to_id(sm), df->cmd);
	log_debug("frame api id:%d", df->cmd);
	log_debug_hex("async data :",df->payload, df->size);
	return true;
}




static stDataFrame_t * make_frame(emApi_t api, stParam_t *param, int paramSize) {
	//static int seq = 0;

	int size = sizeof(stDataFrame_t) + paramSize;

	stDataFrame_t *df = MALLOC(size);
	if (df == NULL) {
		return NULL;
	}

	df->sof			= SOF_CHAR;
	df->len			= paramSize+3;
	df->type		= 0x00,
	df->cmd			= api;
	df->payload = (char *)(df + 1);
	df->size		= paramSize;
	if (df->size > 0) {
		memcpy(df->payload, param, paramSize);
	}
	df->checksum = 0;
	df->timestamp = time(NULL);
	df->checksum_cal = 0;
	df->do_checksum = 0;
	df->trycnt =0;
	df->error = 0;
	
	return df;
}


static int api_post_beat_event() {
	stEvent_t *e = (stEvent_t*)MALLOC(sizeof(stEvent_t));
	e->eid = E_BEAT;
	e->param = NULL;
	lockqueue_push(&env.qSend, e);
	return 0;
}

static int api_post_api_call_event(emApi_t api, stParam_t *param, int param_size) {

	stEvent_t *e = (stEvent_t*)MALLOC(sizeof(stEvent_t) + sizeof(stApiCall_t));
	if (e == NULL) {
		log_debug("not enough memory!");
		return -1;
	}
	e->eid = E_CALL_API;
	e->param = e+1;

	stApiCall_t *ac = (stApiCall_t*)e->param;
	ac->api = api;
	ac->param_size = param_size;
	if (ac->param_size > 0) {
		memcpy(&ac->param, param, sizeof(stParam_t));
	} 

	lockqueue_push(&env.qSend, e);
	log_debug("api queue size is %d", lockqueue_size(&env.qSend));
	return 0;
}

static int api_backup_api_call_event(stEvent_t *event) {

	int size = sizeof(stEvent_t) + sizeof(stApiCall_t);
	stEvent_t *e = MALLOC(size);
	if (e == NULL) {
		log_debug("out of memory!");
		return 0;
	}
	memcpy(e, event, size);
	e->param = e + 1;

	lockqueue_push(&env.qSendBack, e);
	log_debug("api backup queue size is %d", lockqueue_size(&env.qSendBack));
	return 0;
}


static int api_restore_api_call_event() {
	stEvent_t *e = NULL;
	if (lockqueue_pop(&env.qSendBack, (void**)&e) && e != NULL) {
		log_debug("[%s] event : %d", __func__, e->eid);

		lockqueue_push(&env.qSend, e);
	}
	return 0;
}

static int api_post_apicall_over_event(int eid, stDataFrame_t *df) {
	stEvent_t *e = (stEvent_t*)MALLOC(sizeof(stEvent_t) + sizeof(stDataFrame_t) + df->len);
	if (e == NULL) {
		log_debug("not enough memory!");
		return -1;
	}
	e->eid = eid;
	e->param = e+1;

	stDataFrame_t *ac = (stDataFrame_t*)e->param;
	memcpy(ac, df, sizeof(stDataFrame_t));

	ac->payload = (char *)(ac + 1);
	if (df->size > 0) {
		memcpy(ac->payload, df->payload, df->size);
	}

	lockqueue_push(&env.qSend, e);

	return 0;
}

static int api_post_recv_over_event(int eid, stDataFrame_t *df) {
	stEvent_t *e = (stEvent_t*)MALLOC(sizeof(stEvent_t) + sizeof(stDataFrame_t) + df->len);
	if (e == NULL) {
		log_debug("not enough memory!");
		return -1;
	}
	e->eid = eid;
	e->param = (void *)(e+1);

	stDataFrame_t *ac = (stDataFrame_t*)e->param;
	memcpy(ac, df, sizeof(stDataFrame_t));

	ac->payload = (char *)(ac + 1);
	if (df->size > 0) {
		memcpy(ac->payload, df->payload, df->size);
	}

	lockqueue_push(&env.qRecv, e);
	//lockqueue_push(&env.qSend, e);

	return 0;

}

static void api_print_state_info() {
	#if 0
	int sid = state_machine_get_state(&smApi);

	int ssid = -1;
	stState_t * state = state_machine_search_state(&smApi, sid);
	if (state != NULL) {
		stStateMachine_t *sm = (stStateMachine_t*)state->param;
		if (sm != NULL) {
			ssid = state_machine_get_state(sm);
		}
	}

	log_debug("**>main state is : [%d], sub api state is [%d]<**", sid, ssid);
	#endif
}

static bool handlerOneEvent() {
	stEvent_t *event = NULL;
	api_print_state_info();

	if (lockqueue_pop(&env.qRecv, (void **)&event)) {
		if (event != NULL) {
			log_debug("event id %d from RecvQueue", event->eid);
			state_machine_step(&smApi, event);
			FREE(event);
			event = NULL;
			return true;
		}
	} else if (lockqueue_pop(&env.qSend, (void **)&event)) {
		if (event != NULL) {
			log_debug("event id %d from SendQueue", event->eid);
			state_machine_step(&smApi, event);
			FREE(event);
			event = NULL;
			return true;
		}
	}

	return false;
}


static void api_beat(int flag) {
	timer_cancel(env.th, &env.timerBeat);
	timer_set(env.th, &env.timerBeat, 1);
	if (flag) {
		handlerOneEvent();
	}
}


static void api_send_over(void *_df) {
	stDataFrame_t *df = (stDataFrame_t*)_df;

	log_info("[%s]---->:", __func__);
	if (df != NULL) {
		log_info("size is %02x, %02x, trycnt:%d, error:%d", df->size, df->len, df->trycnt, df->error);
		log_debug("type : %02X, cmd : %02x", df->type, df->cmd);
		log_debug_hex("send payload:", df->payload, df->size);

		if (df->error == FE_NONE) {
			/* never go here */
			log_debug("never go here: FE_NONE");
		} else if (df->error == FE_SEND_TIMEOUT) {
			log_debug("send timeout !");
			api_post_apicall_over_event(E_ERROR, df);
			/* timeout , send timeout */
			api_post_apicall_over_event(E_ERROR, df);
		} else if (df->error == FE_SEND_ACK) {
				api_post_apicall_over_event(E_ACK, df);
		} else if (df->error == FE_SEND_NAK) {
			/* nan , send nak */
			api_post_apicall_over_event(E_ERROR, df);
		} else if (df->error == FE_SEND_CAN) {
			/* can , send can */
			api_post_apicall_over_event(E_ERROR, df);
		} else if (df->error == FE_RECV_CHECKSUM) {
			log_debug("never go here: FE_RECV_CHECKSUM");
		} else if (df->error == FE_RECV_TIMEOUT) {
			log_debug("never go here: FE_RECV_TIMEOUT");
		}
		FREE(df);
	}
}

static void api_recv_over(void *_df) {
	stDataFrame_t *df = (stDataFrame_t*)_df;
	log_info("[%s]---->:", __func__);

	if (df != NULL) {
		log_debug("type : %02X, cmd : %02x", df->type, df->cmd);
		log_debug_hex("recv payload:", df->payload, df->size);

		if (df->error == FE_NONE) {
			/* receive ok */
			if (api_is_async_data(df)) {
				api_post_recv_over_event(E_ASYNC_DATA, df);
			} else {
				api_post_recv_over_event(E_DATA, df);
			}
		} else if (df->error == FE_SEND_TIMEOUT) {
			/* never go here */
			log_debug("never go here: FE_SEND_TIMEOUT");
		} else if (df->error == FE_SEND_ACK) {
			/* never go here */
			log_debug("never go here: FE_SEND_ACK");
		} else if (df->error == FE_SEND_NAK) {
			/* never go here */
			log_debug("never go here: FE_SEND_NAK");
		} else if (df->error == FE_SEND_CAN) {
			/* never go here */
			log_debug("never go here: FE_SEND_CAN");
		} else if (df->error == FE_RECV_CHECKSUM) {
			/* receive checksum error */
			log_debug("frame recv checksum error: %02x(correct:%02x)",
								df->checksum, 
								df->checksum_cal);
			//api_post_recv_over_event(E_ASYNC_DATA, df);
		} else if (df->error == FE_RECV_TIMEOUT) {
			/* receive timeout */
			log_debug("frame recv timeout!");
			//api_post_recv_over_event(E_ASYNC_DATA, df);
		}

		FREE(df);
	}
}

static void api_send_timer_callback(struct timer *timer) {
	state_machine_reset(&smApi);
	api_post_beat_event();
	api_beat(0);
}


static void api_beat_timer_callback(struct timer *timer) {
	//stEvent_t *event = NULL;
	if (handlerOneEvent()) {
		api_beat(0);
	}
	/*
	//while (lockqueue_pop(&env.qSend, (void **)&event)) {
	if (lockqueue_pop(&env.qSend, (void **)&event)) {
		if (event != NULL) {
			log_debug("event id %d", event->eid);
			state_machine_step(&smApi, event);
			FREE(event);
			event = NULL;
			api_beat(0);
		}
	}
	*/
}



///////////////////////////////////////////////////////////////////////////////
int api_init(void *_th, API_CALL_CALLBACK _accb, API_RETURN_CALLBACK _arcb) {
	api_ccb = _accb;
	api_crb = _arcb;

	if (session_init(_th, api_send_over, api_recv_over) != 0) {
		log_debug("frame init failed!");
		return -1;
	}

	lockqueue_init(&env.qSend);
	lockqueue_init(&env.qSendBack);
	lockqueue_init(&env.qRecv);

	env.apicall = NULL;

	env.th = (struct timer_head *)_th;
  timer_init(&env.timerSend, api_send_timer_callback);
	timer_init(&env.timerBeat, api_beat_timer_callback);

	env.initFlag = 1;

	return 0;
}
int api_free() {
	session_free();

	lockqueue_destroy(&env.qSend, NULL);
	lockqueue_destroy(&env.qSendBack, NULL);
	lockqueue_destroy(&env.qRecv, NULL);
	timer_cancel(env.th, &env.timerSend);
	timer_cancel(env.th, &env.timerBeat);

	env.initFlag = 0;
	
	return 0;
}
int api_getfd() {
	return session_getfd();
}
int api_step() {
	session_receive_step();
	api_beat(1);
	return 0;
} 
int api_call(emApi_t api, stParam_t *param, int param_size) {
	api_post_api_call_event(api, param, param_size);

	api_beat(0);
	return 0;
}

int api_app_init_over_fetch(void *_ae) {
	stAppEnv_t *ae = (stAppEnv_t*)_ae;
	
	memcpy(&ae->ver,				&env.ver,				sizeof(env.ver));
	memcpy(&ae->initdata,		&env.initdata,	sizeof(env.initdata));
	memcpy(&ae->caps,				&env.caps,			sizeof(env.caps));
	memcpy(&ae->id,					&env.id,				sizeof(env.id));
	memcpy(&ae->sucid,			&env.sucid,			sizeof(env.sucid));
	return 0;
}

///////////////////////////////////////////////////////////////////////////
static void * idle_action_beat(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}


static void * idle_action_call_api(stStateMachine_t *sm, stEvent_t *event) {

	log_debug("------[%s]------", __func__);
	
	if (state_machine_get_state(sm) != S_IDLE) {
		log_debug("state machine busy!");
		return NULL;
	}

	if (event == NULL) {
		log_debug("api call event null!");
		return NULL;
	}

	if (event->eid != E_CALL_API) {
		log_debug("api call event id error : %d, correct:%d", event->eid, E_CALL_API);
		return NULL;
	}
	stApiCall_t * ac = (stApiCall_t*)event->param;

	stDataFrame_t * df = make_frame(ac->api, &ac->param, ac->param_size);
	if (df == NULL) {
		log_debug("api call make frame error !");
		return NULL;
	}
	//frame_send(df);
	session_send(df);

	//Fixed here to a correct one
	//timer_set(env.th, &env.timerSend, API_EXEC_TIMEOUT_MS);

	return (void *)S_RUNNING;
}
static int    idle_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	if (acret == NULL) {
		return S_IDLE;
	} 
	
	return (int)acret;
}

static void * idle_action_async_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);

	stDataFrame_t *df = event->param;
	if (df->cmd == 0x04) { //class get, ApplicationCommandHandler
		char rxStatus = df->payload[0];
		char sourceNode = df->payload[1];
		char len = df->payload[2];
		char cid = df->payload[3];
		char op = df->payload[4];
		char *value = &df->payload[5];
		int value_len = len -2;
		if (rxStatus == 0) { //binary class report
			class_cmd_save(sourceNode, cid, op, value, value_len);
		}
	}

	return NULL;
}

static void * running_action_error(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);
	return NULL;
}
static int  running_transition_error(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	api_restore_api_call_event();
	api_post_beat_event();
	api_beat(1);
	return S_IDLE;
}

static void * running_action_ack(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);

	int sid = state_machine_get_state(sm);
	stState_t * state = state_machine_search_state(sm, sid);
	if (state != NULL) {
		stStateMachine_t *smapi = (stStateMachine_t*)state->param;
		if (smapi != NULL) {
			event->eid = api_ack_event_id_step(sm, event->eid);
			state_machine_step(smapi, event);
			int s = state_machine_get_state(smapi);
			if (s == S_END) {
				api_restore_api_call_event();
				api_post_beat_event();
				api_beat(0);
				state->param = NULL;
				return (void*)S_IDLE;
			}
			return (void *)S_RUNNING;
		} else {
			stDataFrame_t *df = (stDataFrame_t*)event->param;
			state->param = (void*)api_id_to_state_machine(df->cmd);
			if (state->param != NULL) {
				state_machine_reset((stStateMachine_t*)state->param);
			}
		}
	}
	return NULL;
}
static int  running_transition_ack(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	if (acret == NULL) {
		int sid = state_machine_get_state(sm);
		stState_t * state = state_machine_search_state(sm, sid);
		if (state == NULL || state->numevent == 0) {
			return S_IDLE;
		}
		stStateMachine_t *smapi = (stStateMachine_t*)state->param;
		if (smapi == NULL || smapi->numstate == 0) {
			return S_IDLE;
		}
		return S_RUNNING;
	}
	return (int)acret;
}

static void * running_action_async_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);
	stDataFrame_t *df = event->param;
	if (df->cmd == 0x04) { //class get, ApplicationCommandHandler
		char rxStatus = df->payload[0];
		char sourceNode = df->payload[1];
		char len = df->payload[2];
		char cid = df->payload[3];
		char op = df->payload[4];
		char *value = &df->payload[5];
		int value_len = len -2;
		if (rxStatus == 0) { //binary class report
			class_cmd_save(sourceNode, cid, op, value, value_len);
		}
	}

	return NULL;
}
static int  running_transition_async_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("------[%s]------", __func__);
	return S_RUNNING;
}

static void * running_action_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);

	int sid = state_machine_get_state(sm);
	stState_t * state = state_machine_search_state(sm, sid);
	if (state != NULL) {
		stStateMachine_t *smapi = (stStateMachine_t*)state->param;
		if (smapi != NULL) {
			event->eid = api_data_event_id_step(sm, event->eid);
			state_machine_step(smapi, event);
			int s = state_machine_get_state(smapi);
			if (s == S_END) {
				api_restore_api_call_event();
				api_post_beat_event();
				api_beat(0);
				state->param = NULL;
				return (void*)S_IDLE;
			}
		} else {
			log_debug("what ? null api state ? ... funck !!!");
			api_restore_api_call_event();
			api_post_beat_event();
			api_beat(0);
			return (void*)S_IDLE;
		}
	} else {
		log_debug("what ? null state ?...fuck!!!!");
		api_restore_api_call_event();
		api_post_beat_event();
		api_beat(0);
		return (void*)S_IDLE;
	}
	return (void*)S_RUNNING;
}
static int  running_transition_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("------[%s]------%d", __func__, (int)acret);
	return (int)acret;
}

static void * running_action_call_api(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);

	int sid = state_machine_get_state(&smApi);
	if(!api_async_call_api(sm, event, &sid)) {
		api_backup_api_call_event(event);
	} else {
		if (event->eid != E_CALL_API) {
			log_debug("api call event id error : %d, correct:%d", event->eid, E_CALL_API);
		}
		stApiCall_t * ac = (stApiCall_t*)event->param;

		stDataFrame_t * df = make_frame(ac->api, &ac->param, ac->param_size);
		if (df == NULL) {
			log_debug("api call make frame error !");
		}
		//frame_send(df);
		session_send(df);
	}
	return (void*)sid;
}

static int  running_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return (int)(acret);
}


/* CmdZWaveGetVersion */
static void * wait_action_version_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);

	stVersion_t *ver = &env.ver;
	stDataFrame_t *df = event->param;

	if (df == NULL) {
		log_debug("CmdZWaveGet Version : null data!");
		return NULL;
	}

	strcpy(ver->ver, df->payload);
	ver->type = df->payload[strlen(ver->ver) + 1];

	log_debug(":%p, ver:%s, type:%02x", ver, ver->ver, ver->type);	
	
	return NULL;
}
static int    wait_transition_version_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

/* CmdSerialApiGetInitData */
static void * wait_action_init_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-----------", __func__);

	stInitData_t *id= &env.initdata;
	stDataFrame_t *df = event->param;

	if (df == NULL) {
		log_debug("CmdSerialApiGetInitData : null data!");
		return NULL;
	}

	id->ver = df->payload[0];
	id->capabilities = df->payload[1];
	id->nodes_map_size = df->payload[2];
	if (id->nodes_map_size > 0) {
		memcpy(id->nodes_map, &df->payload[3], id->nodes_map_size);
	}
	id->chip_type = df->payload[id->nodes_map_size +  3];
	id->chip_version = df->payload[id->nodes_map_size + 4];

	return NULL;
}
static int    wait_transition_init_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

/* CmdZWaveGetNodeProtoInfo */
static void * wait_action_node_protoinfo(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_node_protoinfo(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


/* CmdSerialApiGetCapabilities */
static void * wait_action_capabilities(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);

	stCapabilities_t *capa= &env.caps;
	stDataFrame_t *df = event->param;

	if (df == NULL) {
		log_debug("CmdSerialApiGetCapabilities : null data!");
		return NULL;
	}

	capa->AppVersion = df->payload[0];
	capa->AppRevisioin = df->payload[1];
	capa->ManufacturerId = df->payload[2]*256 + df->payload[3];
	capa->ManufactureProductType = df->payload[4]*256 + df->payload[5];
	capa->ManufactureProductId = df->payload[6]*256 + df->payload[7];
	memcpy(capa->SupportedFuncIds_map, &df->payload[8], sizeof(capa->SupportedFuncIds_map));

	return NULL;
}
static int    wait_transition_capabilities(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


/* CmdZWaveGetControllerCapabilities */
static void * wait_action_controller_capabilities(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_controller_capabilities(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


/* CmdMemoryGetId */
static void * wait_action_id(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);

	stId_t *i= &env.id;
	stDataFrame_t *df = event->param;

	if (df == NULL) {
		log_debug("CmdMemoryGetId : null data!");
		return NULL;
	}

	i->HomeID = *(int*)&df->payload[0];
	i->NodeID = df->payload[4];

	return NULL;
}
static int    wait_transition_id(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


/* CmdZWaveGetSucNodeId */
static void * wait_action_suc_node_id(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);

	stSucNodeId_t *sni = &env.sucid;
	stDataFrame_t *df = event->param;
	if (df == NULL) {
		log_debug("CmdZWaveGetSucNodeId : null data!");
		return NULL;
	}

	sni->SUCNodeID = df->payload[0];

	return NULL;
}
static int    wait_transition_suc_node_id(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);

	app_util_push_msg(E_INIT_OVER, NULL, 0);
	
	return S_END;
}


/* CmdSerialApiApplNodeInformation */
static void * wait_action_appl_node_information(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_appl_node_information(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

/* CmdZWaveAddNodeToNetwork */
static void * wait_action_ctr_status(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_ctr_status(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_ADDED_OR_CANCLE;
}

static void * wait_action_newdev_added(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_newdev_added(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_ADDED_NODE;
}

static void * wait_action_cancle_add(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_cancle_add(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_CANCLE_CONFIRM;
}

static void * wait_action_added_node(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_added_node(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_ADD_COMP;
}

static void * wait_action_add_comp(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_add_comp(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void * wait_action_cancle_confirm(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_cancle_confirm(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_CANCLE_COMP;
}

static void * wait_action_cancle_comp(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_cancle_comp(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void * wait_action_node_info_ack(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_node_info_ack(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return S_WAIT_NODE_INFO;
}

static void * wait_action_node_info(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	stDataFrame_t *df = event->param;

	if (df == NULL) {
		log_debug("CmdSerialApiGetCapabilities : null data!");
		return NULL;
	}

	stNodeInfo_t ni;

	ni.bStatus = df->payload[0];
	ni.bNodeID = df->payload[1];
	ni.len = df->payload[2];
	if (ni.len > 0) {
		ni.basic = df->payload[3];
		ni.generic = df->payload[4];
		ni.specific = df->payload[5];
		memcpy(ni.commandclasses, df->payload+6, ni.len - 3);
	}

	app_util_push_msg(E_CLASS_STEP, &ni, sizeof(ni));

	return NULL;
}
static int    wait_transition_node_info(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


/* CmdZWaveRemoveNodeFromNetwork */

static void * wait_action_remove_response(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_remove_response(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_LEAVE_OR_CANCLE;
}

static void * wait_action_leaving(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_leaving(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_LEAVED_NODE_S1;
}

static void * wait_action_cancle_remove(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_cancle_remove(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void * wait_action_leaved_node_s1(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_leaved_node_s1(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_LEAVED_NODE_S2;
}

static void * wait_action_leaved_node_s2(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_leaved_node_s2(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	char buf[1] ={ 0x05};
	api_call(CmdZWaveRemoveNodeFromNetwork, (stParam_t*)buf, 1);
	return S_WAIT_LEAVE_COMP;
}

static void * wait_action_leave_comp(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_leave_comp(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

/* CmdZWaveSetSucNodeId */
static void * wait_action_setsuc_response(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_setsuc_response(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void * wait_action_senddata_response(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_senddata_response(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_WAIT_TX_STATUS;
}

static void * wait_action_tx_status(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	stDataFrame_t * df = (stDataFrame_t *)event->param;
	if (df->payload[0] == 0x03) { //
	} else if (df->payload[0] == 0x04) {
	} else if (df->payload[0] == 0x05) {
	}
	return NULL;
}
static int    wait_transition_tx_status(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


static void * wait_action_isfailed_response(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int    wait_transition_isfailed_response(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}


static void *wait_action_remove_failed_response(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_remove_failed_response(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_softreset_response(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_softreset_response(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_protocol_version(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_protocol_version(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_api_started(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_api_started(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_rfpower_level(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_rf_power_level(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_neighbor_count(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_neighbor_count(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_are_neighbors(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_are_neighbors(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_type_library(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_type_library(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_protocol_status(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_protocol_status(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_port_status(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_port_status(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

static void *wait_action_io_port(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
static int wait_transition_action_io_port(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}





////////////////////////////////////////////////////////////////////////////////////////////////
/*
int ZW_AddNodeToNetwork();
int ZW_AreNodesNeighbours();
int ZW_EnableSUC();
int ZW_GetControllerCapabilities();
int ZW_GetLastWorkingRoute();
int ZW_SetLastWorkingRoute();
int ZW_GetNeighborCount();
int ZW_GetNodeProtocolInfo();
int ZW_GetProtocolStatus();
int ZW_GetProtocolVersion();
int ZW_GetRandomWord();
int ZW_GetSUCNodeID();
int ZW_Version();
int ZW_isFailedNode();
int ZW_IsPrimaryCtrl();
int ZW_RemoveNodeFromNetwork();
int ZW_RequestNetWorkUpdate();
int ZW_RequestNodeInfo();
int ZW_RequestNodeNeighborUpdate();
int ZW_RFPowerLevelGet();
int ZW_RFPowerLevelSet();
int ZW_SendData();
int ZW_SendDataAbort();
int ZW_SendData_Bridge();
int ZW_SendDataMeta();
int ZW_SendDataMeta_Bridge();
int ZW_SendDataMulti();
int ZW_SendDataMulti_Bridge();
int ZW_SendNodeInformation();
int ZW_SendSUCID();
int ZW_SetDefault();
int ZW_SetLearnMode();
int ZW_SetLearnNodeState();
int ZW_SetSlaveLearnMode();
int ZW_SetSUCNodeID();
int ZW_Support9600Only();
int ZW_Type_Library();
*/

#endif



