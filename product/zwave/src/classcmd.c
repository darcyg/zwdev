#include "classcmd.h"
#include "api.h"
#include "jansson.h"
#include "hashmap.h"
#include <string.h>
#include "common.h"
#include "file_io.h"
#include "log.h"
#include "app.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
stBasic_t basics[] = {
	{0x01, "controller"},
	{0x02, "static controller"},
	{0x03, "slave"},
	{0x04, "routing slave"},
};
stGeneric_t generics[] = {
	{0x01, "controller"},
	{0x02, "static controller"},
	{0x03, "av control point"},
	{0x04, "display"},
	{0x05, "network extender"},
	{0x06, "appliance"},
	{0x07, "sensor notification", {
			{0x00, "not used"},
			{0x01, "notifaction", "1209"},
		},
	},
	{0x08, "thermostat"},
	{0x09, "window convering"},
	{0x0F, "repeater slave"},
	{0x10, "switch binary", {	
			{0x00, "not used",							"1212"},
			{0x01, "on/off power switch",		"1212"},
			{0x02, "color tunable",					"1213"},
			{0x03, "scene switch",					"1212"},
			{0x04, "power strip",						"1212"},
			{0x05, "siren",									"1212"},
			{0x06, "open/close",						"1212"},
			{0x07, "irrigation controller", "1212"},
		},
	},
	{0x11, "switch multilevel"},
	{0x12, "switch remote"},
	{0x13, "switch toggle"},
	{0x15, "zip node"},
	{0x16, "ventilation"},
	{0x17, "security pannel"},
	{0x18, "wall controller"},
	{0x20, "binary sensor"},
	{0x21, "multilevel sensor"},
	{0x30, "pulse meter"},
	{0x31, "meter"},
	{0x40, "entry control"},
	{0x50, "semi interoperable"},
	{0xA1, "sensor alarm"},
	{0xFF, "non interoperable"},
};

static stProductId_t pis_zwave_plus[] = {
	{"0001", "door",				"1203"},
	{"0002", "switch",			"1212"},
	{"0003", "pir",					"1209"},
	{"0006", "securepir",		"1209"},
	{"0007", "secureswitch","1212"},
};

static stProductType_t pts_sigma_design[] = {
	{"0001", "zip",		0, NULL},
	{"0002", "zwave", 0, NULL},
	{"0003", "zplus", sizeof(pis_zwave_plus)/sizeof(pis_zwave_plus[0]), pis_zwave_plus},
};

static stManufacturerSpecific_t ms[] = {
	{"0000", "Sigma Designs", sizeof(pts_sigma_design)/sizeof(pts_sigma_design[0]), pts_sigma_design},
};

const char *class_cmd_class2type(char *cls, int cnt) {
	int i = 0;
	for (i = 0; i < cnt; i++) {
		if ((cls[i]&0xff) == COMMAND_CLASS_SWITCH_BINARY_V1) {
			return "1212"; //light
		}
		if ((cls[i]&0xff) == COMMAND_CLASS_NOTIFACTION) {
			return "1209";
		}
	}
	return "unknow";
}


const char *class_cmd_manufacturer_specific2model(const char *strms) {
	char m[8];
	char t[8];
	char s[8];

	strncpy(m, strms + 0, 4);
	strncpy(t, strms + 4, 4);
	strncpy(s, strms + 8, 4);

	int i;
	for (i = 0; i < sizeof(ms)/sizeof(ms[0]); i++) {
		if (strcmp(m, ms[i].m) == 0) {
			return ms[i].m;
		}
	}
	return "unknwon";
}
const char *class_cmd_manufacturer_specific2type(const char *strms) {
	char m[8];
	char t[8];
	char s[8];

	strncpy(m, strms + 0, 4);
	strncpy(t, strms + 4, 4);
	strncpy(s, strms + 8, 4);

	int i, j, h;
	for (i = 0; i < sizeof(ms)/sizeof(ms[0]); i++) {
		stManufacturerSpecific_t *pms = &ms[i];
		if (strcmp(m, pms->m) == 0) {
			for (j = 0; j < pms->ptn; j++) {
				stProductType_t *ppt = &pms->pts[i];
				if (strcmp(t, ppt->pt) == 0) {
					for (h = 0; h < ppt->pin; h++) {
						stProductId_t *ppi = &ppt->pis[h];
						if (strcmp(ppi->pi,s) == 0) {
							return ppi->dusunpi;
						}
					}
				}
			}
		}
	}
	return "unknwon";
}

