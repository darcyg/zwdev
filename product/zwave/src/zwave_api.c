#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"

#include "log.h"
#include "file_event.h"

#include "frame.h"
#include "zwave_api.h"


extern int zwave_async_data(stDataFrame_t *dfr);

static unsigned char g_funcID = 1;

static unsigned char geneFuncID() {
	unsigned char ret = g_funcID;
	
	g_funcID++;
	if (g_funcID == 0) {
		g_funcID = 1;
	}
	
	return ret;
}

static int zwave_frame_send_with_ack(stDataFrame_t *dfs, int timeout) {
	int try_cnt = 0;
	int ret = 0;

re_0:
	if (frame_send(dfs, timeout) != 0) {
		return APIERR_FRAME_SEND_ERR;
	}

	stDataFrame_t *dfr = NULL;
re_1:
	ret = frame_recv(&dfr, timeout);
	if (ret == FR_ACK) {
		log_debug("ACK");
		return APIERR_NONE;
	}

	if (ret == FR_OK) { /* normal frame , async */
		if (dfr != NULL) {
			log_debug_hex("async data:", dfr->payload, dfr->size);
			zwave_async_data(dfr);
			FREE(dfr); dfr = NULL;
		}
		goto re_1;
	} 

	if (ret == FR_NAK) {
		return APIERR_FRAME_RECV_NAK;
	} 

	if (ret == FR_CAN) {
		return APIERR_FRAME_RECV_CAN;
	} 

	if (ret == FR_TIMEOUT) {
		if (try_cnt++ < 3) {
			goto re_0;
		} 
		return APIERR_FRAME_RECV_TOU;
	}

	return APIERR_UNKNOWN;
}

static int zwave_frame_wait_frame(stDataFrame_t**dfr, int timeout, char expectcmd) {
	int ret = 0;

re_2:
	ret = frame_recv(dfr, timeout);
	if (ret == FR_ACK) { /* delayed ack, wait for next frame continully */
		goto re_2;
	} 

	if (ret == FR_OK) { /* except frame */
		log_debug_hex("recv frame:", (*dfr)->payload, (*dfr)->size);

		if ((*dfr)->error != FE_NONE) {
			log_debug("[%d] : response frame error: %d",  __LINE__, (*dfr)->error);
			FREE(*dfr); *dfr = NULL;
			return APIERR_FRAME_RECV_CHK;
		}

		if (expectcmd != 0x00 && ((*dfr)->cmd&0xff) != (expectcmd&0xff)) {
			log_debug_hex("async data:", (*dfr)->payload, (*dfr)->size);
			zwave_async_data(*dfr);
			FREE(*dfr); *dfr = NULL;
			goto re_2;
		}
		
		return APIERR_NONE;
	}

	if (ret == FR_NAK) {
		return APIERR_FRAME_RECV_NAK;
	} 

	if (ret == FR_CAN) {
		return APIERR_FRAME_RECV_CAN;
	} 

	if (ret == FR_TIMEOUT) {
		return APIERR_FRAME_RECV_TOU;
	}

	return APIERR_UNKNOWN;
}

int zwave_api_ZWaveGetVersion(stVersion_t *ver) {
	stDataFrame_t *dfs = frame_make(CmdZWaveGetVersion, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveGetVersion);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response data: %d",  __LINE__, ret);
		return -2;
	}
	
	if (ver != NULL) {
		strcpy(ver->ver, dfr->payload);
		ver->type = dfr->payload[strlen(ver->ver) + 1];
		log_debug("ver:%s, type:%02x", ver->ver, ver->type);	
	}

	FREE(dfr); dfr = NULL;

	return 0;
}

