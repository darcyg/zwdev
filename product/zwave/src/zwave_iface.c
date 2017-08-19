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

static int zwave_iface_pipe[2];
static stLockQueue_t zilq;
static stLockQueue_t zilq_rpt;
static struct file_event_table *zwave_iface_fet = NULL;
static struct timer_head *zwave_iface_th = NULL;


json_t *	zwave_iface_list() {
	log_info("[%s] %d", __func__, __LINE__);

	return NULL;
}
int				zwave_iface_include() {
	log_info("[%s] %d", __func__, __LINE__);

	return 0;
}
int				zwave_iface_exclude(const char *mac) {
	log_info("[%s] %d, mac:%s", __func__, __LINE__, mac);
	
	return 0;
}
int				zwave_iface_get(const char *mac, const char *attr, const char *arg) {
	log_info("[%s] %d, mac:%s, attr:%s, arg:%s", __func__, __LINE__, mac, attr, arg);

	return 0;
}
int				zwave_iface_set(const char *mac, const char *attr, const char *arg) {
	log_info("[%s] %d, mac:%s, attr:%s, arg:%s", __func__, __LINE__, mac, attr, arg);

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

int zwave_iface_device_light_toggle(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);

	return 0;
}

int zwave_iface_device_light_brightness(const char *mac, int brightness) {
	log_info("[%s] %d", __func__, __LINE__);

	return 0;
}

int zwave_iface_report(json_t *rpt) {
	log_info("[%s] %d", __func__, __LINE__);
}


int zwave_iface_report_devcie_list() {
	log_info("[%s] %d", __func__, __LINE__);
	return 0;
}
