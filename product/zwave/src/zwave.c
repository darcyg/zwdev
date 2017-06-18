#include <stdio.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "jansson.h"
#include "json_parser.h"

#include "frame.h"

#include "zwave.h"
#include "flash.h"
#include "memory.h"
#include "hex.h"

static stInventory_t inventory;
int zwave_test();

static int zwave_frame_send_with_ack(stDataFrame_t *dfs, int timeout) {
	int ret = 0;
	if (frame_send(dfs, timeout) != 0) {
		return -1;
	}

	stDataFrame_t *dfr = NULL;
re_1:
	ret = frame_recv(&dfr, timeout);
	if (ret == FR_ACK) {
		log_debug("ACK");
		ret = 0;
	} else if (ret == FR_OK) { /* normal frame , async */
		/* queue the frame */
		if (dfr != NULL) {
			log_debug_hex("async data:", dfr->payload, dfr->size);
				
			FREE(dfr);
			dfr = NULL;
		}
		goto re_1;
	} else if (ret == FR_NAK) {
		ret = -2;
	} else if (ret == FR_CAN) {
		ret = -3;
	} else if (ret == FR_TIMEOUT) {
		ret = -4;
	}

	return ret;
}

static int zwave_frame_wait_frame(stDataFrame_t**dfr, int timeout) {
	int ret = 0;
	ret = frame_recv(dfr, timeout);
	if (ret == FR_ACK) {
		ret = -1;
	} else if (ret == FR_OK) { /* except frame */
		log_debug_hex("recv frame:", (*dfr)->payload, (*dfr)->size);
		ret = 0;
	} else if (ret == FR_NAK) {
		ret = -2;
	} else if (ret == FR_CAN) {
		ret = -3;
	} else if (ret == FR_TIMEOUT) {
		ret = -4;
	}

	return ret;
}