const char *class_cmd_basic2str(char b) {
	int i;
	for (i = 0; i < sizeof(basics)/sizeof(basics[0]); i++) {
		if (b == basics[i].val && basics[i].name[0] != 0) {
			return basics[i].name;
		}
	}
	log_debug("unknown basic : %02x", b);
	return "unknown";
}
const char *class_cmd_generic2str(char g) {
	int i;
	for (i = 0; i < sizeof(generics)/sizeof(generics[0]); i++) {
		if (g == generics[i].val && generics[i].name[0] != 0) {
			return generics[i].name;
		}
	}
	log_debug("unknown generic : %02x", g);
	return "unknown";
}
const char *class_cmd_specific2str(char g, char s) {
	int i;
	for (i = 0; i < sizeof(generics)/sizeof(generics[0]); i++) {
		if (g == generics[i].val && generics[i].name[0] != 0) {
			int j;
			for (j = 0; j < sizeof(generics[i].specifics)/sizeof(generics[i].specifics[0]); j++) {
				if (s == generics[i].specifics[j].val && generics[i].specifics[j].name[0] != 0) {
						return generics[i].specifics[j].nick;
				}
			}
		}
	}
	log_debug("unknown specific(generic) : %02x(%02x)", s, g);
	return "unknown";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void basic_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void basic_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void basic_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void version_command_class_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void version_command_class_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void version_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void version_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void zwaveplus_info_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void zwaveplus_info_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void switch_binary_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void switch_binary_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void switch_binary_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void association_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void association_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void association_report(int did, int cid, int aid, char *buf, char *value, int value_len);
static void association_remove(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);

static void association_groupings_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void association_groupings_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void battery_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void battery_report(int did, int cid, int aid, char *buf, char *value, int value_len);


static void manufacturer_specific_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void manufacturer_specific_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void notify_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void notify_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void notify_report(int did, int cid, int aid, char *buf, char *value, int value_len);


static void notify_supported_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void notify_supported_report(int did, int cid, int aid, char *buf, char *value, int value_len);


static void internal_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void internal_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void internal_report(int did, int cid, int aid, char *buf, char *value, int value_len);


static void wakeup_notify_report(int did, int cid, int aid, char *buf, char *value, int value_len);


static void wakeup_nomore_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);

static void association_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void association_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);
static void association_report(int did, int cid, int aid, char *buf, char *value, int value_len);
static void association_remove(int did, int cid, int aid, char *argv[], int argc, void *param, int *len);


