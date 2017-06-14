#include "jansson.h"
#include "log.h"
#include "app.h"
#include "classcmd.h"
#include "json_parser.h"


json_t *zwave_device_list() {
	log_debug("[%s]", __func__);
	json_t *jdevs = app_zlist();
	return jdevs;
}

int zwave_find_device() {
	log_debug("[%s]", __func__);
	app_zinclude();
	return 0;
}

int zwave_del_device(const char *mac) {
	log_debug("[%s]", __func__);
	app_zexclude_by_mac(mac);
}

int zwave_device_light_onoff(const char *mac, int val) {
	log_debug("[%s] -> %d", __func__, val);

	char value[32];
	sprintf(value, "%d", val);
	app_zclass_cmd_set_by_mac(mac, "on_off", value);

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


json_t * zwave_device_rpt(const char *submac, const char *attr, const char *value) {
	if (strcmp(attr, "on_off") == 0) {
		json_t *jval = json_object();
		if (jval != NULL) {
			int x = 0;
			sscanf(value, "%d", &x);
			json_object_set_new(jval, "name", json_string(attr));
			json_object_set_new(jval, "value", json_integer(x));
			return jval;
		}
	}
	return NULL;
}


