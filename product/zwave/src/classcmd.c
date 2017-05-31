#include "classcmd.h"
#include "api.h"
#include "jansson.h"
#include "hashmap.h"
#include <string.h>
#include "common.h"
#include "file_io.h"
#include "log.h"

void basic_get();
void basic_set();
void basic_report();

void version_command_class_get();
void version_command_class_report();
void version_get();
void version_report();
void zwaveplus_info_get();
void zwaveplus_info_report();
void switch_binary_get();
void switch_binary_set();
void switch_binary_report();
void switch_binary_parse(char *buf, char *value, int value_len);
void association_get();
void association_set();
void association_report();
void association_remove();
void association_groupings_get();
void association_groupings_report();


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
								version_command_class_get, NULL, version_command_class_report, NULL, NULL},
				[VERSION] = {VERSION, "version", 1, "version", 
								version_get, NULL, version_report, NULL, NULL},
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
					 switch_binary_get, switch_binary_set, switch_binary_report, NULL, NULL, switch_binary_parse},
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


/*
int class_cmd_get(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi) {
}
int class_cmd_set(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi) {
}
int class_cmd_report(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDAtaInt_t *sdi) {
}
int class_cmd_remove(emClass_t class, emCmd_t cmd, stClassCmdParam_t *param, stSendDAtaInt_t *sdi) {
}
int class_cmd_parse(emClass_t *class, emCmd_t *cmd, stClassCmdParam_t *param, stSendDataIn_t *sdi) {
}
*/
/////////////////////////////////////////////////////////////////////////////////////////
static struct hashmap hmattrs;
static char *inventory_dir = "/etc/config/dusun/zwave";


static void add_attrs(json_t *jattrs, const char *attr,  const char *value) {
	if (jattrs == NULL) {
		return;
	}

	json_object_set(jattrs, attr, json_string(value));
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
	hashmap_init(&hmattrs, hashmap_hash_string, hashmap_compare_string, 60);
	hashmap_set_key_alloc_funcs(&hmattrs, hashmap_alloc_key, hashmap_free_key);
	return 0;
}

int memory_get_attr(int did, const char *attr, char *value) {

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

	void *vhm = hashmap_get(&hmattrs, sdid);
	if (vhm == NULL) {
		struct hashmap *hm = MALLOC(sizeof(struct hashmap));
		hashmap_init(hm, hashmap_hash_string, hashmap_compare_string, 10);
		hashmap_set_key_alloc_funcs(hm, hashmap_alloc_key, hashmap_free_key);
		if (hm == NULL) {
			return -1;
		} 
		vhm = hm;
	}

	if (hashmap_put(vhm, attr, value) == NULL) {
		log_debug("%s %d", __func__, __LINE__);
		return -2;
	}
	
	if (hashmap_put(&hmattrs, sdid, vhm) == NULL) {
		log_debug("%s %d", __func__, __LINE__);
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

int device_module_init() {
	return 0;
}

int device_get_attr(int did, int cid, int aid) {
	//log_debug("[%s]", __func__);
	stSendDataIn_t sdi = {
		.nodeID = did&0xff,
		.pData_len = 0x03,
		.pData_data = {
			cid&0xff, (aid-1)&0xff, 0x00
		},	
		.txOptions = 0x25,
		.funcID = 0x0D,
	};

	sdi.pData_data[3] = sdi.txOptions;
	sdi.pData_data[4] = sdi.funcID;
	api_call(CmdZWaveSendData, (stParam_t*)&sdi, sdi.pData_len + 4);

	return 0;
}
/*
int device_set_attr(int did, const char *attr, char *value) {
	//log_debug("[%s]", __func__);
	return 0;
}
*/


int class_cmd_to_attrs(int did, emClass_t *class_array, int class_cnt, void *jattrs) {
	int i = 0;
	//log_debug("%s - %d", __func__, __LINE__);
	for (i = 0; i < class_cnt; i++) {
		int cid = class_array[i]&0xff;
		//log_debug("search for class: %02x", cid&0xff);
		stClass_t *class = &classes[cid];

		//log_debug("%s - %d, cid:%d, name:%s, nick:%s(usenick:%d)", __func__, __LINE__,
		//		 class->cid, class->name, class->nick, class->usenick);
		if ((class->cid&0xff) != cid) {
			//log_debug("cid not correct: class->cid:%d, cid:%d", class->cid, cid);
			continue;
		}
	
		if (class->attrs_cnt == 0) {
			//log_debug("no attrs");
			continue;
		}
	
		int j = 0;
		for (j = 0; j < CLASS_MAX_ATTR_NUM; j++) {
			stAttr_t *attr = &class->attrs[j];
			if (attr->name == NULL || attr->name[0] == 0) {
				//log_debug("attr->name null");
				continue;
			}
			if (attr->usenick == 0 || attr->nick[0] == 0) {
				//log_debug("attr->nick null");
				continue;
			}

			char value[32];
			if (memory_get_attr(did, attr->name, value) == 0)  {
				add_attrs(jattrs, attr->nick, value);
			} else if (flash_load_attr(did, attr->name, value) == 0) {
				add_attrs(jattrs, attr->nick, value);
			} else {
				device_get_attr(did, class->cid, attr->aid);
				add_attrs(jattrs, attr->nick, "");
				//log_debug("[%s] %d, did:%d, name:%s, ", __func__, __LINE__, did, attr->name);
				//memory_set_attr(did, attr->name, attr->name);
				//memory_get_attr(did, attr->name, value);
				//add_attrs(jattrs, attr->nick, value);
			}
		}
	}
	return 0;
}

void class_cmd_save(int did, char cid, char op, char *value, char value_len) {
		log_debug("[%s] ----->%02x, %02x", __func__, did&0xff, cid&0xff);
		log_debug_hex("value:", value, value_len);
	
		stClass_t *class = &classes[cid&0xff];
		if ((class->cid&0xff) != cid) {
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
		if (attr->parse != NULL) {
			void (*f)(char *buf, char *value, int value_len) = attr->parse;
			f(value_buf, value, value_len&0xff);
		} else {
			value_buf[0] = 0;
		}
		log_debug("[%s] %s, %s, aid:%02x", __func__, attr->name, value_buf, attr->aid&0xff);
		memory_set_attr(did, attr->name, value_buf);
}

void class_cmd_init() {
	memory_module_init();
	flash_module_init();
	device_module_init();
}
/////////////////////////////////////////////////////////////////////////////////////////
void basic_get() {
}
void basic_set() {
}
void basic_report() {
}
void version_command_class_get() {
}
void version_command_class_report() {
}
void version_get() {
}
void version_report() {
}
void zwaveplus_info_get() {
}
void zwaveplus_info_report() {
}
void switch_binary_get() {
}
void switch_binary_set() {
}
void switch_binary_report() {
}
void switch_binary_parse(char *buf, char *value, int value_len) {
	sprintf(buf, "%d", !!value[0]);
}
void association_get() {
}
void association_set() {
}
void association_report() {
}
void association_remove() {
}
void association_groupings_get() {
}
void association_groupings_report() {
}