int zwave_ZWaveGetVersion() {
	stDataFrame_t *dfs = frame_make(CmdZWaveGetVersion, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 80);
	FREE(dfs);
	if (ret < 0) {
		log_debug("zwave send, no ack! : %d", ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_debug("zwave get version can't wait version data! : %d", ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_debug("zwave get version wait error frame!");
		FREE(dfr);
		return -3;
	}
	
	if (dfr->cmd != CmdZWaveGetVersion) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}
	
	
	stInventory_t *inv = zwave_get_inventory();
	stVersion_t *ver = &inv->ver;
	strcpy(ver->ver, dfr->payload);
	ver->type = dfr->payload[strlen(ver->ver) + 1];

	FREE(dfr)	;
	dfr = NULL;

	log_debug("ver:%s, type:%02x", ver->ver, ver->type);	

	return 0;
}
int zwave_SerialApiGetInitData() {
	stDataFrame_t *dfs = frame_make(CmdSerialApiGetInitData, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs);
	if (ret < 0) {
		log_debug("zwave send, no ack! : %d", ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_debug("zwave get initdata can't wait init data! : %d", ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_debug("zwave get version wait error frame!");
		FREE(dfr);
		dfr = NULL;
		return -3;
	}

	if (dfr->cmd != CmdSerialApiGetInitData) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}
	
	
	stInventory_t *inv = zwave_get_inventory();
	stInitData_t *id= &inv->initdata;

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

	FREE(dfr)	;
	dfr = NULL;


	int i = 0;
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		if (id == 1) {
			continue;
		}

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			memory_del_dev(id);
			flash_remove_dev(id);
			continue;
		} else {
			json_t * jdev = flash_load_dev(id);
			if (jdev == NULL) {
				jdev = json_object();
				json_object_set_new(jdev, "id", json_integer(id));
				memory_set_dev(id, jdev);
				flash_save_dev(id, jdev);
			} else {
				memory_del_dev(id);
				memory_set_dev(id, jdev);
			}
		}
	}
	
	return 0;
}
int zwave_SerialApiGetCapabilities() {
	stDataFrame_t *dfs = frame_make(CmdSerialApiGetCapabilities, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs);
	if (ret < 0) {
		log_debug("zwave send, no ack! : %d", ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 80);
	if (ret < 0) {
		log_debug("serial api get cap can't wait cap data! : %d", ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_debug("serial api get cap wait error frame!");
		FREE(dfr);
		dfr = NULL;
		return -3;
	}

	if (dfr->cmd != CmdSerialApiGetCapabilities) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}
	
	
	stInventory_t *inv = zwave_get_inventory();
	stCapabilities_t *capa= &inv->caps;

	capa->AppVersion = dfr->payload[0];
	capa->AppRevisioin = dfr->payload[1];
	capa->ManufacturerId = dfr->payload[2]*256 + dfr->payload[3];
	capa->ManufactureProductType = dfr->payload[4]*256 + dfr->payload[5];
	capa->ManufactureProductId = dfr->payload[6]*256 + dfr->payload[7];
	memcpy(capa->SupportedFuncIds_map, &dfr->payload[8], sizeof(capa->SupportedFuncIds_map));

	log_debug("AppVersion:%02x, AppReVersion:%02x, ManufacturerId:%04x, ManufacturerProductType:%04x, ManufacturerProductId:%04x",
						capa->AppVersion, capa->AppRevisioin, capa->ManufacturerId, capa->ManufactureProductType, capa->ManufactureProductId);

	log_debug_hex("supportted funcs:", capa->SupportedFuncIds_map, sizeof(capa->SupportedFuncIds_map));

	FREE(dfr);
	dfr = NULL;

	return 0;
}
int zwave_MemoryGetId() {
	stDataFrame_t *dfs = frame_make(CmdMemoryGetId, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 80);
	FREE(dfs);
	if (ret < 0) {
		log_debug("zwave send, no ack! : %d", ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 80);
	if (ret < 0) {
		log_debug("memory get id can't wait memory id data! : %d", ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_debug("memory get id wait error frame!");
		FREE(dfr);
		dfr = NULL;
		return -3;
	}

	if (dfr->cmd != CmdMemoryGetId) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}
	

	stInventory_t *inv = zwave_get_inventory();
	stId_t *i= &inv->id;

	i->HomeID = *(int*)&dfr->payload[0];
	i->NodeID = dfr->payload[4];

	log_debug("HomeId:%08x, NodeId:%02x", i->HomeID, i->NodeID);

	FREE(dfr);
	dfr = NULL;
	return 0;
}
int zwave_ZWaveGetSucNodeId() {

	stDataFrame_t *dfs = frame_make(CmdZWaveGetSucNodeId, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 80);
	FREE(dfs);
	if (ret < 0) {
		log_debug("zwave send, no ack! : %d", ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 80);
	if (ret < 0) {
		log_debug("get suc node id can't wait id data! : %d", ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_debug("get suc node id wait error frame!");
		FREE(dfr);
		dfr = NULL;
		return -3;
	}

	if (dfr->cmd != CmdZWaveGetSucNodeId) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}
	

	stInventory_t *inv = zwave_get_inventory();
	stSucNodeId_t *sni = &inv->sucid;

	sni->SUCNodeID = dfr->payload[0];

	log_debug("suc node id:%02x", sni->SUCNodeID);

	FREE(dfr);
	dfr = NULL;
	
	return 0;
}

int zwave_CancleZWaveAddNodeToNetwork() {
	log_info("[%d]", __LINE__);

	frame_reset();

	int ret = 0;
	stDataFrame_t *dfs = NULL;
	do {
		stAddNodeToNetworkIn_t antni = {0x05, 0x00, 0x00, 0x00};
		//stDataFrame_t *dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni, sizeof(antni)-2);
		dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni, sizeof(antni)-2);

		ret = zwave_frame_send_with_ack(dfs, 4000);
		FREE(dfs);
		if (ret < 0) {
			log_err("[%d] no ack : %d", __LINE__, ret);
			//return -1;
		}
	} while (ret != 0);

	
	stAddNodeToNetworkIn_t antni_ = {0x05, 0x05, 0x00, 0x00};
	dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni_, sizeof(antni_)-2);
	ret = zwave_frame_send_with_ack(dfs, 4000);
	FREE(dfs);
	if (ret < 0) {
		log_err("[%d] no ack : %d", __LINE__, ret);
		return -2;
	}


	stAddNodeToNetworkIn_t antni__ = {0x05, 0x00, 0x00, 0x00};
	dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni__, sizeof(antni__)-2);
	ret = zwave_frame_send_with_ack(dfs, 4000);
	FREE(dfs);
	if (ret < 0) {
		log_err("[%d] no ack : %d", __LINE__, ret);
		return -3;
	}


	return 0;
}
int zwave_ZWaveAddNodeToNetwork() {
	log_info("[%d]", __LINE__);

	stAddNodeToNetworkIn_t antni = {0x81, 0x04, 0x00, 0x00};
	stDataFrame_t *dfs = frame_make(CmdZWaveAddNodeToNetwork, (void *)&antni, sizeof(antni)-2);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs);
	if (ret < 0) {
		log_err("[%d] no ack! : %d", __LINE__, ret);
		return -1;
	}

	
	/* ctr */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] not ctr frame: %d", __LINE__, ret);
		return -2;
	}
	
	if (dfr->error != FE_NONE) {
		log_err("[%d] error frame: %d", __LINE__, dfr->error);
		FREE(dfr);
		return -3;
	}
	FREE(dfr); dfr = NULL;
	log_info("[%d] ctr", __LINE__);


	/*newdev added */
	ret = zwave_frame_wait_frame(&dfr, 5000);
	if (ret < 0) {
		log_err("[%d] no newadded frame : %d", __LINE__, ret);
		if (ret == -4) {
			zwave_CancleZWaveAddNodeToNetwork();
		}
		return -2;
	}
	
	if (dfr->error != FE_NONE) {
		log_err("[%d] error frame : %d", __LINE__, dfr->error);
		FREE(dfr);
		return -3;
	}
	FREE(dfr); dfr = NULL;
	log_info("[%d] newadded", __LINE__);

	

	/*added node*/
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] no addeed node: %d", __LINE__, ret);
		return -2;
	}
	
	if (dfr->error != FE_NONE) {
		log_err("[%d] error frame: %d", __LINE__, dfr->error);
		FREE(dfr);
		return -3;
	}
	log_info("[%d] added node", __LINE__);

	stAddNodeToNetwork_t antn;

	antn.funcID		= dfr->payload[0];
	antn.bStatus	= dfr->payload[1];
	antn.bSource	= dfr->payload[2];
	antn.len			= dfr->payload[3];
	antn.basic		= dfr->payload[4];
	antn.generic	= dfr->payload[5];
	antn.specific	= dfr->payload[6];
	if (antn.len - 3 > 0) {
		memcpy(antn.commandclasses, &dfr->payload[7], antn.len - 3);
	}

	//stInventory_t *inv = zwave_get_inventory();
	int id = antn.bSource&0xff;
	json_t *jdev = memory_get_dev(id);
	if (jdev != NULL) {
		memory_del_dev(id);
		flash_remove_dev(id);
	}
	jdev = json_object();

	char buf[32];
	sprintf(buf, "%02x", id&0xff);
	json_object_set_new(jdev, "id", json_string(buf));
	sprintf(buf, "%02x", antn.basic&0xff); json_object_set_new(jdev, "basic", json_string(buf));
	sprintf(buf, "%02x", antn.generic&0xff); json_object_set_new(jdev, "generic", json_string(buf));
	sprintf(buf, "%02x", antn.specific&0xff); json_object_set_new(jdev, "specific", json_string(buf));

	json_t *jclasses = json_object();
	int i = 0;
	for (i = 0; i < antn.len - 3; i++) {
		sprintf(buf, "%02x", antn.commandclasses[i]&0xff);
		json_object_set_new(jclasses, buf, json_object());
	}
	json_object_set_new(jdev,"classes", jclasses);

	memory_del_dev(id);
	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);

	FREE(dfr); dfr = NULL;

	/* add comp */
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] no add comp : %d", __LINE__, ret);
		return -2;
	}
	
	if (dfr->error != FE_NONE) {
		log_err("[%d] error frame : %d", __LINE__, dfr->error);
		FREE(dfr);
		return -3;
	}
	FREE(dfr); dfr = NULL;
	log_info("[%d] add comp", __LINE__);

	stNodeProtoInfo_t npi;
	if (zwave_ZWaveGetNodeProtoInfo(id&0xff,  &npi) != 0) {
		log_err("[%d] get node proto info failed : %d", __LINE__, ret);
		return -4;
	}

	jdev = memory_get_dev(id);
	sprintf(buf, "%02x", npi.Capability&0xff);
	json_object_set_new(jdev, "capability", json_string(buf));
	sprintf(buf, "%02x", npi.Security&0xff);
	json_object_set_new(jdev, "security", json_string(buf));
	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);
	

	for (i = 0; i < antn.len - 3; i++) {
		char class = antn.commandclasses[i]&0xff;
		zwave_class_init(id&0xff, class);
	}

	flash_save_dev(id, jdev);

	return 0;
}