int zwave_api_SerialApiGetInitData(stInitData_t *id) {
	stDataFrame_t *dfs = frame_make(CmdSerialApiGetInitData, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdSerialApiGetInitData);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response data: %d",  __LINE__, ret);
		return -2;
	}

	if (id != NULL) {
		id->ver = dfr->payload[0];
		id->capabilities = dfr->payload[1];
		id->nodes_map_size = dfr->payload[2];
		if (id->nodes_map_size > 0) {
			memcpy(id->nodes_map, &dfr->payload[3], id->nodes_map_size);
		}
		id->chip_type = dfr->payload[id->nodes_map_size +  3];
		id->chip_version = dfr->payload[id->nodes_map_size + 4];

		log_debug("ver:%02x, cap:%02x, nmapsize:%02x, chip_type:%02x, chip_version:%02x", 
				id->ver, id->capabilities, id->nodes_map_size, id->chip_type,
				id->chip_version);

		log_debug_hex("nmap:", id->nodes_map, id->nodes_map_size);
	}

	FREE(dfr); dfr = NULL;

	return 0;
}

int zwave_api_SerialApiGetCapabilities(stCapabilities_t *capa) {
	stDataFrame_t *dfs = frame_make(CmdSerialApiGetCapabilities, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdSerialApiGetCapabilities);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response data: %d",  __LINE__, ret);
		return -2;
	}
	
	if (capa != NULL) {
		capa->AppVersion = dfr->payload[0];
		capa->AppRevisioin = dfr->payload[1];
		capa->ManufacturerId = dfr->payload[2]*256 + dfr->payload[3];
		capa->ManufactureProductType = dfr->payload[4]*256 + dfr->payload[5];
		capa->ManufactureProductId = dfr->payload[6]*256 + dfr->payload[7];
		memcpy(capa->SupportedFuncIds_map, &dfr->payload[8], sizeof(capa->SupportedFuncIds_map));

		log_debug("AppVersion:%02x, AppReVersion:%02x, ManufacturerId:%04x,"
							"ManufacturerProductType:%04x, ManufacturerProductId:%04x",
							capa->AppVersion, capa->AppRevisioin, capa->ManufacturerId,
							 capa->ManufactureProductType, capa->ManufactureProductId);
		log_debug_hex("supportted funcs:", capa->SupportedFuncIds_map, sizeof(capa->SupportedFuncIds_map));
	}

	FREE(dfr); dfr = NULL;

	return 0;
}
int zwave_api_MemoryGetId(stId_t *i) {
	stDataFrame_t *dfs = frame_make(CmdMemoryGetId, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdMemoryGetId);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response data: %d",  __LINE__, ret);
		return -2;
	}

	if (i != NULL) {
		i->HomeID = *(int*)&dfr->payload[0];
		i->NodeID = dfr->payload[4];
		log_debug("HomeId:%08x, NodeId:%02x", i->HomeID, i->NodeID);
	}

	FREE(dfr); dfr = NULL;

	return 0;
}

int zwave_api_ZWaveGetSucNodeId(stSucNodeId_t *sni) {
	stDataFrame_t *dfs = frame_make(CmdZWaveGetSucNodeId, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveGetSucNodeId);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response data: %d",  __LINE__, ret);
		return -2;
	}


	if (sni != NULL) {
		sni->SUCNodeID = dfr->payload[0];
		log_debug("suc node id:%02x", sni->SUCNodeID);
	}

	FREE(dfr); dfr = NULL;
	
	return 0;
}

int zwave_api_ZWaveGetNodeProtoInfo(char nodeid, stNodeProtoInfo_t *npi) {
	stNodeProtoInfoIn_t npii = {nodeid};
	stDataFrame_t *dfs = frame_make(CmdZWaveGetNodeProtoInfo, (void*)&npii, sizeof(npii));
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveGetNodeProtoInfo);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response data: %d",  __LINE__, ret);
		return -2;
	}

	if (npi != NULL) {
		npi->Capability = dfr->payload[0];
		npi->Security = dfr->payload[1];
		npi->Basic = dfr->payload[2];
		npi->Generic = dfr->payload[3];
		npi->Specific = dfr->payload[4];
	}

	FREE(dfr); dfr = NULL;
	return 0;
}

