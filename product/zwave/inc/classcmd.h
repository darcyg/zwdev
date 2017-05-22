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
}emClass_t

typedef enum emCmd {
	BASIC_GET = 0x01,
	BASIC_REPORT = 0x02,
	BASIC_SET = 0x03,
	
	VERSION_COMMAND_CLASS_GET = 0x01,
	VERSION_COMMAND_CLASS_REPORT = 0x01,
	VERSION_GET = 0x01,
	VERSION_SET = 0x02,

	ZWAVEPLUS_INFO_GET = 0x01,
	ZWAVEPLUS_INFO_REPORT = 0X02,

	SWITCH_BINARY_GET = 0x1,
	SWITCH_BINARY_REPORT = 0X02,
	SWITCH_BINARY_SET = 0x03,

	ASSOCIATION_GET = 0x01,
	ASSOCIATION_GROUPINGS_GET = 0x02,
	ASSOCIATION_GROUPINGS_REPORT = 0x03,
	ASSOCIATION_REMOVE = 0x03,
	ASSOCIATION_REPORT = 0x04,
	ASSOCIATION_SET = 0x05,
	ASSOCIATION_SPECIFIC_GROUP_GET,
	ASSOCIATION_SPECIFIC_GROUP_REPORT,

	ASSOCIATION_GROUP_NAME_GET,
	ASSOCIATION_GROUP_NAME_REPORT,
	ASSOCIATION_GROUP_INFO_GET,
	ASSOCIATION_GROUP_INFO_REPORT,
	ASSOCIATION_GROUP_COMMAND_LIST_GET,
	ASSOCIATION_GROUP_COMMAND_LIST_REPORT,

	MANUFACTURER_GET
	MANUFACTURER_REPORT,

	MANUFACTURER_SPECIFIC_GET,
	MANUFACTURER_SPECIFIC_REPORT,
	DEVICE_SPECIFIC_GET,
	DEVICE_SPECIFIC_REPORT,

	DEVICE_RESET_LOCALLY_NOTIFICATION,

	POWERLEVEL_GET,
	POWERLEVEL_REPORT,
	POWERLEVEL_SET,
	POWERLEVEL_TEST_NODE_GET,
	POWERLEVEL_TEST_NODE_REPORT,
	POWERLEVEL_TEST_NODE_SET,
}emCmd_t;


typedef struct stClassCmdParam {
}stClassCmdParam_t;

typedef struct stAttr {
	int		aid;	
	char	name[32];
	void	*set;
	void	*get;
	void	*report;
}stAttr_t;

typedef struct stClass {
	int				cid;
	char			name[32];
	int				attrs_cnt;
	stAttr_t	attrs[CLASS_MAX_ATTR_NUM];
}stClass_t;


stClass_t classes[] = {
	[COMMAND_CLASS_BASIC_V1] = {
		{COMMAND_CLASS_BASIC_V1, "basic_v1", 1, {
				{BASIC, "basic", basic_get, basic_set, basic_report},
			},
		},
	},
	[COMMAND_CLASS_VERSION_V1] = {
		{COMMAND_CLASS_VERSION_V1, "version_v1", 2, {
				{VERSION_COMMAND_CLASS, "version_command_class", version_command_class_get, verision_command_class_set, version_command_class_report},
				{VERSION, "version", version_get, verision_set, version_report},
			},
		},
	},
	[COMMAND_CLASS_VERSION_V2] = {
		{COMMAND_CLASS_VERSION_V1, "version_v1", 2, {
				{VERSION_COMMAND_CLASS, "version_command_class", version_command_class_get, verision_command_class_set, version_command_class_report},
				{VERSION, "version", version_get, verision_set, version_report},
			},
		},
	},
	[COMMAND_CLASS_SWITCH_BINARY_V1] = {
		{COMMAND_CLASS_SWITCH_BINARY_V1, "switch_binary", 1, {
				{SWITCH_BINARY, "switch_binary", switch_binary_get, switch_binary_set, switch_binary_report},
			},
		},
	},
};


int class_cmd_get(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi);
int class_cmd_set(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi);
int class_cmd_report(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDAtaInt_t *sdi);
int class_cmd_parse(emClass_t *class, emCmd_t *cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi);

#endif