/* if usenick = 1, nick must has value */
stClass_t classes[] = {
#if 1
	[COMMAND_CLASS_BASIC_V1] = {
		COMMAND_CLASS_BASIC_V1, "basic_v1", 1, "basic", 1, {
			[BASIC] = {BASIC, "basic", 0, "basic", 
					basic_set, basic_get, basic_report, NULL, NULL},  
		},
	},
#endif
	[COMMAND_CLASS_VERSION_V1] = {
		COMMAND_CLASS_VERSION_V1, "version_v1", 1, "version_1", 2, {
				[VERSION_COMMAND_CLASS] = {VERSION_COMMAND_CLASS, "version_command_class", 0, "", 
								NULL, version_command_class_get, version_command_class_report, NULL, NULL},
				[VERSION] = {VERSION, "version", 1, "version", 
								NULL, version_get, version_report, NULL, NULL},
		},
	},
	[COMMAND_CLASS_BATTERY] = {
		COMMAND_CLASS_BATTERY, "battery", 1, "battery", 1, {
			[BATTERY] = {BATTERY, "battery", 1, "battery", 
								NULL, battery_get, battery_report, NULL, NULL},
		},
	},
	[COMMAND_CLASS_NOTIFACTION] = {
		COMMAND_CLASS_NOTIFACTION, "notify", 1, "notify", 2,  {
			[NOTIFICATION] = {NOTIFICATION, "notify", 1, "notify", 
								notify_set, notify_get, notify_report, NULL, NULL},
			[NOTIFICATION_SUPPORTED] = {NOTIFICATION_SUPPORTED, "notify_supported", 1, "",
								NULL, notify_supported_get, notify_supported_report, NULL, NULL},
		},
	},
	[COMMAND_CLASS_WAKE_UP] = {
		COMMAND_CLASS_WAKE_UP, "wakeup", 1, "wakeup", 3, {
			[WAKE_UP_INTERNAL] = {WAKE_UP_INTERNAL, "internal", 1, "",
								internal_set, internal_get, internal_report, NULL, NULL},
			[WAKE_UP_NOTIFICATION] = {WAKE_UP_NOTIFICATION, "wakeup_notify", 1, "",
								NULL, NULL, wakeup_notify_report, NULL, NULL},
			[WAKE_UP_NO_MORE_INFORMATION] = {WAKE_UP_NO_MORE_INFORMATION, "nomore", 1, "", 
								wakeup_nomore_set, NULL, NULL, NULL, NULL},
		},
	},
	/*
	[COMMAND_CLASS_VERSION_V2] = {
		COMMAND_CLASS_VERSION_V2, "version_v2", 1, "version_v2", 2, {
				[VERSION_COMMAND_CLASS] = {VERSION_COMMAND_CLASS, "version_command_class", 1, "version_command_class", 
							 version_command_class_get,NULL, version_command_class_report, NULL, NULL},
				[VERSION] = {VERSION, "version", 1, "version", 
							 version_get, NULL, version_report, NULL, NULL},
		},
	},
	*/
#if 0
	[COMMAND_CLASS_ZWAVEPLUS_INFO_V1] = {
		COMMAND_CLASS_ZWAVEPLUS_INFO_V1, "zwave_info_v1", 1, "zwave_info_v1", 1, {
				[ZWAVEPLUS_INFO] = {ZWAVEPLUS_INFO, "zwaveplus_info", 1, "zwaveplus_info",
						 zwaveplus_info_get,NULL, zwaveplus_info_report, NULL, NULL},
		},
	},
#endif
	/*
	[COMMAND_CLASS_ZWAVEPLUS_INFO_V2] = {
		{COMMAND_CLASS_ZWAVEPLUS_INFO_V2, "zwave_info_v2", 0, "", 1, {
				[ZWAVEPLUS_INFO] = {ZWAVEPLUS_INFO, "zwaveplus_info", 0, "", 
							zwaveplus_info_get, NULL, zwaveplus_info_report, NULL, NULL},
			},
		},
	},
	*/
	[COMMAND_CLASS_SWITCH_BINARY_V1] = {
		COMMAND_CLASS_SWITCH_BINARY_V1, "switch_binary", 1, "switch_binary", 1, {
				[SWITCH_BINARY] = {SWITCH_BINARY, "switch_binary", 1, "on_off",
					 switch_binary_set, switch_binary_get, switch_binary_report, NULL, NULL},
		},
	},
#if 1
	[COMMAND_CLASS_ASSOCIATION_V1] = {
		COMMAND_CLASS_ASSOCIATION_V1, "association_v1", 1, "association", 1, {
				[ASSOCIATION] = {ASSOCIATION, "association", 1, "", 
					association_set, NULL, association_report, association_remove, NULL},
				/*
				[ASSOCIATION_GROUPINGS] = {ASSOCIATION_GROUPINGS, "association_groupings", 1, "association_groupings", 
					association_groupings_get,NULL, association_groupings_report, NULL, NULL},
				*/
		},
	},
#endif
	/*
	[COMMAND_CLASS_ASSOCIATION_V2] = {
		{COMMAND_CLASS_ASSOCIATION_V2, "association_v2", 0, "", 2, {
				{ASSOCIATION, "association", 0, "", 
					association_get, association_set, association_report, association_remove, NULL},
				{ASSOCIATION_GROUPINGS, "association_groupings", 0, "",
					association_groupings_get, NULL, association_groupings_report, NULL, NULL},
				{ASSOCIATION_SPECIFIC_GROUP, "association_specific_group", 0, "", 
					association_specific_group_get,NULL, association_specific_group_report, NULL, NULL},
			},
		},
	},
	*/
#if 0
	[COMMAND_CLASS_ASSOCIATION_GRP_INFO_V1] = {
		{COMMAND_CLASS_ASSOCIATION_GRP_INFO_V1, "association_grp_info_v1", 0, "", 3, {
				{ASSOCIATION_GROUP_NAME, "association_group_name", 0, "", 
					association_group_name_get, NULL, association_group_name_report, NULL, NULL},
				{ASSOCIATION_GROUP_INFO, "association_group_info", 0, "", 
					association_group_info_get,NULL, association_group_info_report, NULL, NULL},
				{ASSOCIATION_GROUP_COMMAND_LIST, "association_group_command_list", 0, "", 
					association_group_command_list_get,NULL, association_group_command_list_report, NULL, NULL},
			},
		},
	},
#endif
	[COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1] = {
		COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1, "manufacturer_specific_v1", 1, "mac", 1, {
				[MANUFACTURER_SPECIFIC] = {MANUFACTURER_SPECIFIC, "manufacturer_specific", 1, "mac", 
					NULL, manufacturer_specific_get, manufacturer_specific_report, NULL, NULL},
		},
	},
#if 0
	[COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2] = {
		{COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2, "manufacturer_specific_v2", 0, "", 1, {
				{MANUFACTURER_SPECIFIC, "manufacturer_specific", 0, "", 
					manufacturer_specific_get,NULL, manufacturer_specific_report, NULL, NULL},
				{DEVICE_SPECIFIC, "device_specific", 0, "", 
					device_specific_get, NULL, device_specific_report, NULL, NULL},
			},
		},
	},
	[COMMAND_CLASS_DEVICE_RESET_LOCALLY_V1] = {
		{COMMAND_CLASS_DEVICE_RESET_LOCALLY_V1, "device_reset_locally_v1", 0, "", 1, {
				{DEVICE_RESET_LOCALLY, "device_reset_locally", 0, "", 
						NULL, NULL, NULL, NULL, device_reset_locally_notifaction},
			},
		},
	},
	[COMMAND_CLASS_POWERLEVEL_V1] = {
		{COMMAND_CLASS_POWERLEVEL_V1, "powerlevle_v1", 0, "", 2, {
				{POWERLEVEL, "powerlevel", 0, "", 
					powerlevel_get, powerlevel_set, powerlevel_report, NULL, NULL},
				{POWERLEVEL_TEST, "powerlevel_test", 0, "",
					powerlevel_test_get, powerlevel_test_set, powerlevel_test_report, NULL, NULL},
			},
		},
	},
#endif
};

