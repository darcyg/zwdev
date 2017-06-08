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
	return 0;
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

