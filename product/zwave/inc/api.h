#ifndef _API_H_
#define _API_H_

/* this file contains zwave static controller serial api interfaces */

typedef struct stVersion {
	char ver[32];
	char type;
}stVersion_t;

typedef struct stInitData {
	char ver;
	char capablities;
	char nodes_num;
	char nodes_slots[230];
	char chip_type;
	char chip_version;	
}stInitData_t;

typedef struct stNodeProtoInfoIn {
	char bNodeID;
}stNodeProtoInfoIn_t;
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
	char ManufacturerId;
	char ManufactureProductType;
	char ManufactureProductId;
	char SupportedFuncIds_num;
	char SupportedFuncIds_slots[255];
}stCapabilities_t;

typedef struct stControllerCapabilities {
	char RetVal;
}stControllerCapabilities_t;

typedef struct stId {
	int		HomeID;
	char	NodeID;
}stId_t;

typedef struct stSucNodeId {
	char SUCNodeID;
}stSucNodeId_t;

typedef struct stNodeInformation {
	char deviceOptionsMask;
	char generic;
	char specific;
	char nodeParm[16];
}stNodeInformation_t;

typedef struct stAddNodeToNetworkIn {
	char mode;
	char funcID;
}stAddNodeToNetworkIn_t;

typedef struct stAddNodeToNetwork {
	char funcID;
}stAddNodeToNetwork_t;

typedef struct stNodeInfoIn {
	char NodeID;	
}stNodeInfoIn_t;
typedef struct stNodeInfo {
	char retVal;
}stNodeInfo_t;

typedef struct stControllerUpdateIn {
	char funcID;
}stControlleUpdateIn_t;

typedef struct stRemoveNodeFromNetworkIn {
	char mode;
	char funcID;
}stRemoveNodeFromNetworkIn_t;
typedef struct stRemoveNodeFromNetwork {
	char funcID;
}stRemoveNodeFromNetwork_t;

typedef struct stSetSucNodeIdIn {
	char nodeID;
	char SUCState;
	char bTxOption;
	char capabilities;
	char funcID;
}stSetSucNodeIdIn_t;
typedef struct stSetSucNodeId {
	char RetVal;
}stSetSucNodeId_t;


typedef struct stSendDataIn {
	char nodeID;
	char pData_len;
	char pData_data[200];
	char txOptions;
	char funcID;
}stSendDataIn_t;
typedef struct stSendData {
	char RetVal;
}stSendData_t;
typedef struct stSendDataSts {
	char funcID;
	char txStatus;
}stSendDataSts_t;


typedef struct stIsFailedNodeIn {
	char funcID;
}stIsFailedNodeIn_t
typedef struct stIsFailedNode {
	char retVal;
}stIsFailedNode_t;

typedef struct stReplaceFailedNodeIn {
	char nodID;
	char funcID;
}stReplaceFailedNodeIn_t;
typedef struct stReplaceFailedNode {
	char retVal;
}stReplaceFailedNode_t;
typedef struct stReplaceFailedNodeSts {
	char funcID;
	char txStatus;
}stReplaceFailedNodeSts_t;


typedef emApi {
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

	CmdApplicationControllerUpdate = 0x49,

	CmdZWaveRemoveNodeFromNetwork = 0x4B,

	CmdZWaveRequestNodeInfo = 0x60,

	CmdZWaveSetSucNodeId = 0x54,

	CmdZWaveSendData = 0x13,

	CmdZWaveSendDataAbort = 0x16,

	CmdZWaveIsFailedNode = 0x62,

	CmdZWaveReplaceFailedNode = 0x63,
}emApi_t;

typedef struct stParam {
}stParam_t;


typedef void (*API_CALLBACK)(emApi_t api, stParam_t param);

int api_init(API_CALLBACK _acb);
int api_exec(emApi_t api, stParam_t param);
int api_free();










#endif
