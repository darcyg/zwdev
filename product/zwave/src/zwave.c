#include "jansson.h"
#include "log.h"
#include "app.h"
#include "classcmd.h"
#include "json_parser.h"


json_t *zwave_device_list() {
	log_debug("[%s]", __func__);
	stAppEnv_t *ae = app_util_getae();
	
	json_t *jdevs = json_array();

	int i = 0;
	for (i = 0; i < sizeof(ae->devs)/sizeof(ae->devs[0]); i++) {
		stDevice_t *dev = &ae->devs[i];
		if (dev->id == 0) {
			continue;
		}
		json_t *jdev = json_object();
		json_object_set_new(jdev,	"mac",			json_integer(dev->id));
		json_object_set_new(jdev,	"type",			json_string(specific2str(dev->generic, dev->specific)));
		//json_object_set_new(jdev,	"version",	json_string(json_get_string(jattrs, "version")));
		json_object_set_new(jdev,	"model",		json_string(generic2str(dev->generic)));
		json_object_set_new(jdev,	"online",		json_integer(dev->online));

		log_debug("dev %d", dev->id);
		log_debug_hex("class:", dev->class, dev->clen);
		json_t *jattrs = json_object();
		if (jattrs != NULL) {
			emClass_t class[32];
			int j;
			for (j = 0; j < dev->clen; j++) {
				class[j] = dev->class[j];
			}
			class_cmd_to_attrs(dev->id, class, dev->clen, jattrs);
			
			const char *version = json_get_string(jattrs, "version");
			if (version != NULL) {
				json_object_set_new(jdev,	"version",	json_string(version));
				json_object_del(jattrs, "version");
			}

			const char *battery = json_get_string(jattrs, "battery");
			if (battery != NULL) {
				int battery_value;
				sscanf(battery, "%d", &battery_value);
				json_object_set_new(jdev,	"battery",	json_integer(battery_value));
			} else {
				json_object_set_new(jdev,	"battery",	json_integer(100));
			}
	
			json_object_set_new(jdev, "attrs", jattrs);
		}

		json_array_append_new(jdevs, jdev);
	}

	return jdevs;
}

int zwave_find_device() {
	log_debug("[%s]", __func__);
	return 0;
}

int zwave_device_light_onoff(const char *mac, int val) {
	log_debug("[%s] -> %d", __func__, val);

	int did = 0;
	sscanf(mac, "%d", &did);
	
	int cid = COMMAND_CLASS_SWITCH_BINARY_V1;
	int aid = SWITCH_BINARY;

	char value[32];
	sprintf(value, "0x%02x", val&0xff);
	char *argv[] = {value};

	int argc = sizeof(argv)/sizeof(argv[0]);
	app_util_push_cmd(E_COMMAND, NULL, 0);
	class_cmd_set_attr(did, cid, aid, argv, argc);

	return 0;
}
int zwave_device_light_toggle(const char *mac) {
	log_debug("[%s]", __func__);
	return 0;
}
int zwave_device_light_brightness(const char *mac, int val) {
	log_debug("[%s] -> %d", __func__, val);
	return 0;
}

