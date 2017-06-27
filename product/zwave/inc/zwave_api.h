#ifndef __ZWAVE_API_H_
#define __ZWAVE_API_H_

#include "frame.h"

typedef struct stRemoveNodeFromNetworkIn {
	char mode;
	char funcID;
}stRemoveNodeFromNetworkIn_t;


typedef struct stNodeInfoIn {
	char NodeID;	
}stNodeInfoIn_t;


typedef struct stNodeProtoInfoIn {
	char bNodeID;
}stNodeProtoInfoIn_t;


typedef struct stInitData {
	char ver;
	char capabilities;
	char nodes_map_size;
	char nodes_map[255];
	char chip_type;
	char chip_version;	
}stInitData_t;


typedef struct stAddNodeToNetwork {
	char funcID;
	char bStatus;
	char bSource;
	char len;
	char basic;
	char generic;
	char specific;
	char commandclasses[32];
}stAddNodeToNetwork_t;



typedef struct stVersion {
	char ver[32];
	char type;
}stVersion_t;
typedef struct stNodeProtoInfo {
	char Capability;
	char Security;
	char Basic;
	char Generic;
	char Specific;
}stNodeProtoInfo_t;

typedef struct stCapabilities {
	char AppVersion;
	char AppRevisioin;
	short ManufacturerId;
	short ManufactureProductType;
	short ManufactureProductId;
	char SupportedFuncIds_map[32];
}stCapabilities_t;

typedef struct stId {
	int		HomeID;
	char	NodeID;
}stId_t;

typedef struct stSucNodeId {
	char SUCNodeID;
}stSucNodeId_t;

typedef struct stNodeInfo {
	char bStatus;
	char bNodeID;
	char len;
	char basic;
	char generic;
	char specific;
	char commandclasses[32];
}stNodeInfo_t;

typedef struct stSendDataIn {
	char nodeID;
	char pData_len;
	char pData_data[200];
	char txOptions;
	char funcID;
}stSendDataIn_t;

typedef struct stAddNodeToNetworkIn {
	char mode;
	char funcID;
	char yummy1;	
	char yummy2;
}stAddNodeToNetworkIn_t;



typedef struct stInventory {
	stInitData_t initdata;
	stVersion_t ver;
	stCapabilities_t caps;
	stId_t id;
	stNodeInfo_t ni;
	stSucNodeId_t sucid;
}stInventory_t;

typedef enum emApi {
	CmdZWaveGetVersion = 0x15,

	CmdSerialApiGetInitData = 0x02,

	CmdZWaveGetNodeProtoInfo = 0x41,

	CmdSerialApiGetCapabilities = 0x07,

	CmdZWaveGetControllerCapabilities = 0x05,

	CmdMemoryGetId = 0x20,

	CmdZWaveGetSucNodeId = 0x56,

	CmdSerialApiApplNodeInformation = 0x03,

	CmdZWaveAddNodeToNetwork = 0x4A,

	CmdZWaveRequestNodeInfo = 0x60,

	CmdApplicationControllerUpdate = 0x49,  //<---

	CmdZWaveRemoveNodeFromNetwork = 0x4B,


	CmdZWaveSetSucNodeId = 0x54,

	CmdZWaveSendData = 0x13,

	CmdZWaveSendDataAbort = 0x16,

	CmdZWaveIsFailedNode = 0x62,

	//CmdZWaveReplaceFailedNode = 0x63,

	CmdZWaveRemoveFailedNodeId = 0x61,

	CmdSerialApiSoftReset = 0x08,
	CmdZWaveGetProtocolVersion = 0x09,
	CmdSerialApiStarted = 0x0A,

	CmdZWaveRfPowerLevelGet = 0xBA,
	CmdZWaveGetNeighborCount = 0xBB,
	CmdZWaveAreNodesNeighbours = 0xBC,
	
	CmdZWaveTypeLibrary = 0xBD,
	CmdZWaveGetProtocolStatus = 0xBF,
	
	CmdIoPortStatus = 0xE5,
	CmdIoPort = 0xE6,

	CmdApplicationCommandHandler = 0x04,
}emApi_t;

typedef enum emApiErr {
	APIERR_NONE = 0,
	APIERR_FRAME_SEND_ERR = -1,
	APIERR_FRAME_RECV_NAK = -2,
	APIERR_FRAME_RECV_CAN = -3,
	APIERR_FRAME_RECV_TOU = -4,
	APIERR_FRAME_RECV_CHK = -5,
	APIERR_UNKNOWN = -9999,
}emApiErr_t;

int zwave_api_ZWaveGetVersion(stVersion_t *ver);
int zwave_api_SerialApiGetInitData(stInitData_t *id);
int zwave_api_SerialApiGetCapabilities(stCapabilities_t *capa);
int zwave_api_MemoryGetId(stId_t *i);
int zwave_api_ZWaveGetSucNodeId(stSucNodeId_t *sni);
int zwave_api_ZWaveGetNodeProtoInfo(char nodeid, stNodeProtoInfo_t *npi);
int zwave_api_ZWaveSendData(void *data, int len);
int zwave_api_ZWaveRequestNodeInfo(int id, stNodeInfo_t *ni);
int zwave_api_ZWaveAddNodeToNetwork(stAddNodeToNetwork_t *antn);
int zwave_api_ZWaveRemoveNodeFromNetwork();
int zwave_api_ZWaveSendData(void *data, int len);

int zwave_api_util_cc(int id, char class, int command, char *inparam, int inlen, int wait,  char  *outparam, int *outlen);
int zwave_api_util_wait_frame(stDataFrame_t **dfr, int timeout, char expect);

#endif