int zwave_ZWaveGetNodeProtoInfo(char nodeid, stNodeProtoInfo_t *npi) {
	log_err("[%d]", __LINE__);

	stNodeProtoInfoIn_t npii = {nodeid};
	stDataFrame_t *dfs = frame_make(CmdZWaveGetNodeProtoInfo, (void*)&npii, sizeof(npii));
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs);
	if (ret < 0) {
		log_err("[%d] no ack : %d", __LINE__, ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] no node proto info : %d", __LINE__, ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_err("[%d] frame error : %d", __LINE__, ret);
		FREE(dfr);
		return -3;
	}

	if (dfr->cmd != CmdZWaveGetNodeProtoInfo) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}

	npi->Capability = dfr->payload[0];
	npi->Security = dfr->payload[1];
	npi->Basic = dfr->payload[2];
	npi->Generic = dfr->payload[3];
	npi->Specific = dfr->payload[4];

	return 0;
}

int zwave_ZWaveRequestNodeInfo(int id) {
	log_info("[%d]", __LINE__);

	stNodeInfoIn_t npii = {id&0xff};
	stDataFrame_t *dfs = frame_make(CmdZWaveRequestNodeInfo, (void*)&npii, sizeof(npii));
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs);
	if (ret < 0) {
		log_err("[%d] no ack : %d", __LINE__, ret);
		return -1;
	}

	
	/* ack */
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] no node proto info ack: %d", __LINE__, ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_err("[%d] frame error : %d", __LINE__, ret);
		FREE(dfr);
		return -3;
	}

	if (dfr->cmd != CmdZWaveRequestNodeInfo) {
		log_debug_hex("async data:", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}
	FREE(dfr); dfr = NULL;

	/* node info */
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] no node proto info : %d", __LINE__, ret);
		return -2;
	}

	if (dfr->error != FE_NONE) {
		log_err("[%d] frame error : %d", __LINE__, ret);
		FREE(dfr);
		return -3;
	}

	if (dfr->cmd != CmdApplicationControllerUpdate) {
		log_debug("[%d] async :cmd(%02X),wait(%02x)", __LINE__, dfr->cmd, CmdZWaveRequestNodeInfo);
		log_debug_hex("async data:%02x", dfr->payload, dfr->size);
		FREE(dfr);
		dfr = NULL;
		return -4;
	}


	stNodeInfo_t ni;
	ni.bStatus = dfr->payload[0];
	ni.bNodeID = dfr->payload[1];
	ni.len = dfr->payload[2];
	if (ni.len > 0) {
		ni.basic = dfr->payload[3];
		ni.generic = dfr->payload[4];
		ni.specific = dfr->payload[5];
		memcpy(ni.commandclasses, dfr->payload+6, ni.len - 3);
	}

	//stInventory_t *inv = zwave_get_inventory();
	//int id = ni.bNodeID&0xff;
	json_t *jdev = memory_get_dev(id);
	if (jdev != NULL) {
		memory_del_dev(id);
		flash_remove_dev(id);
	}
	jdev = json_object();

	char buf[32];
	sprintf(buf, "%02x", id&0xff);
	json_object_set_new(jdev, "id", json_string(buf));
	sprintf(buf, "%02x", ni.basic&0xff); json_object_set_new(jdev, "basic", json_string(buf));
	sprintf(buf, "%02x", ni.generic&0xff); json_object_set_new(jdev, "generic", json_string(buf));
	sprintf(buf, "%02x", ni.specific&0xff); json_object_set_new(jdev, "specific", json_string(buf));

	json_t *jclasses = json_object();
	int i = 0;
	for (i = 0; i < ni.len - 3; i++) {
		sprintf(buf, "%02x", ni.commandclasses[i]&0xff);
		json_object_set_new(jclasses, buf, json_object());
	}
	json_object_set_new(jdev,"classes", jclasses);

	memory_del_dev(id);
	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);

	FREE(dfr); dfr = NULL;

	for (i = 0; i < ni.len - 3; i++) {
		char class = ni.commandclasses[i]&0xff;
		zwave_class_init(id&0xff, class);
	}
	flash_save_dev(id, jdev);

	return 0;
}

