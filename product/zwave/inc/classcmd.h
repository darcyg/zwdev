#ifndef __CLASSCMD_H_
#define __CLASSCMD_H_

//5E 25 85 59  86 72 5A 73  
typedef enum emClass {
	COMMAND_CLASS_BASIC_V1 = 0x20,

	COMMAND_CLASS_VERSION_V1 = 0X86,
	COMMAND_CLASS_VERSION_V2 = 0X86,

	COMMAND_CLASS_ZWAVEPLUS_INFO_V1 = 0x5e,
	COMMAND_CLASS_ZWAVEPLUS_INFO_V2 = 0x5e,

	COMMAND_CLASS_SWITCH_BINARY_V1 = 0x25,

	COMMAND_CLASS_ASSOCIATION_V1 = 0X85,
	COMMAND_CLASS_ASSOCIATION_V2 = 0X85,

	COMMAND_CLASS_ASSOCIATION_GRP_INFO_V1 = 0X59,

	COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1 = 0X72,
	COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2 = 0X72,

	COMMAND_CLASS_DEVICE_RESET_LOCALLY_V1 = 0x5A,

	COMMAND_CLASS_POWERLEVEL_V1 = 0x73,
}emClass_t;

typedef enum emCmd {
	BASIC = 0x00,
	VERSION_COMMAND_CLASS = 0x14,
	VERSION = 0x12,
	ZWAVEPLUS_INFO = 0x02,
	SWITCH_BINARY = 0x03,
	ASSOCIATION = 0x03,
	ASSOCIATION_GROUPINGS = 0x06,
}emCmd_t;

#define CLASS_MAX_ATTR_NUM 0x32

typedef struct stAttr {
	int		aid;	
	char	*name;
	int		usenick;
	char	*nick;
	void	*set;
	void	*get;
	void	*report;
	void  *remove;
	void	*notify;
	void  *parse;
}stAttr_t;

typedef struct stClass {
	int				cid;
	char			*name;
	int				usenick;
	char			*nick;
	int				attrs_cnt;
	stAttr_t	attrs[CLASS_MAX_ATTR_NUM];
}stClass_t;

extern stClass_t classes[];

/*
int class_cmd_get(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi);
int class_cmd_set(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi);
int class_cmd_report(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDAtaInt_t *sdi);
int class_cmd_remove(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDAtaInt_t *sdi);
int class_cmd_parse(emClass_t *class, emCmd_t *cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi);
*/


int memory_module_init();
int flash_module_init();
int device_module_init();

int memory_get_attr(int did, const char *attr, char *value);
int memory_set_attr(int did, const char *attr, char *value);

int flash_load_attr(int did, const char *attr, char *value);
int flash_save_attr(int did, const char *attr, char *value);

int device_get_attr(int did, int cid, int aid);
//int device_set_attr(int did, const char *attr, char *value);


int class_cmd_to_attrs(int did, emClass_t *class, int class_cnt, void *jattrs);
void class_cmd_save(int did, char cid, char op, char *value, char value_len);

void class_cmd_init();
#endif