#if 0

int class_cmd_get_dev_attr(int did, emClass_t *class_array, int class_cnt) {
	int i = 0;
	for (i = 0; i < class_cnt; i++) {
		int cid = class_array[i]&0xff;
		stClass_t *class = &classes[cid];


		if ((class->cid&0xff) != cid) {
			continue;
		}
	
		if (class->attrs_cnt == 0) {
			continue;
		}
	
		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &class->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				continue;
			}
			if (attr->usenick == 0 || attr->nick[0] == 0) {
				continue;
			}

			if (attr->get != NULL) {
				attr->get(did, class->cid, attr->aid, NULL, 0);
			}
		}
	}
	return 0;
}


void class_cmd_save(int did, char cid, char op, char *value, char value_len) {
		log_debug("[%s] ----->%02x, %02x", __func__, did&0xff, cid&0xff);
		log_debug_hex("value:", value, value_len);
	
		stClass_t *class = &classes[cid&0xff];
		if ((class->cid&0xff) != (cid&0xff)) {
			return;
		}
	
		if (class->attrs_cnt == 0) {
			return;
		}
	
		stAttr_t *attr = &class->attrs[op&0xff];
		if (attr->name == NULL || attr->name[0] == 0) {
			return;
		}
		if ((attr->aid&0xff) != op) {
			return;
		}
	
		char value_buf[32];
		if (attr->report != NULL) {
			attr->report(did, class->cid, attr->aid, value_buf, value, value_len&0xff);
		} else {
			sprintf(value_buf, "nil(report)");
		}
		log_debug("[%s] %s, %s, aid:%02x", __func__, attr->name, value_buf, attr->aid&0xff);
		memory_set_attr(did, attr->name, value_buf);
}
#endif