int zwave_ZWaveSendData(void *data, int len) {
	log_info("[%d]", __LINE__);
	stDataFrame_t *dfs = frame_make(CmdZWaveSendData, data, len);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 1000);
	FREE(dfs);
	if (ret < 0) {
		log_err("[%d] no ack : %d", __LINE__, ret);
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] can't wait RetVal:%d", __LINE__, ret);
		return -2;
	}
	
	if (dfr->error != FE_NONE) {
		log_err("[%d] error frame: %d", __LINE__, dfr->error);
		FREE(dfr);
		return -3;
	}

	if (dfr->cmd != CmdZWaveSendData) {
		log_err("[%d] async data: %d", __LINE__, dfr->cmd&0xff);
		return -4;
	}


	FREE(dfr); dfr = NULL;


	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] can't wait tx status: %d", __LINE__, ret);
		return -4;
	}
	
	if (dfr->error != FE_NONE) {
		log_err("[%d] error frame: %d", __LINE__, dfr->error);
		FREE(dfr);
		return -5;
	}
	if (dfr->cmd != CmdZWaveSendData) {
		log_err("[%d] async data: %d", __LINE__, dfr->cmd&0xff);
		return -4;
	}


	FREE(dfr); dfr = NULL;

	return 0;
}


int zwave_class_command(int id, char class, int command, char *inparam, int inlen, int wait,  char  *outparam, int *outlen) {
	log_info("[%d]", __LINE__);
	static char funcID = 0x01;
	stSendDataIn_t sdi;
	int size = 0;

	sdi.nodeID = id&0xff;
	sdi.pData_len = (inlen+2) & 0xff;
	sdi.pData_data[0] = class&0xff;	
	sdi.pData_data[1] = command&0xff;
	memcpy(sdi.pData_data + 2, inparam, inlen);

	sdi.txOptions = 0x25;
	sdi.funcID = funcID++;
	if (funcID == 0) {
		funcID = 1;
	}
	sdi.pData_data[inlen+2] = sdi.txOptions;
	sdi.pData_data[inlen+3] = sdi.funcID;
	size = 6 + inlen;
	
	int ret = zwave_ZWaveSendData((void *)&sdi, size);
	if (ret != 0) {
		log_err("[%d] zwave send data error: %d", __LINE__, ret);
		return -1;
	}

	if (wait == 0) {
		return 0;
	}
	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 1000);
	if (ret < 0) {
		log_err("[%d] zwave get wait no data: %d", __LINE__, ret);
		return -2;
	}

	if (dfr->cmd != CmdApplicationCommandHandler) {
		log_err("[%d] async data : %d", __LINE__, dfr->cmd & 0xff);
		return -3;
	}

	*outlen = dfr->size;
	memcpy(outparam, dfr->payload, dfr->size);

	FREE(dfr); dfr = NULL;
	
	return 0;
}