int zwave_api_ZWaveRequestNodeInfo(int id, stNodeInfo_t *ni) {
	log_info("[%d]", __LINE__);

	stNodeInfoIn_t npii = {id&0xff};
	stDataFrame_t *dfs = frame_make(CmdZWaveRequestNodeInfo, (void*)&npii, sizeof(npii));
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	/* ack */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveRequestNodeInfo);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response ack data: %d",  __LINE__, ret);
		return -2;
	}
	/* TODO CHECK */
	FREE(dfr); dfr = NULL;


	/* node info */
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdApplicationControllerUpdate);
	if (ret < APIERR_NONE) {
		log_debug("[%d] : can't wait node info: %d",  __LINE__, ret);
		return -3;
	}

	if (ni != NULL) {
		ni->bStatus = dfr->payload[0];
		ni->bNodeID = dfr->payload[1];
		ni->len = dfr->payload[2];
		if (ni->len > 0) {
			ni->basic = dfr->payload[3];
			ni->generic = dfr->payload[4];
			ni->specific = dfr->payload[5];
			memcpy(ni->commandclasses, dfr->payload+6, ni->len - 3);
		}
	}
	FREE(dfr); dfr = NULL;

	return 0;
}

static int zwave_api_ZWaveAddNodeToNetwork_cancle(unsigned char funcID) {
	//file_event_unreg(&zwave_fet, frame_getfd(), zwave_in, NULL, NULL);
	//frame_reset();
	//file_event_reg(&zwave_fet, frame_getfd(), zwave_in, NULL, NULL);

	int ret = 0;
	stDataFrame_t *dfs = NULL;
	stAddNodeToNetworkIn_t antni = {0x05, 0x00, 0x00, 0x00};
	dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni, sizeof(antni)-2);

	ret = zwave_frame_send_with_ack(dfs, 4000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	funcID++;
	if (funcID == 0) {
		funcID++;
	}
	stAddNodeToNetworkIn_t antni_ = {0x05, funcID, 0x00, 0x00};
	dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni_, sizeof(antni_)-2);
	ret = zwave_frame_send_with_ack(dfs, 4000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -2;
	}

	stAddNodeToNetworkIn_t antni__ = {0x05, 0x00, 0x00, 0x00};
	dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni__, sizeof(antni__)-2);
	ret = zwave_frame_send_with_ack(dfs, 4000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -3;
	}

	return 0;
}


int zwave_api_ZWaveAddNodeToNetwork(stAddNodeToNetwork_t *antn) {
	unsigned char funcID = geneFuncID();
	stAddNodeToNetworkIn_t antni = {0x81, funcID, 0x00, 0x00};
	stDataFrame_t *dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni, sizeof(antni)-2);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}
	
	/* ctr */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveAddNodeToNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response ctr data: %d",  __LINE__, ret);
		return -2;
	}
	/* TODO CHECK */
	FREE(dfr); dfr = NULL;
	log_info("[%d] ctr", __LINE__);


	/*newdev added */
	ret = zwave_frame_wait_frame(&dfr, 10000, CmdZWaveAddNodeToNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response newdev added data: %d",  __LINE__, ret);
		if (ret == APIERR_FRAME_RECV_TOU) {
			zwave_api_ZWaveAddNodeToNetwork_cancle(funcID);
		}
		return -3;
	}
	/* TODO CHECK */
	FREE(dfr); dfr = NULL;
	log_info("[%d] newadded", __LINE__);


	/*added node*/
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveAddNodeToNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response added node data: %d",  __LINE__, ret);
		return -4;
	}
	/* TODO CHECK */
	if (antn != NULL) {
		antn->funcID		= dfr->payload[0];
		antn->bStatus	= dfr->payload[1];
		antn->bSource	= dfr->payload[2];
		antn->len			= dfr->payload[3];
		antn->basic		= dfr->payload[4];
		antn->generic	= dfr->payload[5];
		antn->specific	= dfr->payload[6];
		if (antn->len - 3 > 0) {
			memcpy(antn->commandclasses, &dfr->payload[7], antn->len - 3);
		}
	}
	FREE(dfr); dfr = NULL;
	log_info("[%d] added node", __LINE__);

	/* add comp */
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveAddNodeToNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response added comp data: %d",  __LINE__, ret);
		return -5;
	}
	/* TODO CHECK */
	FREE(dfr); dfr = NULL;
	log_info("[%d] added comp", __LINE__);

	return 0;
}

