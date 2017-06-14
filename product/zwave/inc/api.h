#ifndef _API_H_
#define _API_H_

/* this file contains zwave static controller serial api interfaces */
#pragma pack(1)
typedef struct stVersion {
	char ver[32];
	char type;
}stVersion_t;

typedef struct stInitData {
	char ver;
	char capabilities;
	char nodes_map_size;
	char nodes_map[255];
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
	short ManufacturerId;
	short ManufactureProductType;
	short ManufactureProductId;
	char SupportedFuncIds_map[32];
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

typedef struct stApplNodeInformationIn {
	char deviceOptionsMask;
	char generic;
	char specific;
	char nodeParm[16];
}stApplNodeInformationIn_t;



typedef struct stAddNodeToNetworkIn {
	char mode;
	char funcID;
	char yummy1;	
	char yummy2;
}stAddNodeToNetworkIn_t;

typedef struct stAddNodeToNetworkWait {
	char funcID;
	char dummy1;
	char dummy2;
	char dummy3;
}stAddNodeToNetworkWait_t;

typedef struct stAddNodeToNetworkBack {
	char funcID;
	char dummy1;
	char dummy2;
	char dummy3;
}stAddNodeToNetworkBack_t;

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

typedef struct stAddNodeToNetworkComp {
	char funcID;
	char dummy1;
	char dummy2;
	char dummy3;
}stAddNodeToNetworkComp_t;





typedef struct stNodeInfoIn {
	char NodeID;	
}stNodeInfoIn_t;
typedef struct stNodeInfoAck {
	char retVal;
}stNodeInfoAck_t;
typedef struct stNodeInfo {
	char bStatus;
	char bNodeID;
	char len;
	char basic;
	char generic;
	char specific;
	char commandclasses[32];
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
}stIsFailedNodeIn_t;
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

typedef struct stRemoveFailedNodeIdIn {
	char nodeID;
	char fundID;
}stRemoveFailedNodeIdIn_t;

typedef struct stGetNeighborCountIn {
	char nodeID;
}stGetNeighborCountIn_t;

typedef struct stAreNodesNeighborsIn {
	char nodeOne;
	char nodeTwo;
}stAreNodesNeighborsIn_t;
#pragma pack(4)





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
}emApi_t;

typedef union stParam {
	stVersion_t version;
	
	stInitData_t initData;

	stNodeProtoInfoIn_t nodeProtoInfoIn;
	stNodeProtoInfo_t nodeProtoInfo;

	stCapabilities_t capabilities;	

	stControllerCapabilities_t controllerCapabilities;

	stId_t id;

	stSucNodeId_t sucNodeId;

	stApplNodeInformationIn_t nodeInformation;
	
	stAddNodeToNetworkIn_t addNodeToNetworkIn;
	stAddNodeToNetwork_t addNodeToNetwork;

	stNodeInfoIn_t nodeInfoIn;
	stNodeInfoAck_t nodeInfoAck;
	stNodeInfo_t nodeInfo;

	stControlleUpdateIn_t controlleUpdateIn;

	stRemoveNodeFromNetworkIn_t removeNodeFromNetworkIn;
	stRemoveNodeFromNetwork_t removeNodeFromNetwork;

	stSetSucNodeIdIn_t setSucNodeIdIn;
	stSetSucNodeId_t setSucNodeId;

	stSendDataIn_t sendDataIn;
	stSendData_t sendData;
	stSendDataSts_t sendDataSts;	

	stIsFailedNodeIn_t isFailNodeIn;
	stIsFailedNode_t isFailNode;

	stReplaceFailedNodeIn_t replaceFailedNodeIn;
	stReplaceFailedNode_t replaceFailedNode;
	stReplaceFailedNodeSts_t replaceFailedNodeSts;
}stParam_t;

typedef struct stApiCall {
	emApi_t api;
	stParam_t param;
	int param_size;
} stApiCall_t;

typedef enum emApiError{
	AE_NONE = 0,
	AE_SEND_TIMEOUT = 1,
	AE_NAK = 2,
	AE_CAN = 3,
	AE_CHECKSUM = 4,
	AE_RECV_TIMEOUT = 5,
}emApiError_t;

typedef enum emApiState {
	AS_READY = 0,
	AS_END = 9999,
}emApiState_t;

typedef struct stApiState {
	void *parse;
	void *view;
	void *step;
}stApiState_t;