int zwave_class_version_get(int id, char class) {
	log_info("[%d] dev(%02x), class(%02x)", __LINE__, id&0xff, class&0xff);
	
	char outparam[128];
	char inparam[1] = {class&0xff};
	int outlen;
	int ret = zwave_class_command(id, 0x86, 0x13, inparam, sizeof(inparam), 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -1;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | command | version */
	/* 86        14       class     version */
	if ((outparam[5]&0xff) != (class&0xff)) {
		log_err("[%d] not corrent class to get: want(%02x), get(%02x)", __LINE__, class&0xff, outparam[5]&0xff);
		return -2;
	}

	int version = outparam[6] & 0xff;

	return version;
}

int zwave_init(void *_th, void *_fet) {
	frame_init("/dev/ttyACM0", 115200);

	zwave_ZWaveGetVersion();
	zwave_SerialApiGetInitData();
	zwave_SerialApiGetCapabilities();
	zwave_MemoryGetId();
	zwave_ZWaveGetSucNodeId();

	//zwave_ZWaveAddNodeToNetwork();

	zwave_test();
	return 0;
}
int zwave_include() {
	return 0;
}
int zwave_exclude(int id) {
	return 0;
}
int zwave_set(int id, int class, int command, json_t *param) {
	return 0;
}


stInventory_t *zwave_get_inventory() {
	return &inventory;
}

//=================================================================================
typedef int (*CLASS_INIT)(int id, char class, int version);
typedef struct stClassCommandFuncs {
	char							class;
	CLASS_INIT				init;
}stClassCommandFuncs_t;

int powerlevel_init(int id, char class, int version);
int switch_binary_init(int id, char class, int version);
int zwaveplus_info_init(int id, char class, int version);
int association_init(int id, char class, int version);
int association_grp_info_init(int id, char class, int version);
int version_init(int id, char class, int version);
int manufacturer_specific_init(int id, char class, int version);
int device_reset_locally_init(int id, char class, int version);

static stClassCommandFuncs_t _zwave_class_init_funcs[] = {
	[0x73] = {0x73, powerlevel_init},
	[0x25] = {0x25, switch_binary_init},
	[0x5e] = {0x5e, zwaveplus_info_init},
	[0x85] = {0x85, association_init},
	[0x59] = {0x59, association_grp_info_init},
	[0x86] = {0x86, version_init},
	[0x72] = {0x72, manufacturer_specific_init},
	[0x5a] = {0x5a, device_reset_locally_init},
};
int _zwave_class_init(int id, char class, int version);
int zwave_class_init(int id, char class) {
	log_info("[%d] : dev(%02x), class(%02x)", __LINE__, id&0xff, class);
	int ret = zwave_class_version_get(id, class);
	if (ret < 0) {
		log_err("[%d] get version failed: %d", __LINE__, ret);
		return -1;
	}
	log_info("[%d] class:%02x, version:%d", __LINE__, class&0xff, ret);
	int version = ret;

	json_t *jdev = memory_get_dev(id);
	json_t *jclasses = json_object_get(jdev, "classes");
	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);
	{
		json_object_set_new(jclass, "version", json_integer(version));
	}
	
	_zwave_class_init_funcs[class&0xff].init(id, class, version);
	return 0;
}

