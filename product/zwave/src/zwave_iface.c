#include "log.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uproto.h>

#include "common.h"
#include "log.h"
#include "jansson.h"
#include "json_parser.h"
#include "zwave_iface.h"
#include "zwave_util.h"
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
		json_object_set_new(jdev, "type", json_string(device_make_typestr(zd)));
		json_object_set_new(jdev, "version", json_string(device_make_versionstr(zd)));
	}

	return jdevs;
}
int				zwave_iface_include() {
	log_info("[%s] %d", __func__, __LINE__);

	return zwave_include();
}
int				zwave_iface_exclude(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);
	
	return zwave_exclude((char *)mac);
}

json_t *	zwave_iface_info() {
	log_info("[%s] %d", __func__, __LINE__);
	stInventory_t *inv = zwave_get_inventory();

	json_t *jret = json_object();
	char sbuf[32];
	sprintf(sbuf, "%08X", inv->id.HomeID);
	json_object_set_new(jret, "HomeID", json_string(sbuf));
	sprintf(sbuf, "%02X", inv->id.NodeID);
	json_object_set_new(jret, "NodeID", json_string(sbuf));

	return jret;
}

int				zwave_iface_remove_failed_node(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);
	return  zwave_remove_failed_node((char *)mac);
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

int zwave_iface_report_register(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);
	

	uproto_rpt_register(mac);
	
	return 0;
}
int zwave_iface_report_unregister(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);

	uproto_rpt_unregister(mac);

	return 0;
}

int zwave_iface_report_status(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);

	uproto_rpt_status(mac);

	return 0;
}


int zwave_iface_report_cmd(const char *mac, char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len) {
	log_info("[%s] %d", __func__, __LINE__);
	uproto_rpt_cmd(mac, ep, clsid, cmdid, buf, len);
	return 0;
}

int zwave_iface_device_switch_onoff(const char *mac, char ep, int onoff) {
	log_info("[%s] %d", __func__, __LINE__);

	return zwave_switch_onoff((char *)mac, ep, onoff);

}




