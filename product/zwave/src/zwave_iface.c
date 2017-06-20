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
#include "lockqueue.h"

static int zwave_iface_pipe[2];
static stLockQueue_t zilq;
static stLockQueue_t zilq_rpt;
static struct file_event_table *zwave_iface_fet = NULL;
static struct timer_head *zwave_iface_th = NULL;


void zwave_iface_in(void *arg, int fd) {
	char ch;
	int ret = read(zwave_iface_pipe[0], &ch, 1);
	ret = ret;
	
	json_t *jrpt = NULL;
	while (lockqueue_pop(&zilq_rpt,  (void **)&jrpt) && jrpt != NULL) {
		const char *mac = json_get_string(jrpt, "mac");
		const char *attr = json_get_string(jrpt, "attr");

		char smac[32];
		char sattr[32];
		strcpy(smac, mac);
		strcpy(sattr, attr);
		json_object_del(jrpt, "mac");
		json_object_del(jrpt, "attr");

		uproto_report_umsg(smac, sattr, jrpt);
	}
}

int zwave_iface_init(void *_th, void *_fet) {
	lockqueue_init(&zilq);

	lockqueue_init(&zilq_rpt);
	zwave_iface_fet = (struct file_event_table *)_fet;
	zwave_iface_th = (struct timer_head*)_th;
	int ret = pipe(zwave_iface_pipe);
	ret = ret;
	file_event_reg(zwave_iface_fet, zwave_iface_pipe[0], zwave_iface_in, NULL, NULL);
	return 0;
}

int zwave_iface_wait() {
	lockqueue_wait(&zilq);
	return 0;
}

int zwave_iface_push(json_t *elem) {
	lockqueue_push(&zilq, elem);
	lockqueue_wake(&zilq);
	return 0;
}


json_t *	zwave_iface_list() {
	log_info("[%s] %d", __func__, __LINE__);

	zwave_push(E_ZWAVE_LIST, NULL, 0);

	zwave_iface_wait();

	json_t *jdevs = NULL;
	lockqueue_pop(&zilq, (void **)&jdevs);
	if (jdevs != NULL) {
		return jdevs;
	}

	return NULL;
}
int				zwave_iface_include() {
	log_info("[%s] %d", __func__, __LINE__);
	zwave_push(E_ZWAVE_INCLUDE, NULL, 0);

	zwave_iface_wait();
	json_t *jret;
	lockqueue_pop(&zilq, (void **)&jret);
	if (jret == NULL) {
		return -1;
	}

	int ret; json_get_int(jret, "ret", &ret);
	if (ret != 0) {
		json_decref(jret);
		return -2;
	}

	json_decref(jret);

	return 0;
}
int				zwave_iface_exclude(const char *mac) {
	log_info("[%s] %d, mac:%s", __func__, __LINE__, mac);
	
	json_t *jarg = json_object();
	json_object_set_new(jarg, "mac", json_string(mac));

	zwave_push(E_ZWAVE_EXCLUDE, jarg, sizeof(jarg));

	zwave_iface_wait();
	json_t *jret;
	lockqueue_pop(&zilq, (void **)&jret);
	if (jret == NULL) {
		return -1;
	}

	int ret; json_get_int(jret, "ret", &ret);
	if (ret != 0) {
		json_decref(jret);
		return -2;
	}

	json_decref(jret);

	return 0;
}
int				zwave_iface_get(const char *mac, const char *attr, const char *arg) {
	log_info("[%s] %d, mac:%s, attr:%s, arg:%s", __func__, __LINE__, mac, attr, arg);

#if 0
	json_t *jarg = json_object();
	json_object_set_new(jarg, "attr", json_string(attr));
	json_object_set_new(jarg, "arg", json_string(arg));
	zwave_push(E_ZWAVE_GET, jarg, sizeof(jarg));
#else
	log_warn("now not support get");
#endif
	return 0;
}
int				zwave_iface_set(const char *mac, const char *attr, const char *arg) {
	log_info("[%s] %d, mac:%s, attr:%s, arg:%s", __func__, __LINE__, mac, attr, arg);

#if 0
	json_t *jarg = json_object();
	json_object_set_new(jarg, "attr", json_string(attr));
	json_object_set_new(jarg, "arg", json_string(arg));
	zwave_push(E_ZWAVE_SET, jarg, sizeof(jarg));
#else
	log_warn("now not support get");
#endif

	return 0;
}
json_t *	zwave_iface_info() {
	log_info("[%s] %d", __func__, __LINE__);
	zwave_push(E_ZWAVE_INFO, NULL, 0);

	zwave_iface_wait();
	json_t *jinfo;	
	lockqueue_pop(&zilq, (void **)&jinfo);
	if (jinfo != NULL) {
		return jinfo;
	}

	return NULL;
}


int zwave_iface_device_light_onoff(const char *mac, int onoff) {
	log_info("[%s] %d", __func__, __LINE__);

	json_t *jarg = json_object();
	json_object_set_new(jarg, "mac", json_string(mac));
	json_object_set_new(jarg, "onoff", json_integer(onoff));
	zwave_push(E_ZWAVE_LIGHT_ONOFF, jarg, sizeof(jarg));

	zwave_iface_wait();
	json_t *jret;
	lockqueue_pop(&zilq, (void **)&jret);
	if (jret == NULL) {
		return -1;
	}

	int ret; json_get_int(jret, "ret", &ret);
	if (ret != 0) {
		json_decref(jret);
		return -2;
	}

	json_decref(jret);

	return 0;
}

int zwave_iface_device_light_toggle(const char *mac) {
	log_info("[%s] %d", __func__, __LINE__);

#if 0
	json_t *jarg = json_object();
	json_object_set_new(jarg, "mac", json_string(mac));
	zwave_push(E_ZWAVE_LIGHT_TOGGLE, jarg, sizeof(jarg));
#else
	log_warn("now not support get");
#endif

	return 0;
}

int zwave_iface_device_light_brightness(const char *mac, int brightness) {
	log_info("[%s] %d", __func__, __LINE__);

#if 0
	json_t *jarg = json_object();
	json_object_set_new(jarg, "mac", json_string(mac));
	json_object_set_new(jarg, "brightness", json_integer(brightness));
	zwave_push(E_ZWAVE_LIGHT_BRIGHTNESS, jarg, sizeof(jarg));
#else
	log_warn("now not support get");
#endif

	return 0;
}

int zwave_iface_report(json_t *rpt) {
	lockqueue_push(&zilq_rpt, (void *)rpt);
	int ret = write(zwave_iface_pipe[1], "A", 1);
	return ret;
}