static int zwave_api_ZWaveRemoveNodeFromNetwork_cancle(unsigned funcID) {
	//file_event_unreg(&zwave_fet, frame_getfd(), zwave_in, NULL, NULL);
	//frame_reset();
	//file_event_reg(&zwave_fet, frame_getfd(), zwave_in, NULL, NULL);

	int ret = 0;
	stDataFrame_t *dfs = NULL;

	stRemoveNodeFromNetworkIn_t rnfn = {0x05, 0x00};
	dfs = frame_make(CmdZWaveRemoveNodeFromNetwork, (void *)&rnfn,  sizeof(rnfn));

	ret = zwave_frame_send_with_ack(dfs, 4000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	return 0;
}

static int zwave_api_ZWaveRemoveNodeFromNetwork_complete() {
	int ret = 0;
	stDataFrame_t *dfs = NULL;

	stRemoveNodeFromNetworkIn_t rnfn = {0x05, 0x00};
	dfs = frame_make(CmdZWaveRemoveNodeFromNetwork, (void *)&rnfn, 1);

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}
	
	return 0;
}


int zwave_api_ZWaveRemoveNodeFromNetwork() {
	unsigned char funcID = geneFuncID();
	stRemoveNodeFromNetworkIn_t rnfn = {0x01, funcID};
	stDataFrame_t *dfs = frame_make(CmdZWaveRemoveNodeFromNetwork, (void *)&rnfn, sizeof(rnfn));
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}
	
	/* remove response 0*/
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveRemoveNodeFromNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response 0 data: %d",  __LINE__, ret);
		return -2;
	}
	/* TODO CHECK*/
	FREE(dfr); dfr = NULL;
	log_info("[%d] response 0", __LINE__);


	/*remove response 1 */
	ret = zwave_frame_wait_frame(&dfr, 10000, CmdZWaveRemoveNodeFromNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait response 1 data: %d",  __LINE__, ret);
		if (ret == APIERR_FRAME_RECV_TOU) {
			zwave_api_ZWaveRemoveNodeFromNetwork_cancle(funcID);
		}
		return -3;
	}
	/* TODO CHECK*/
	FREE(dfr); dfr = NULL;
	log_info("[%d] response 1", __LINE__);

	

	/*remove s1 */
	ret = zwave_frame_wait_frame(&dfr, 5000, CmdZWaveRemoveNodeFromNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait remove s1 data: %d",  __LINE__, ret);
		return -4;
	}
	/* TODO CHECK*/
	FREE(dfr); dfr = NULL;
	log_info("[%d] remove s1", __LINE__);

	

	/*remove s2*/
	ret = zwave_frame_wait_frame(&dfr, 5000, CmdZWaveRemoveNodeFromNetwork);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait remove s1 data: %d",  __LINE__, ret);
		return -5;
	}
	/* TODO CHECK*/
	FREE(dfr); dfr = NULL;
	log_info("[%d] remove s2", __LINE__);

	/* comp */
	zwave_api_ZWaveRemoveNodeFromNetwork_complete();
	return 0;
}


