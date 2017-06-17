#ifndef __ZWAVE_H_
#define __ZWAVE_H_


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



int zwave_ZWaveGetVersion();
int zwave_SerialApiGetInitData();
int zwave_SerialApiGetCapabilities();
int zwave_MemoryGetId();
int zwave_ZWaveGetSucNodeId();
int zwave_ZWaveGetNodeProtoInfo(char nodeid, stNodeProtoInfo_t *npi);

int zwave_class_command(int id, char class, int command, char *inparam, int inlen, int wait,  char  *outparam, int *outlen);
int zwave_class_version_get(int id, char class);
int zwave_class_init(int id, char class);

int zwave_init(void *_th, void *_fet);
int zwave_include();
int zwave_exclude(int id);
int zwave_set(int id, int class, int command, json_t *param);


stInventory_t *zwave_get_inventory();


#endif
