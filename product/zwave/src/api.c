#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "frame.h"
#include "api.h"
#include "log.h"
#include "common.h"

#include "session.h"
#include "lockqueue.h"

typedef struct stApiEnv {
	stLockQueue_t qSend;
	stApiCall_t apicall;
	int initFlag;
}stApiEnv_t;

static API_CALL_CALLBACK api_ccb = NULL;
static API_RETURN_CALLBACK api_crb = NULL;


static stApiEnv_t env = {
	.qSend = {},
	.apicall = {},
	.initFlag = 0,
};


static stApiStateMachine_t asms[] = {
	[CmdZWaveGetVersion] = {
		CmdZWaveGetVersion, "CmdZWaveGetVersion", 0, 2, {
			{CmdZWaveGetVersion_parse_input, CmdZWaveGetVersion_view_input},
			{CmdZWaveGetVersion_parse_version},
		},
	},
	[CmdSerialApiGetInitData] = {
		CmdSerialApiGetInitData, "CmdSerialApiGetInitData", 0, 2, {
			{CmdSerialApiGetInitData_parse_input}，
			{CmdSerialApiGetInitData_parse_initdata},
		},
	},
	[CmdZWaveGetNodeProtoInfo] = {
		CmdZWaveGetNodeProtoInfo, "CmdZWaveGetNodeProtoInfo", sizeof(stNodeProtoInfoIn_t), 2, {
			{CmdZWaveGetNodeProtoInfo_parse_input},
			{CmdZWaveGetNodeProtoInfo_parse_nodeprotoinfo},
		},
	},
	[CmdZWaveGetControllerCapabilities] = {
			CmdZWaveGetControllerCapabilities, "CmdZWaveGetControllerCapabilities", 0, 3, {
			{CmdZWaveGetControllerCapabilities_parse_input},
			{CmdZWaveGetControllerCapabilities_parse_capalities},
			{CmdZWaveGetControllerCapabilities_parse_controllercapalities},
		},
	},
	[CmdMemoryGetId] = {
		CmdMemoryGetId, "CmdMemoryGetId", 0, 2, {
			{CmdMemoryGetId_parse_input},
			{CmdMemoryGetId_parse_id},
		},
	},
	[CmdZWaveGetSucNodeId] ={
		CmdZWaveGetSucNodeId, "CmdZWaveGetSucNodeId", 0, 2, {
			{CmdZWaveGetSucNodeId_parse_input},
			{CmdZWaveGetSucNodeId_parse_sucnodeid},
		},
	},
	[CmdSerialApiApplNodeInformation] = {
		CmdSerialApiApplNodeInformation, "CmdSerialApiApplNodeInformation", 0, 2, {
			{CmdSerialApiApplNodeInformation_parse_input},
			{CmdSerialApiApplNodeInformation_parse_nodeinformation},
		},
	},

	[CmdZWaveAddNodeToNetwork] = {
		CmdZWaveAddNodeToNetwork, "CmdZWaveAddNodeToNetwork", 0, 2, {
			{CmdZWaveAddNodeToNetwork_parse_input},
			{CmdZWaveAddNodeToNetwork_parse_addnodetonetwork},
		}
	},

};

void api_param_view(emApi_t api, stParam_t *param, emApiState_t state) {
	int num_state = asms[api].num_state;
	if (state > num_state || state < 1) {
		log_debug("api state error!");
		return;
	}

	void (*view)(emApi_t,stParam_t*) = asms[api].states[state].view;
	if (view != NULL) {
		view(param);
	}
}

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
}

static void api_start() {
	if (env.apicall != NULL) {
		return;
	}

	lockqueue_pop(&env.qSend, &env.apicall);
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
}

static void send_over(stDataFrame_t *df) {
	log_debug("%s send over!", asms[df->cmd].name);

	if (df != NULL) {
		if (df->error == FE_NONE) {
			/* never go here */
			log_debug("never go here: FE_NONE");
		} else if (df->error == FE_SEND_TIMEOUT) {
			/* timeout , send timeout */
			if (api_ccb != NULL) {
				api_ccb(df->cmd, asms[df->cmd].states[0].parse(df), AE_SEND_TIMEOUT);
			}
			api_end();
			api_start();
		} else if (df->error == FE_SEND_ACK) {
			/* ack , send ok*/
			if (api_ccb != NULL) {
				api_ccb(df->cmd, asms[df->cmd].states[0].parse(df), AE_NONE);
			}
		} else if (df->error == FE_SEND_NAK) {
			/* nan , send nak */
			if (api_ccb != NULL) {
				api_ccb(df->cmd, asms[df->cmd].states[0].parse(df), AE_NAK);
			}
			api_end();
			api_start();
		} else if (df->error == FE_SEND_CAN) {
			/* can , send can */
			if (api_ccb != NULL) {
				api_ccb(df->cmd, asms[df->cmd].states[0].parse(df), AE_CAN);
			}
			api_end();
			api_start();
		} else if (df->error == FE_RECV_CHECKSUM) {
			log_debug("never go here: FE_RECV_CHECKSUM");
		} else if (df->error == FE_RECV_TIMEOUT) {
			log_debug("never go here: FE_RECV_TIMEOUT");
		}
		log_info("size is %02x, %02x, trycnt:%d", df->size, df->len, df->trycnt);
		log_debug("type : %02X, cmd : %02x", df->type, df->cmd);
		log_debug_hex("send payload:", df->payload, df->size);

		FREE(df);
	}
}

static void recv_over(stDataFrame_t *df) {
	log_debug("%s recv over!", cmdStringMap[df->cmd]);

	if (df != NULL) {
		if (df->error == FE_NONE) {
			/* receive ok */
			if (api_crb != NULL) {
				api_crb(df->cmd, asms[df->cmd].states[env.apicall->state].parse(df), AE_NONE);
				if (env.apicall->state == asms[df->cmd].states[env.apicall].num_state) {
					api_end();
					api_send();
				}
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
				api_crb(df->cmd, parse_param(df), AE_CHECKSUM);
			}
		} else if (df->error == FE_RECV_TIMEOUT) {
			/* receive timeout */
			if (api_crb != NULL) {
				api_crb(df->cmd, parse_param(df), AE_RECV_TIMEOUT);
			}
		}

		log_debug("type : %02X, cmd : %02x", df->type, df->cmd);
		log_debug_hex("recv payload:", df->payload, df->size);
		FREE(df);
	}
}


//////////////////////////////////////////////
int api_init(void *_th, API_CALL_CALLBACK _accb, API_RETURN_CALLBACK _arcb) {
	
	if (_accb != NULL) {
		api_ccb = _accb;
	}
	if (_arcb != NULL) {
		api_crb = _arcb;
	}

	if (session_init(_th, send_over, recv_over) != 0) {
		log_debug("frame init failed!");
		return -1;
	}

	lockqueue_init(&env.qSend);
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

