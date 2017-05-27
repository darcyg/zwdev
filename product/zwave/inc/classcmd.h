#ifndef __CLASSCMD_H_
#define __CLASSCMD_H_


typedef enum emClass {
	COMMAND_CLASS_BASIC_V1 = 0x01,
	COMMAND_CLASS_VERSION_V1 = 0X02,
	COMMAND_CLASS_VERSION_V2 = 0X03,
	COMMAND_CLASS_ZWAVEPLUS_INFO_V1 = 0x04,
	COMMAND_CLASS_ZWAVEPLUS_INFO_V2 = 0x05,
	COMMAND_CLASS_SWITCH_BINARY_V1 = 0x06,
	COMMAND_CLASS_ASSOCIATION_V1 = 0X07,
	COMMAND_CLASS_ASSOCIATION_V2 = 0X08,
	COMMAND_CLASS_ASSOCIATION_GRP_INFO_V1 = 0X09,
	COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1 = 0X10,
	COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2 = 0X11,
	COMMAND_CLASS_DEVICE_RESET_LOCALLY_V1 = 0x12,
	COMMAND_CLASS_POWERLEVEL_V1 = 0x13,
}emClass_t;

typedef enum emCmd {
	BASIC = 0x01,
	VERSION_COMMAND_CLASS = 0x02,
	VERSION = 0X03,
	ZWAVEPLUS_INFO = 0x04,
}emCmd_t;

#define CLASS_MAX_ATTR_NUM 16

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

int device_get_attr(int did, const char *attr, char *value);


int class_cmd_to_attrs(int did, emClass_t *class, int class_cnt, void *jattrs);
#endif

