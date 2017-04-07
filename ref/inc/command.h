#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "ZW_SerialAPI.h"

#define PARAM_MAX_DATA_SIZE    32
#define PARAM_MAX_STRING_SIZE 62
#define PARAM_NAME_MAX_SIZE 32

#define COMMAND_MAX_PARAM_SIZE 32

#define COMMAND_NAME_MAX_SIZE 32

/* parse hex param data from zwave controller to json string data */
typedef bool (*PARSE_FUNC)(char *data, int dSize, char *string, int sSize);
/* packet json string data to hex param */
typedef bool (*PACK_FUNC)(char *string, int sSize, char *data, int dSize);

typedef struct _stParam {
  const char name[PARAM_NAME_MAX_SIZE];
  char data[PARAM_MAX_DATA_SIZE];
  char string[PARAM_MAX_STRING_SIZE];
  PARAM_PARSE_FUNC parse;
  PARAM_PACK_FUNC pack;
}stParam_t;

typedef struct _stCommand {
  const char name[COMMAND_NAME_MAX_SIZE];
  int id;
  int type;
  int support;
	PARSE_FUNC parse;
	PACK_FUNC pack;	
}stCommand_t;

stCommand_t commands[] = {
  {"SerialApiGetInitData",        FUNC_ID_SERIAL_API_GET_INIT_DATA, 0, 1, {
		serial_api_get_init_data_parse, serial_api_get_init_data_pack,
	}},

  {"SerialApiApplNodeInfomation", FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION, 0, 1, {
	}},

  {"ApplicationCommandHandler", 	FUNC_ID_APPLICATION_COMMAND_HANDLER, 0, 1, {
	}},

  {"ZwGetControllerCapabilities", FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES, 0, 1, {
	}},
};

bool commandParse(int id, int type, char *buf, int len,  char *string, int size);
bool commandPack(char *string, int size, int *id, int *type, char *buf, int *len);

#endif
