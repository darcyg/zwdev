#ifndef _API_H_
#define _API_H_

/* this file contains zwave static controller serial api interfaces */

#if 0
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

	CmdApplicationControllerUpdate = 0x49,

	CmdZWaveRemoveNodeFromNetwork = 0x4B,


	CmdZWaveSetSucNodeId = 0x54,

	CmdZWaveSendData = 0x13,

	CmdZWaveSendDataAbort = 0x16,

	CmdZWaveIsFailedNode = 0x62,

	CmdZWaveReplaceFailedNode = 0x63,
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

typedef struct stApiStateMachine {
	emApi_t	api;
	char *name;
	int param_size;
	int num_state;
	stApiState_t states[8];
}stApiStateMachine_t;

typedef struct stApiCall {
	emApi_t api;
	int state;
	stParam_t *param;
	int paramSize;
}stApiCall_t;

typedef void (*API_CALL_CALLBACK)(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error);
typedef void (*API_RETURN_CALLBACK)(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error);

#define API_EXEC_TIMEOUT_MS (1000*2)

int api_init(void *_th, API_CALL_CALLBACK _accb, API_RETURN_CALLBACK _arcb);
int api_exec(emApi_t api, stParam_t *param);
int api_free();
int api_getfd();
int api_step();

const char *api_name(emApi_t api);
void api_param_view(emApi_t api, stParam_t *param, emApiState_t state);
#elif 0

typedef enum emApiMachineState {
	AMS_IDLE = 0,
	AMS_RUNNING = 1,
}emApiMachineState_t;

typedef enum  emApiMachineEvent {
	AME_CALL_API = 0,
	AME_ACK = 1,
	AME_ERROR = 2,
	AME_DATA = 3,
	AME_ASYNC_DATA= 4,
	AME_CALL_EXIT = 5,
	AME_FREE_RUN = 6,
}emApiMachineEvent_t;

typedef struct stApiMachineAction {
	void *exec;
}stApiMachineAction;

typedef struct stApiMachineTransition {
	void *trans;
}stApiMachineTransition_t;

typedef struct stApiMachineEvent {
	emApiMachineEvent event;
	void *event_data;
}stApiMachineEvent_t;


typedef em emApiCallMachine {
	stAcmTest,
}emApiCallMachine_t;
typedef struct stApiMachineEnv {
	int state;
	void *event_queue;
	emApiCallMachine_t *acm;
}stApiMachineEnv_t;

int am_transition_call_api();
int am_transition_call_exit();
int am_action_idle_async_data();
int am_action_running_data();
int am_action_running_ack();
int am_transition_running_err();
int am_action_running_call_async_api();
int am_action_running_async_data();


int api_init();
int api_free();
int api_getfd();
int api_step();
#else

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

	CmdZWaveReplaceFailedNode = 0x63,
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


#define API_EXEC_TIMEOUT_MS (1000*2)

typedef void (*API_CALL_CALLBACK)(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error);
typedef void (*API_RETURN_CALLBACK)(emApi_t api, stParam_t *param, emApiState_t state, emApiError_t error);


int api_init(void *_th, API_CALL_CALLBACK _accb, API_RETURN_CALLBACK _arcb);
int api_free();
int api_call(emApi_t api, stParam_t *param, int param_size);
int api_getfd();
int api_step();


#endif

#endif
