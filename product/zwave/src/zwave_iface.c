#include "log.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uproto.h>

#include "common.h"
#include "jansson.h"
#include "json_parser.h"
#include "zwave_iface.h"
#include "zwave.h"
#include "zwave_util.h"
#include "zwave_device.h"

extern stZWaveCache_t zc;

json_t *	zwave_iface_list() {
	log_info("[%s] %d", __func__, __LINE__);

	json_t *jdevs = json_array();
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);
	for (i = 0; i < cnt; i++) {
		stZWaveDevice_t *zd = &zc.devs[i];
		if (zd->used == 0) {
			continue;
		}

		json_t *jdev = json_object();
		json_array_append_new(jdevs, jdev);

		json_object_set_new(jdev, "mac", json_string(device_make_macstr(zd)));
		json_object_set_new(jdev, "battery", json_integer(device_get_battery(zd)));
		json_object_set_new(jdev, "online", json_integer(device_get_online(zd)));
		json_object_set_new(jdev, "model", json_string(device_make_modelstr(zd)));
	}

	return jdevs;
}
int				zwave_iface_include() {
	log_info("[%s] %d", __func__, __LINE__);

	return 0;
}
int				zwave_iface_exclude(const char *mac) {
	log_info("[%s] %d, mac:%s", __func__, __LINE__, mac);
	
	return 0;
}

json_t *	zwave_iface_info() {
	log_info("[%s] %d", __func__, __LINE__);

	return NULL;
}


int zwave_iface_device_light_onoff(const char *mac, int onoff) {
	log_info("[%s] %d", __func__, __LINE__);

	return 0;
}

int zwave_iface_report_devcie_list() {
	log_info("[%s] %d", __func__, __LINE__);
	return 0;
}


int zwave_iface_switch_onoff_rpt(stZWaveDevice_t *zd, char ep, stZWaveClass_t *zcls, stZWaveCommand_t *zcmd) {
	log_info("[%s] %d", __func__, __LINE__);
	return 0;
}



int zwave_iface_test() {
	log_info("[%s] %d", __func__, __LINE__);
	zwave_test();
	return 0;
}

int zwave_iface_viewall() {
	log_info("[%s] %d", __func__, __LINE__);
	device_view_all();
	return 0;
}

