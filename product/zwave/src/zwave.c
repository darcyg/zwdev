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
static stInventory_t inventory;

static int zwave_frame_send_with_ack(stDataFrame_t *dfs, int timeout) {
	int ret = 0;
	if (frame_send(dfs, timeout) != 0) {
		return -1;
	}

	stDataFrame_t *dfr = NULL;
re_1:
	ret = frame_recv(&dfr, 1000);
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
int zwave_ZWaveGetNodeProtoInfo() {
	stDataFrame_t *dfs = frame_make(CmdSerialApiGetCapabilities, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 80);
	FREE(dfs);
	if (ret < 0) {
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 80);
	if (ret < 0) {
		return -2;
	}

	if (dfr->error != FE_NONE) {
		FREE(dfr);
		return -3;
	}

	/*
	stInventory_t *inv = zwave_get_inventory();
	stNodeProtoInfo_t npi;
	npi.Capability = dfr->payload[0];
	npi.Security = dfr->payload[1];
	npi.Basic = dfr->payload[2];
	npi.Generic = dfr->payload[3];
	npi.Specific = dfr->payload[4];
	*/

	return 0;
}
int zwave_ZWaveSendData() {
	stDataFrame_t *dfs = frame_make(CmdSerialApiGetCapabilities, NULL, 0);
	int ret = 0;

	ret = zwave_frame_send_with_ack(dfs, 80);
	FREE(dfs);
	if (ret < 0) {
		return -1;
	}

	
	stDataFrame_t *dfr = NULL;
	ret = zwave_frame_wait_frame(&dfr, 80);
	if (ret < 0) {
		return -2;
	}
	
	if (dfr->error != FE_NONE) {
		FREE(dfr);
		return -3;
	}
	FREE(dfr); dfr = NULL;


	ret = zwave_frame_wait_frame(&dfr, 80);
	if (ret < 0) {
		return -4;
	}
	
	if (dfr->error != FE_NONE) {
		FREE(dfr);
		return -5;
	}
	FREE(dfr); dfr = NULL;

	return 0;
}


int zwave_class_command(int id, int class, int command, json_t *param) {
	return 0;
}
int zwave_class_version_get(int id, int class) {
	return 0;
}

int zwave_init(void *_th, void *_fet) {
	frame_init("/dev/ttyACM0", 115200);

	zwave_ZWaveGetVersion();
	zwave_SerialApiGetInitData();
	zwave_SerialApiGetCapabilities();
	zwave_MemoryGetId();
	zwave_ZWaveGetSucNodeId();
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


