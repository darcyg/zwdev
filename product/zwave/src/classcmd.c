#include "classcmd.h"
#include "api.h"
#include "jansson.h"
#include "hashmap.h"
#include <string.h>
#include "common.h"
#include "file_io.h"
#include "log.h"

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


/* if usenick = 1, nick must has value */
stClass_t classes[] = {
#if 0
	[COMMAND_CLASS_BASIC_V1] = {
		COMMAND_CLASS_BASIC_V1, "basic_v1", 1, "basic", 1, {
			[BASIC] = {BASIC, "basic", 1, "basic", 
					basic_get, basic_set, basic_report, NULL, NULL},  
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
	[COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1] = {
		{COMMAND_CLASS_MANUFACTURER_SPECIFIC_V1, "manufacturer_specific_v1", 0, "", 1, {
				{MANUFACTURER_SPECIFIC, "manufacturer_specific", 0, "", 
					manufacturer_specific_get,NULL, manufacturer_specific_report, NULL, NULL},
			},
		},
	},
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

/////////////////////////////////////////////////////////////////////////////////////////
static struct hashmap hmattrs;
static char *inventory_dir = "/etc/config/dusun/zwave";


static void add_attrs(json_t *jattrs, const char *attr,  const char *value) {
	if (jattrs == NULL) {
		return;
	}

	json_object_set_new(jattrs, attr, json_string(value));
}

static void *hashmap_alloc_key(const void *_key) {
	const char *key = (const char *)_key;
	int len = strlen(key);
	void *newkey = malloc(len+1);
	if (newkey != NULL) {
		strcpy(newkey, key);
	}
	return newkey;
}

static void hashmap_free_key(void * _key) {
	if (_key == NULL) {
		return;
	}
	free(_key);
}


int memory_module_init() {
	hashmap_init(&hmattrs, hashmap_hash_string, hashmap_compare_string, 254);
	hashmap_set_key_alloc_funcs(&hmattrs, hashmap_alloc_key, hashmap_free_key);
	return 0;
}

int memory_get_attr(int did, const char *attr, char *value) {
	//log_debug("memory_get_attr: %d, %s", did, attr);

	char sdid[32];
	sprintf(sdid, "%d", did);
	void *vhm = hashmap_get(&hmattrs, sdid);
	if (vhm == NULL) {
		value[0] = 0;
		return -1;
	}
	
	const char *v = hashmap_get((struct hashmap *)vhm, attr);
	if (v == NULL) {
		value[0] = 0;
		return -2;
	}
	
	strcpy(value, v);

	return 0;
}

int memory_set_attr(int did, const char *attr, char *value) {
	char sdid[32];
	sprintf(sdid, "%d", did);

	//log_debug("memory_set_attr: %d, %s, %s", did, attr, value);

	void *vhm = hashmap_get(&hmattrs, sdid);
	if (vhm == NULL) {
		struct hashmap *hm = MALLOC(sizeof(struct hashmap));
		if (hm == NULL) {
			log_debug("out of memory!");
			return -1;
		} 
		hashmap_init(hm, hashmap_hash_string, hashmap_compare_string, 16);
		hashmap_set_key_alloc_funcs(hm, hashmap_alloc_key, hashmap_free_key);
		vhm = hm;

		if (hashmap_put(&hmattrs, sdid, vhm) == NULL) {
			log_debug("error : %s %d", __func__, __LINE__);
			return -2;
		}
	}

	char *tmp_value = (char*)MALLOC(strlen(value)+1);
	if (tmp_value == NULL) {
		log_debug("out of memory!");
		return -3;
	}
	strcpy(tmp_value, value);

	void *old = hashmap_remove((struct hashmap*)vhm, attr);
	if (old != NULL) {
		FREE(old);
	}

	if (hashmap_put((struct hashmap*)vhm, attr, tmp_value) == NULL) {
		return -3;
	}

	return 0;
}


int flash_module_init() {
  if (!file_is_dir(inventory_dir)) {
    file_create_dir(inventory_dir, 755);
  }
	return 0;
}

int flash_load_attr(int did, const char *attr, char *value) {
  char f[256];

  sprintf(f, "%s/%d/%s", inventory_dir, did, attr);
  FILE *fp = fopen(f, "r");
  if (fp != NULL) {
    char buf[64+1];
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size = size > sizeof(buf) ? sizeof(buf) : size;
    int ret = fread(buf, size, 1, fp);
    buf[ret*size] = 0;
    fclose(fp);

    log_debug("load string :%s", buf);
    strcpy(value, buf);
    log_debug("load value is %s", buf);
		return 0;
  }
  *value = 0;
  return -1;
}
int flash_save_attr(int did, const char *attr, char *value) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
  if (!file_is_dir(f)) {
    file_create_dir(f, 755);
  }

  strcat(f, "/");
  strcat(f, attr);
  FILE *fp = fopen(f, "w");
  if (fp != NULL) {
    char buf[32];
    sprintf(buf, "%s", value);
    fwrite(buf, strlen(buf), 1, fp);
    fclose(fp);
  }
	return 0;
}


int class_cmd_to_attrs(int did, emClass_t *class_array, int class_cnt, void *jattrs) {
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

			char value[32];
			if (memory_get_attr(did, attr->name, value) == 0)  {
				add_attrs(jattrs, attr->nick, value);
			} else if (flash_load_attr(did, attr->name, value) == 0) {
				memory_set_attr(did, attr->name, value);
				add_attrs(jattrs, attr->nick, value);
			} else {
				if (attr->get != NULL) {
					attr->get(did, class->cid, attr->aid, NULL, 0);
				}
				add_attrs(jattrs, attr->nick, "");
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

void class_cmd_init() {
	memory_module_init();
	flash_module_init();
}

void class_cmd_get_attr(int did, int cid, int aid, char *argv[], int argc) {
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
void class_cmd_set_attr(int did, int cid, int aid, char *argv[], int argc) {
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
		}
}
/////////////////////////////////////////////////////////////////////////////////////////
static int device_set_attr(int did, int cid, int aid, char *buf, int size) {
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

	api_call(CmdZWaveSendData, (stParam_t*)&sdi, sdi.pData_len + 4);

	funcID++;

	return 0;
}


static int device_get_attr(int did, int cid, int aid, char *buf, int size) {
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

	api_call(CmdZWaveSendData, (stParam_t*)&sdi, sdi.pData_len + 4);

	funcID++;

	return 0;
}

static void basic_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void basic_set(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void basic_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void version_command_class_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void version_command_class_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void version_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
	device_get_attr(did, cid, aid, NULL, 0);
}
static void version_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	sprintf(buf, "L%d-P%d.%d-A%d.%d", value[0], value[1], value[2], value[3], value[4]); 
}

static void zwaveplus_info_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void zwaveplus_info_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}

static void switch_binary_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
	device_get_attr(did, cid, aid, NULL, 0);
}
static void switch_binary_set(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");

	int onoff;
	if (argc != 1 || sscanf(argv[0], "0x%02x", &onoff) != 1) {
		log_debug("error argments for %s", __func__);
		return;
	}
	char buf[1] = {onoff ? 0xff : 0x00};

	device_set_attr(did, cid, aid, buf, sizeof(buf));
}
static void switch_binary_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
	sprintf(buf, "%d", !!value[0]);
}

static void association_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void association_set(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void association_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}
static void association_remove(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}

static void association_groupings_get(int did, int cid, int aid, char *argv[], int argc) {
	log_debug("-");
}
static void association_groupings_report(int did, int cid, int aid, char *buf, char *value, int value_len) {
	log_debug("-");
}



