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
};

/* state machine relative */
enum {
	S_IDLE = 0,
	S_RUNNING = 1,

	S_WAIT_INIT_DATA = 2,
	S_WAIT_VERSION_DATA = 3,
	S_WAIT_NODE_PROTOINFO = 4,
	S_WAIT_CAPABILITIES = 5,


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
};

void * idle_action_beat(stStateMachine_t *sm, stEvent_t *event);

void * idle_action_call_api(stStateMachine_t *sm, stEvent_t *event);
int    idle_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret);

void * idle_action_async_data(stStateMachine_t *sm, stEvent_t *event);

void * running_action_error(stStateMachine_t *sm, stEvent_t *event);
int  running_transition_error(stStateMachine_t *sm, stEvent_t *event, void *acret);

void * running_action_ack(stStateMachine_t *sm, stEvent_t *event);
int  running_transition_ack(stStateMachine_t *sm, stEvent_t *event, void *acret);

void * running_action_async_data(stStateMachine_t *sm, stEvent_t *event);
int  running_transition_async_data(stStateMachine_t *sm, stEvent_t *event, void *acret);

void * running_action_data(stStateMachine_t *sm, stEvent_t *event);
int  running_transition_data(stStateMachine_t *sm, stEvent_t *event, void *acret);

void * running_action_call_api(stStateMachine_t *sm, stEvent_t *event);
int  running_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret);



void * wait_action_version_data(stStateMachine_t *sm, stEvent_t *event);
int    wait_transition_version_data(stStateMachine_t *sm, stEvent_t *event, void *acret);

void * wait_action_init_data(stStateMachine_t *sm, stEvent_t *event);
int    wait_transition_init_data(stStateMachine_t *sm, stEvent_t *event, void *acret);


void * wait_action_node_protoinfo(stStateMachine_t *sm, stEvent_t *event);
int    wait_transition_node_protoinfo(stStateMachine_t *sm, stEvent_t *event, void *acret);


void * wait_action_capabilities(stStateMachine_t *sm, stEvent_t *event);
int    wait_transition_capabilities(stStateMachine_t *sm, stEvent_t *event, void *acret);


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


static stStateMachine_t* api_id_to_state_machine(emApi_t api) {
	if (api == CmdZWaveGetVersion) {
		return &smCmdZWaveGetVersion;
	} else if (api == CmdSerialApiGetInitData) {
		return &smCmdSerialApiGetInitData;
	} else if (api == CmdZWaveGetNodeProtoInfo) {
		return &smCmdZWaveGetNodeProtoInfo;
	} else if (api == CmdSerialApiGetCapabilities) {
		return &smCmdSerialApiGetCapalibities;
	}
	return NULL;
}
static int api_state_machine_to_id(void *sm) {
	if (sm == &smCmdZWaveGetVersion) {
		return CmdZWaveGetVersion;
	} else if (sm == &smCmdSerialApiGetInitData) {
		return CmdSerialApiGetInitData;
	} else if (sm == &smCmdZWaveGetNodeProtoInfo) {
		return CmdZWaveGetNodeProtoInfo;
	} else if (sm == &smCmdSerialApiGetCapalibities) {
		return CmdSerialApiGetCapabilities;
	}
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
			}
		}
	}
	return -1;
}


static bool api_is_async_data(stDataFrame_t *df) {
	int sid = state_machine_get_state(&smApi);
	stState_t * state = state_machine_search_state(&smApi, sid);
	if (state != NULL) {
		stStateMachine_t *sm = (stStateMachine_t*)state->param;
		if (sm != NULL) {
			if (api_state_machine_to_id(sm) != df->cmd /* && seq == seq */) {
				log_debug("async data(state api id:%d, frame api id:%d)", api_state_machine_to_id(sm), df->cmd);
				log_debug_hex("async data :",df->payload, df->size);
				return true;
			}
		}
	}
	log_debug("sync data.");
	//log_debug_hex("sync data :",df->payload, df->len);
	
	return false;
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
	if (df->len > 0) {
		memcpy(ac->payload, df->payload, df->len);
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
	if (df->len > 0) {
		memcpy(ac->payload, df->payload, df->len);
	}

	lockqueue_push(&env.qSend, e);

	return 0;

}

static bool handlerOneEvent() {
	stEvent_t *event = NULL;
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
///////////////////////////////////////////////////////////////////////////
void * idle_action_beat(stStateMachine_t *sm, stEvent_t *event) {
	return NULL;
}


void * idle_action_call_api(stStateMachine_t *sm, stEvent_t *event) {

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
	frame_send(df);

	timer_set(env.th, &env.timerSend, API_EXEC_TIMEOUT_MS);

	return (void *)S_RUNNING;
}
int    idle_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	if (acret == NULL) {
		return S_IDLE;
	} 
	
	return (int)acret;
}

void * idle_action_async_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);
	return NULL;
}

void * running_action_error(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);
	return NULL;
}
int  running_transition_error(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	api_restore_api_call_event();
	api_post_beat_event();
	api_beat(1);
	return S_IDLE;
}

void * running_action_ack(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);

	int sid = state_machine_get_state(sm);
	stState_t * state = state_machine_search_state(sm, sid);
	if (state != NULL) {
		stDataFrame_t *df = (stDataFrame_t*)event->param;
		state->param = (void*)api_id_to_state_machine(df->cmd);
		state_machine_reset((stStateMachine_t*)state->param);
	}
	return NULL;
}
int  running_transition_ack(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	int sid = state_machine_get_state(sm);
	stState_t * state = state_machine_search_state(sm, sid);
	if (state == NULL || state->numevent == 0) {
		return S_IDLE;
	}
	return S_RUNNING;
}

void * running_action_async_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);
	return NULL;
}
int  running_transition_async_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("------[%s]------", __func__);
	return S_RUNNING;
}

void * running_action_data(stStateMachine_t *sm, stEvent_t *event) {
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
int  running_transition_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("------[%s]------%d", __func__, (int)acret);
	return (int)acret;
}

void * running_action_call_api(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("------[%s]------", __func__);

	int sid = state_machine_get_state(&smApi);
	if(!api_async_call_api(sm, event, &sid)) {
		api_backup_api_call_event(event);
	}

	return (void*)sid;
}

int  running_transition_call_api(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return (int)(acret);
}


/* CmdZWaveGetVersion */
void * wait_action_version_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
int    wait_transition_version_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	log_debug("----------[%s]-..----------", __func__);
	return S_END;
}

/* CmdSerialApiGetInitData */
void * wait_action_init_data(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-----------", __func__);
	return NULL;
}
int    wait_transition_init_data(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return S_END;
}

/* CmdZWaveGetNodeProtoInfo */
void * wait_action_node_protoinfo(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
int    wait_transition_node_protoinfo(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return S_END;
}


/* CmdSerialApiGetCapabilities */
void * wait_action_capabilities(stStateMachine_t *sm, stEvent_t *event) {
	log_debug("----------[%s]-..----------", __func__);
	return NULL;
}
int    wait_transition_capabilities(stStateMachine_t *sm, stEvent_t *event, void *acret) {
	return S_END;
}



#endif