int zwave_api_ZWaveIsFailedNode(char id) {
	int ret = 0;
	stDataFrame_t *dfs = NULL;

	stIsFailedNodeIn_t ifni = {id};
	dfs = frame_make(CmdZWaveIsFailedNode, (void *)&ifni, 1);

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	/* ret */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveIsFailedNode);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait ret data: %d",  __LINE__, ret);
		return -2;
	}

	return !!dfr->payload[0];
}

int zwave_api_ZWaveRemoveFailedNodeId(char id) {
	int ret = 0;
	stDataFrame_t *dfs = NULL;
	char funcID = geneFuncID();

	stRemoveFailedNodeIn_t rfni = {id, funcID};
	dfs = frame_make(CmdZWaveRemoveFailedNodeId, (void *)&rfni, 2);

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}

	/* cmd ack */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveRemoveFailedNodeId);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait cmd ack: %d",  __LINE__, ret);
		return -2;
	}
	if (dfr->payload[0] != 0x00) {
		log_debug("[%d] : cmd ack failed %02x",  __LINE__, dfr->payload[0]&0xff);
		return -3;
	}

	/* remove result */
	dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveRemoveFailedNodeId);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait cmd result: %d",  __LINE__, ret);
		return -4;
	}

	return 0;
}


int zwave_api_ZWaveSendData(void *data, int len) {
	stDataFrame_t *dfs = frame_make(CmdZWaveSendData, data, len);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs); dfs = NULL;
	if (ret != APIERR_NONE) {
		log_debug("[%d] : may be no ack : %d",  __LINE__, ret);
		return -1;
	}
	
	/* ack */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveSendData);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait ack data: %d",  __LINE__, ret);
		return -2;
	}
	/* TODO CHECK*/
	FREE(dfr); dfr = NULL;
	log_info("[%d] ack", __LINE__);

	/* tx status */
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdZWaveSendData);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait tx status: %d",  __LINE__, ret);
		return -3;
	}
	/* TODO CHECK*/
	FREE(dfr); dfr = NULL;
	log_info("[%d] tx status", __LINE__);

	return 0;
}


int zwave_api_util_cc(int id, char ep, char class, int command, char *inparam, int inlen, int wait, 
											 char  *outparam, int *outlen) {
	stSendDataIn_t sdi;
	int size = 0;

	sdi.nodeID = id&0xff;
	if (ep == 0) {
		sdi.pData_data[0] = class&0xff;	
		sdi.pData_data[1] = command&0xff;
		memcpy(sdi.pData_data + 2, inparam, inlen);
	} else {
		sdi.pData_data[0] = 0x60;
		sdi.pData_data[1] = 0x0D;
		sdi.pData_data[2] = 0;
		sdi.pData_data[3] = ep;
		sdi.pData_data[4] = class&0xff;
		sdi.pData_data[5] = command&0xff;
		memcpy(sdi.pData_data + 6, inparam, inlen);
		inlen += 4;
	}
	sdi.pData_len = (inlen+2) & 0xff;

	sdi.txOptions = 0x25;
	sdi.funcID = geneFuncID();
	sdi.pData_data[inlen+2] = sdi.txOptions;
	sdi.pData_data[inlen+3] = sdi.funcID;
	size = 6 + inlen;
	
	int ret = zwave_api_ZWaveSendData((void *)&sdi, size);
	if (ret != 0) {
		log_err("[%d] zwave send data error: %d", __LINE__, ret);
		return -1;
	}

	if (wait == 0) {
		return 0;
	}
	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000, CmdApplicationCommandHandler);
	if (ret != APIERR_NONE) {
		log_debug("[%d] : can't wait extra data: %d",  __LINE__, ret);
		return -2;
	}

	/* TODO CHECK*/
	*outlen = dfr->size;
	memcpy(outparam, dfr->payload, dfr->size);

	FREE(dfr); dfr = NULL;
	return 0;
}

int zwave_api_util_wait_frame(stDataFrame_t **dfr, int timeout, char expect) {
	return zwave_frame_wait_frame(dfr, 1000, expect);
}