int powerlevel_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);

	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | power level*/
	/* 73        03        level */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}
	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	return 0;
}
int switch_binary_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);

	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 25        03        value  */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}
	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	return 0;
}
int zwaveplus_info_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x01;
	int ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 5e        02      value(3 v1) or value(5 v2) */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	return 0;
}
int association_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;

	char command = 0x05;
	int ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 85        06      supportted groups(1bype v1/v2) */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	char ars = outparam[5];
	int i = 0;
	for (i = 0; i < 8; i++) {
		if ((ars & ( 1 << i)) == 0) {
			continue;
		}
		char gid = i+1;
		char command = 0x01;
		char buf[2] = {gid, 0x01};
		int ret = zwave_class_command(id, class, command, buf, 2, 0, outparam, &outlen);
		if (ret < 0) {
			log_err("[%d] association set failed:%d", __LINE__, ret);
		}

		command = 0x02;
		
		ret = zwave_class_command(id, class, command, &gid, 1, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 85        03      gid, maxgrp, reports to flow, node id,*/

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	}

	if (version == 2) {
		command = 0x0b;
		ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 85        0c      supportted groups(1bype v1/v2) */

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));

		ars = outparam[5];
		int i = 0;
		for (i = 0; i < 8; i++) {
			if ((ars & ( 1 << i)) == 0) {
				continue;
			}
			char gid = i+1;
			char command = 0x01;
			char buf[2] = {gid, 0x01};
			int ret = zwave_class_command(id, class, command, buf, 2, 0, outparam, &outlen);
			if (ret < 0) {
				log_err("[%d] association set failed:%d", __LINE__, ret);
			}

			command = 0x02;

			ret = zwave_class_command(id, class, command, &gid, 1, 1, outparam, &outlen);

			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			/* rxStatus | sourceNode | len */
			/* class | command | value */
			/* 85        03      gid, maxgrp, reports to flow, node id,*/

			json_t *jdev = memory_get_dev(id);

			json_t *jclasses = json_object_get(jdev, "classes");

			sprintf(sclass, "%02x", class&0xff);
			json_t *jclass = json_object_get(jclasses, sclass);

			sprintf(scommand, "%02x", outparam[4]&0xff);
			json_t *jcommand = json_object_get(jclass, scommand);
			if (jcommand != NULL) {
				json_object_del(jclass, scommand);
			}

			char svalue[(outparam[2] - 2) * 3+1];
			hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
			json_object_set_new(jclass, scommand, json_string(svalue));
		}
	}
	
	return 0;
}
int association_grp_info_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	return 0;
}
int version_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);

	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x11;
	int ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 86        12      5(v1) |  7(v2）+*/
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	return 0;
}
int manufacturer_specific_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x04;
	int ret = zwave_class_command(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 72        05      5(v1) |  7(v2）+*/
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	if (version == 2) {
		char outparam[128];
		int outlen;
		char command = 0x06;
		//char inparam[] = {0x01};
		char inparam[] = {0x02};
		int ret = zwave_class_command(id, class, command, inparam, 1, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 72       07      5(v1) |  7(v2）+*/

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	}
	
	return 0;
}

int device_reset_locally_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	return 0;
}

///////////////////////////////////////////////////////
int zwave_test() {
	stInventory_t *inv = zwave_get_inventory();

	int i = 0;
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		if (id == 1) {
			continue;
		}

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		} 

		json_t * jdev = memory_get_dev(id);
		char *sdev = json_dumps(jdev, 0);
		if (sdev != NULL) {
			log_info("[%d] dev:%02x: %s", __LINE__, id&0xff, sdev);
			FREE(sdev);
			sdev = NULL;
		}

		log_info("[%d] request info : %d", __LINE__, id);
		zwave_ZWaveRequestNodeInfo(id);
		log_info("[%d] request info : %d over", __LINE__, id);
	
	}
	return 0;
}

