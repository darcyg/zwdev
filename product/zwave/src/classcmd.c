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
	{0x07, "sensor notification"},
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

const char *class_cmd_basic2str(char b) {
	int i;
	for (i = 0; i < sizeof(basics)/sizeof(basics[0]); i++) {
		if (b == basics[i].val && basics[i].name[0] != 0) {
			return basics[i].name;
		}
	}
	return "unknown";
}
const char *class_cmd_generic2str(char g) {
	int i;
	for (i = 0; i < sizeof(generics)/sizeof(generics[0]); i++) {
		if (g == generics[i].val && generics[i].name[0] != 0) {
			return generics[i].name;
		}
	}
	return "unknown";
}
const char *class_specific2str(char g, char s) {
	int i;
	for (i = 0; i < sizeof(generics)/sizeof(generics[0]); i++) {
		if (g == generics[i].val && generics[i].name[0] != 0) {
			int j;
			for (j = 0; j < sizeof(generics[i].specifics)/sizeof(generics[i].specifics[0]); j++) {
				if (s == generics[i].specifics[j].val && generics[i].specifics[j].name[0] != 0) {
					if (generics[i].specifics[j].nick[0] != 0) {
						return generics[i].specifics[j].nick;
					} else {
						return generics[i].specifics[j].name;
					}
				}
			}
		}
	}
	return "unknown";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void basic_get(int did, int cid, int aid, char *argv[], int argc);
static void basic_set(int did, int cid, int aid, char *argv[], int argc);
static void basic_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void version_command_class_get(int did, int cid, int aid, char *argv[], int argc);
static void version_command_class_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void version_get(int did, int cid, int aid, char *argv[], int argc);
static void version_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void zwaveplus_info_get(int did, int cid, int aid, char *argv[], int argc);
static void zwaveplus_info_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void switch_binary_get(int did, int cid, int aid, char *argv[], int argc);
static void switch_binary_set(int did, int cid, int aid, char *argv[], int argc);
static void switch_binary_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void association_get(int did, int cid, int aid, char *argv[], int argc);
static void association_set(int did, int cid, int aid, char *argv[], int argc);
static void association_report(int did, int cid, int aid, char *buf, char *value, int value_len);
static void association_remove(int did, int cid, int aid, char *argv[], int argc);

static void association_groupings_get(int did, int cid, int aid, char *argv[], int argc);
static void association_groupings_report(int did, int cid, int aid, char *buf, char *value, int value_len);

static void battery_get(int did, int cid, int aid, char *argv[], int argc);
static void battery_report(int did, int cid, int aid, char *buf, char *value, int value_len);


static void manufacturer_specific_get(int did, int cid, int aid, char *argv[], int argc);
static void manufacturer_specific_report(int did, int cid, int aid, char *buf, char *value, int value_len);

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
				[VERSION_COMMAND_CLASS] = {VERSION_COMMAND_CLASS, "version_command_class", 0, "version_command_class", 
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
#if 0
	[COMMAND_CLASS_ASSOCIATION_V1] = {
		COMMAND_CLASS_ASSOCIATION_V1, "association_v1", 1, "association", 2, {
				[ASSOCIATION] = {ASSOCIATION, "association", 1, "association", 
					association_get, association_set, association_report, association_remove, NULL},

				[ASSOCIATION_GROUPINGS] = {ASSOCIATION_GROUPINGS, "association_groupings", 1, "association_groupings", 
					association_groupings_get,NULL, association_groupings_report, NULL, NULL},
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
		COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1, "manufacturer_specific_v1", 1, "msv1", 1, {
				[MANUFACTURER_SPECIFIC] = {MANUFACTURER_SPECIFIC, "manufacturer_specific", 1, "msv1", 
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

		attr->get(did, cid, aid, argv, argc);
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

		if (attr->set != NULL) {
			attr->set(did, cid, aid, argv, argc);
			if (attr->get != NULL) {
				attr->get(did, cid, aid, NULL, 0);
			}
		}
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
			cid&0xff, (aid-2)&0xff,
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
			cid&0xff, (aid-1)&0xff,
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

	funcID++;

	return 0;
}

static void basic_get(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");
	device_get_attr(did, cid, aid, NULL, 0, param, len);
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
	device_get_attr(did, cid, aid, NULL, 0, param, len);
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
	device_get_attr(did, cid, aid, NULL, 0, param, len);
}
static void switch_binary_set(int did, int cid, int aid, char *argv[], int argc, void *param, int *len) {
	log_debug("-");

	int onoff;
	if (argc != 1 || sscanf(argv[0], "0x%02x", &onoff) != 1) {
		log_debug("error argments for %s", __func__);
		return;
	}
	char buf[1] = {onoff ? 0xff : 0x00};

	device_set_attr(did, cid, aid, buf, sizeof(buf), param, len);
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
	device_get_attr(did, cid, aid, NULL, 0);
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
	device_get_attr(did, cid, aid, NULL, 0);
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