void class_cmd_get_attr(int did, int cid, int aid, char *argv[], int argc, char *param, int *len) {
		stClass_t *class = &classes[cid&0xff];
		if ((class->cid&0xff) != (cid&0xff)) {
		log_debug("%d - %d, %d", __LINE__, class->cid&0xff, cid&0xff);
			return;
		}
	
		if (class->attrs_cnt == 0) {
		log_debug("%d", __LINE__);
			return;
		}
	
		stAttr_t *attr = &class->attrs[aid&0xff];
		if (attr->name == NULL || attr->name[0] == 0) {
		log_debug("%d", __LINE__);
			return;
		}
		if ((attr->aid&0xff) != aid) {
		log_debug("%d", __LINE__);
			return;
		}

		if (attr->get == NULL) {
			log_debug("%d", __LINE__);
			return;
		}

		attr->get(did, cid, aid, argv, argc, param, len);
}
void class_cmd_set_attr(int did, int cid, int aid, char *argv[], int argc, char *param, int *len) {
		stClass_t *class = &classes[cid&0xff];
		if ((class->cid&0xff) != (cid&0xff)) {
			return;
		}
	
		if (class->attrs_cnt == 0) {
			return;
		}
	
		stAttr_t *attr = &class->attrs[aid&0xff];
		if (attr->name == NULL || attr->name[0] == 0) {
			return;
		}
		if ((attr->aid&0xff) != aid) {
			return;
		}

		if (attr->set == NULL) {
			log_debug("%d", __LINE__);
			return;
		}
		attr->set(did, cid, aid, argv, argc, param, len);
}
void class_cmd_rpt_attr(int did, int cid, int aid, char *buf, char *value, int value_len) {
		log_debug("[%s] ----->%02x, %02x", __func__, did&0xff, cid&0xff);
		log_debug_hex("value:", value, value_len);
	
		stClass_t *class = &classes[cid&0xff];
		if ((class->cid&0xff) != (cid&0xff)) {
			return;
		}
	
		if (class->attrs_cnt == 0) {
			return;
		}
	
		stAttr_t *attr = &class->attrs[aid&0xff];
		if (attr->name == NULL || attr->name[0] == 0) {
			return;
		}
		if ((attr->aid&0xff) != aid) {
			return;
		}
	
		char value_buf[32];
		if (attr->report != NULL) {
			attr->report(did, class->cid, attr->aid, value_buf, value, value_len&0xff);
		} else {
			sprintf(value_buf, "nil(report)");
		}
		log_debug("[%s] %s, %s, aid:%02x", __func__, attr->name, value_buf, attr->aid&0xff);

		//memory_set_attr(did, attr->name, value_buf);
		strcpy(buf, value_buf);
}
//------------------------------------------ funcs to implement -------------------------------------
static int device_set_attr(int did, int cid, int aid, char *buf, int size, char *param, int *len) {
	static char funcID = 0x01;
	log_debug("did:%02x, cid:%02x, aid:%02x", did, cid, aid);
	stSendDataIn_t sdi = {
		.nodeID = did&0xff,
		.pData_len = (0x02 + size),
		.pData_data = {
			cid&0xff, (aid)&0xff,
		},	
		.txOptions = 0x25,
		.funcID = funcID,
	};
	if (size > 0) {
		memcpy(&sdi.pData_data[2], buf, size);
	}
	sdi.pData_data[2+size] = sdi.txOptions;
	sdi.pData_data[3+size] = sdi.funcID;

	memcpy(param, &sdi, sdi.pData_len + 4);
	*len = sdi.pData_len + 4;

	funcID++;

	return 0;
}


