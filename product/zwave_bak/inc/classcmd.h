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

	COMMAND_CLASS_BATTERY = 0X80,

	COMMAND_CLASS_NOTIFACTION = 0x71,

	COMMAND_CLASS_WAKE_UP = 0x84,
}emClass_t;

typedef enum emCmd {
	BASIC = 0x03,
	VERSION_COMMAND_CLASS = 0x14,
	VERSION = 0x12,
	ZWAVEPLUS_INFO = 0x02,
	SWITCH_BINARY = 0x03,
	ASSOCIATION = 0x03,
	ASSOCIATION_GROUPINGS = 0x06,
	BATTERY = 0x03,
	MANUFACTURER_SPECIFIC = 0x05, 
	NOTIFICATION = 0x05,
	NOTIFICATION_SUPPORTED = 0x08,
	WAKE_UP_INTERNAL = 0x06,
	WAKE_UP_NOTIFICATION = 0x07,
	WAKE_UP_NO_MORE_INFORMATION = 0x0a,
}emCmd_t;

#define CLASS_MAX_ATTR_NUM 0x32

typedef void (*SET_FUNC)(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
typedef void (*GET_FUNC)(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
typedef void (*REPORT_FUNC)(int did, int cid, int aid, char *buf, char *value, int value_len);
typedef void (*REMOVE_FUNC)(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
typedef void (*NOTIFY_FUNC)(int did, int cid, int aid, char *argv[], int argc);

typedef struct stAttr {
	int		aid;	
	char	*name;
	int		usenick;
	char	*nick;

	SET_FUNC set;
	GET_FUNC get;
	REPORT_FUNC report;
	REMOVE_FUNC remove;
	NOTIFY_FUNC notify;
}stAttr_t;

typedef struct stClass {
	int				cid;
	char			*name;
	int				usenick;
	char			*nick;
	int				attrs_cnt;
	stAttr_t	attrs[CLASS_MAX_ATTR_NUM];
}stClass_t;

void class_cmd_get_attr(int did, int cid, int aid, char *argv[], int argc, char *param, int *len);
void class_cmd_set_attr(int did, int cid, int aid, char *argv[], int argc, char *param, int *len);
void class_cmd_rpt_attr(int did, int cid, int aid, char *buf, char *value, int value_len);


#define MAX_SPECIFIC_NUM 0x32

typedef struct stBasic {
	char			val;
	char			*name;
}stBasic_t;
typedef struct stSpecific {
	char			val;
	char			*name;
	char			*nick;
}stSpecific_t;
typedef struct stGeneric {
	char			val;
	char			*name;
	stSpecific_t specifics[MAX_SPECIFIC_NUM];
}stGeneric_t;
const char *class_cmd_basic2str(char b);
const char *class_cmd_generic2str(char g);
const char *class_cmd_specific2str(char g, char s);


typedef struct stProductId {
	char			*pi;
	char			*productid;
	char			*dusunpi;
}stProductId_t;

typedef struct stProductType {
	char				*pt;
	char				*producttype;
	int					 pin;
	stProductId_t	*pis;
}stProductType_t;

typedef struct stManufacturer {
	char						*m;
	char						*manufacturer;
	int							 ptn;
	stProductType_t	*pts;
}stManufacturerSpecific_t;


const char *manufacturer_specific2model(const char *ms);
const char *manufacturer_specific2type(const char *ms);
#endif