#define API_EXEC_TIMEOUT_MS (3000*1)
#define API_INCLUDE_TIMEOUT_MS (1000 * 5) 
#define API_EXCLUDE_TIMEOUT_MS (1000 * 5) 


typedef void (*API_CALL_CALLBACK)(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error);
typedef void (*API_RETURN_CALLBACK)(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error);


int api_init(void *_th, API_CALL_CALLBACK _accb, API_RETURN_CALLBACK _arcb);
int api_free();
int api_call(emApi_t api, stParam_t *param, int param_size);
int api_getfd();
int api_step();


/*                      api                           */
/*
int ZW_GetRoutingInfo(){return 0;}
int ZW_LockRoute(){return 0;}

int MemoryGetBuffer(){return 0;}
int MemoryGetByte(){return 0;}
int MemoryGetID();
int MemoryPutBuffer(){return 0;}
int MemoryPutByte(){return 0;}

int NVM_get_id(){return 0;}
int NVM_ext_read_long_buffer(){return 0;}
int NVM_ext_read_long_byte(){return 0;}
int NVM_ext_write_long_buffer(){return 0;}
int NVM_ext_write_long_byte(){return 0;}

int PWR_Clk_PD(){return 0;}
int PWR_Clk_PUp(){return 0;}
int PWR_Select_Clk(){return 0;}
int PWR_SetStopMode(){return 0;}

int ZW_StoreHomeID(){return 0;}
int ZW_StoreNodeInfo(){return 0;}
int ZW_AddNodeToNetwork();
int ZW_AES_ECB(){return 0;}
int ZW_AreNodesNeighbours();
int ZW_AssignReturnRoute(){return 0;}
int ZW_AssignSUCReturnRoute(){return 0;}
int ZW_ControllerChange(){return 0;}
int ZW_CreateNewPrimaryCtrl(){return 0;}
int ZW_DeleteReturnRoute(){return 0;}
int ZW_DeleteSUCReturnRoute(){return 0;}
int ZW_EnableSUC();
int ZW_ExploreRequestInclusion(){return 0;}
int ZW_GetControllerCapabilities();
int ZW_GetLastWorkingRoute();
int ZW_SetLastWorkingRoute();
int ZW_GetNeighborCount();
int ZW_GetNodeProtocolInfo();
int ZW_GetProtocolStatus();
int ZW_GetProtocolVersion();
int ZW_GetRandomWord();
int ZW_GetRoutingMAX(){return 0;}
int ZW_GetSUCNodeID();
int ZW_Version();
int ZW_GetVirtualNodes(){return 0;}
int ZW_isFailedNode();
int ZW_IsPrimaryCtrl();
int ZW_IsVirtualNode(){return 0;}
int ZW_IsWutKicked(){return 0;}
int ZW_NewController(){return 0;}
int ZW_RediscoveryNeeded(){return 0;}
int ZW_RemoveFailedNode(){return 0;}
int ZW_RemoveNodeFromNetwork();
int ZW_ReplaceFailedNode(){return 0;}
int ZW_ReplicationReceiveComplete(){return 0;}
int ZW_ReplicationSend(){return 0;}
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
int ZW_SendSlaveData(){return 0;}
int ZW_SendSUCID();
int ZW_SendTestFrame(){return 0;}
int ZW_SetDefault();
int ZW_SetExtIntLevel(){return 0;}
int ZW_SetLearnMode();
int ZW_SetLearnNodeState();
int ZW_SetPromiscuousMode(){return 0;}
int ZW_SetRFReceiveMode(){return 0;}
int ZW_SetRoutingMAX(){return 0;}
int ZW_SetSlaveLearnMode();
int ZW_SetSleepMode(){return 0;}
int ZW_SetSUCNodeID();
int ZW_SetWutTimeout(){return 0;}
int ZW_Support9600Only();
int ZW_Type_Library();
int ZW_WatchDogDisable(){return 0;}
int ZW_WatchDogEnable(){return 0;}
int ZW_WatchDogKick(){return 0;}
int ZW_WatchDogEnable(){return 0;}
int ZW_WatchDogDisable(){return 0;}
int ZW_NVRGetValue(){return 0;}
int ZW_ClearTxTimers(){return 0;}
int ZW_GetTxTimer(){return 0;}
*/

#endif