static int device_get_attr(int did, int cid, int aid, char *buf, int size, char *param, int *len) {
	static char funcID = 0x01;
	log_debug("did:%02x, cid:%02x, aid:%02x", did, cid, aid);
	stSendDataIn_t sdi = {
		.nodeID = did&0xff,
		.pData_len = (0x02 + size),
		.pData_data = {
			cid&0xff, (aid)&0xff,
		},	
		.txOptions = 0x25,
		.funcID = funcID,
	};
	if (size > 0) {
		memcpy(&sdi.pData_data[2], buf, size);
	}
	sdi.pData_data[2+size] = sdi.txOptions;
	sdi.pData_data[3+size] = sdi.funcID;

	memcpy(param, &sdi, sdi.pData_len + 4);
	*len = sdi.pData_len + 4;

	funcID++;

	return 0;
}

static void basic_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}
static void basic_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
}
static void basic_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void version_command_class_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
}
static void version_command_class_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void version_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}
static void version_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	sprintf(buf, "L%d-P%d.%d-A%d.%d", value[0], value[1], value[2], value[3], value[4]); 
}

static void zwaveplus_info_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
}
static void zwaveplus_info_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void switch_binary_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}
static void switch_binary_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");

	int onoff;
	if (argc != 1 || sscanf(argv[0], "%d", &onoff) != 1) {
		log_debug("error argments for %s", __func__);
		return;
	}
	char buf[1] = {onoff ? 0xff : 0x00};

	device_set_attr(did, cid, aid-2, buf, sizeof(buf), param, len);
}
static void switch_binary_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	sprintf(buf, "%d", !!value[0]);
}

static void association_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
}
static void association_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");

	char buf[2] = {0x01, 0x01};
	device_set_attr(did, cid, aid-2, buf, sizeof(buf), param, len);
}
static void association_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}
static void association_remove(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
}

static void association_groupings_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
}
static void association_groupings_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void battery_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}
static void battery_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	char battery = value[0];
	if (battery >= 0x00 && battery <= 0x64) {
		sprintf(buf, "%d", battery);
	} else {
		/* low warning */
		sprintf(buf, "0");
	}
}

static void manufacturer_specific_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}
static void manufacturer_specific_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	/*
	short wid = *(short *)(value + 0);
	short tid = *(short *)(value + 2);	
	short pid = *(short *)(value + 4);
	*/
	sprintf(buf, "%02x%02X%02x%02x%02x%02x", value[0],value[1],value[2],value[3],value[4],value[5]);
}

static void notify_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	char buf[3] = {0x07, 0x02};
	device_set_attr(did, cid, aid+1, buf, 2, param, len);
}

static void notify_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	char buf[3] = {0x00, 0x07, 0x00};
	device_get_attr(did, cid, aid-1, buf, 1, param, len);
}

static void notify_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	if (value[3] == 0x00) {
		//sprintf(buf, "%02x", 0);
		sprintf(buf, "%s", "");
	} else {
		if (value[4] == 0x07 && value[5] == 0x08) {
			sprintf(buf, "pir:%d", !!value[3]);
		} else {
			sprintf(buf, "xxx:%d", !!value[3]);
		}
	}
	log_debug("-");
}

static void notify_supported_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}

static void notify_supported_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	log_debug_hex("notify_supportted:", value, value_len);
}

static void internal_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	char buf[4] = {0x00, 0x00, 0x10, 0x01};
	device_set_attr(did, cid, aid-2, buf, sizeof(buf), param, len);
}

static void internal_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid-1, NULL, 0, param, len);
}
static void internal_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	int  seconds= value[0] * 256 * 256 + value[1] *256  +  value[2];
	char nodeid = value[3];
	sprintf(buf, "%d(%02x)", seconds, nodeid);
}


static void wakeup_notify_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	int curr = time(NULL);
	sprintf(buf, "%d", curr);
}


static void wakeup_nomore_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_set_attr(did, cid, aid-2, NULL, 0, param, len);
}





