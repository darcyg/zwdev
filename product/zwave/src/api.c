#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "frame.h"
#include "api.h"
#include "log.h"
#include "common.h"

#include "session.h"

static API_CALL_CALLBACK api_ccb = NULL;
static API_RETURN_CALLBACK api_crb = NULL;


static const char *cmdStringMap[256] =  {
	[CmdZWaveGetVersion] = "CmdZWaveGetVersion", 

	[CmdSerialApiGetInitData] = "CmdSerialApiGetInitData",

	[CmdZWaveGetNodeProtoInfo] = "CmdZWaveGetNodeProtoInfo",

	[CmdSerialApiGetCapabilities] = "CmdSerialApiGetCapabilities",

	[CmdZWaveGetControllerCapabilities] = "CmdZWaveGetControllerCapabilities",

	[CmdMemoryGetId] = "CmdMemoryGetId",

	[CmdZWaveGetSucNodeId] = "CmdZWaveGetSucNodeId",

	[CmdSerialApiApplNodeInformation] = "CmdSerialApiApplNodeInformation",

	[CmdZWaveAddNodeToNetwork] = "CmdZWaveAddNodeToNetwork",

	[CmdZWaveRequestNodeInfo] = "CmdZWaveRequestNodeInfo",

	[CmdApplicationControllerUpdate] = "CmdApplicationControllerUpdate",

	[CmdZWaveRemoveNodeFromNetwork] = "CmdZWaveRemoveNodeFromNetwork",

	[CmdZWaveSetSucNodeId] = "CmdZWaveSetSucNodeId",

	[CmdZWaveSendData] = "CmdZWaveSendData",

	[CmdZWaveSendDataAbort] = "CmdZWaveSendDataAbort",

	[CmdZWaveIsFailedNode] = "CmdZWaveIsFailedNode",

	[CmdZWaveReplaceFailedNode] = "CmdZWaveReplaceFailedNode",

};

static int inParamSizeMap[256] = {
	[CmdZWaveGetVersion] = 0, 

	[CmdSerialApiGetInitData] = 0,

	[CmdZWaveGetNodeProtoInfo] = sizeof(stNodeProtoInfoIn_t),

	[CmdSerialApiGetCapabilities] = 0,

	[CmdZWaveGetControllerCapabilities] = 0,

	[CmdMemoryGetId] = 0,

	[CmdZWaveGetSucNodeId] = 0,

	[CmdSerialApiApplNodeInformation] = 0,

	[CmdZWaveAddNodeToNetwork] = sizeof(stAddNodeToNetworkIn_t),

	[CmdZWaveRequestNodeInfo] = sizeof(stNodeInfoIn_t),

	[CmdApplicationControllerUpdate] = sizeof(stControlleUpdateIn_t),

	[CmdZWaveRemoveNodeFromNetwork] = sizeof(stRemoveNodeFromNetworkIn_t),

	[CmdZWaveSetSucNodeId] = sizeof(stRemoveNodeFromNetworkIn_t),

	[CmdZWaveSendData] = sizeof(stSendDataIn_t),

	[CmdZWaveSendDataAbort] = 0,

	[CmdZWaveIsFailedNode] = sizeof(stIsFailedNodeIn_t),

	[CmdZWaveReplaceFailedNode] = sizeof(stReplaceFailedNodeIn_t),
};


void api_param_view(emApi_t api, stParam_t *param, emApiState_t state) {
	switch (api) {
	case CmdZWaveGetVersion:
		switch (state) {
		case AS_Recv: {
				stVersion_t *ver = (stVersion_t*)param;
				log_debug("version:%s,type:%d", ver->ver, ver->type);
			} break;
		default:
			break;
		}
		break;
	}
}

static stParam_t* parse_param(stDataFrame_t* df) {
	switch (df->cmd) {
	case CmdZWaveGetVersion:
		switch (df->type) {
		case 0:
			return NULL;
		case 1: {
			stVersion_t *ver = MALLOC(sizeof(stVersion_t));
			if (ver != NULL) {
				strcpy(ver->ver, df->payload);
				ver->type = df->payload[strlen(ver->ver) + 1];
			}
			return (stParam_t*)ver;}
		default:
			break;
		}
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
		break;
	case CmdZWaveRequestNodeInfo:
		break;
	case CmdApplicationControllerUpdate:
		break;
	case CmdZWaveRemoveNodeFromNetwork:
		break;
	case CmdZWaveSetSucNodeId:
		break;
	case CmdZWaveSendData:
		break;
	case CmdZWaveSendDataAbort:
		break;
	case CmdZWaveIsFailedNode:
		break;
	case CmdZWaveReplaceFailedNode:
		break;
	default:
		break;
	}
	return NULL;
}

static stDataFrame_t * make_frame(emApi_t api, stParam_t *param) {
	//static int seq = 0;
	int size = sizeof(stDataFrame_t) + inParamSizeMap[api];

	stDataFrame_t *df = MALLOC(size);
	if (df == NULL) {
		return NULL;
	}

	df->sof			= SOF_CHAR;
	df->len			= inParamSizeMap[api]+3;
	df->type		= 0x00,
	df->cmd			= api;
	df->payload = (char *)(df + 1);
	df->size		= inParamSizeMap[api];
	if (inParamSizeMap[api] > 0) {
		memcpy(df->payload, param, inParamSizeMap[api]);
	}
	df->checksum = 0;
	df->timestamp = time(NULL);
	df->checksum_cal = 0;
	df->do_checksum = 0;
	df->trycnt =0;
	df->error = 0;
	
	return df;
}

static void send_over(stDataFrame_t *df) {
	log_debug("%s send over!", cmdStringMap[df->cmd]);

	if (df != NULL) {
		if (df->error == FE_NONE) {
			/* never go here */
			log_debug("never go here: FE_NONE");
		} else if (df->error == FE_SEND_TIMEOUT) {
			/* timeout , send timeout */
			if (api_ccb != NULL) {
				api_ccb(df->cmd, parse_param(df), AE_SEND_TIMEOUT);
			}
		} else if (df->error == FE_SEND_ACK) {
			/* ack , send ok*/
			if (api_ccb != NULL) {
				api_ccb(df->cmd, parse_param(df), AE_NONE);
			}
		} else if (df->error == FE_SEND_NAK) {
			/* nan , send nak */
			if (api_ccb != NULL) {
				api_ccb(df->cmd, parse_param(df), AE_NAK);
			}
		} else if (df->error == FE_SEND_CAN) {
			/* can , send can */
			if (api_ccb != NULL) {
				api_ccb(df->cmd, parse_param(df), AE_CAN);
			}
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
				api_crb(df->cmd, parse_param(df), AE_NONE);
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

	if (frame_init(_th, send_over, recv_over) != 0) {
		log_debug("frame init failed!");
		return -1;
	}

	return 0;
}

int api_exec(emApi_t api, stParam_t *param) {
	stDataFrame_t * df = make_frame(api, param);
	if (df == NULL) {
		log_debug("make frame failed!");
		return -1;
	}
	
	return frame_send(df);
}

int api_free() {
	frame_free();
	return 0;
}

int api_getfd() {
	return frame_getfd();
}

int api_step() {
	session_receive_step();
	return 0;
}


const char *api_name(emApi_t api) {
	return cmdStringMap[api];
}

