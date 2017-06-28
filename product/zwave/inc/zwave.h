#ifndef __ZWAVE_H_
#define __ZWAVE_H_

#include "frame.h"
#include "zwave_api.h"

int zwave_init(void *_th, void *_fet, const char *dev, int buad);

typedef int (*ZWAVE_IFACE_FUNC)(stEvent_t *e);

enum {
	E_ZWAVE_LIST = 0,
	E_ZWAVE_INCLUDE = 1,
	E_ZWAVE_EXCLUDE = 2,
	E_ZWAVE_GET = 3,
	E_ZWAVE_SET = 4,
	E_ZWAVE_INFO = 5,
	E_ZWAVE_LIGHT_ONOFF = 6,
	E_ZWAVE_LIGHT_TOGGLE = 7,
	E_ZWAVE_LIGHT_BRIGHTNESS = 8,
};
int zwave_push(int eid, void *param, int len);

int zwave_test();

typedef struct stZWaveEnv {
	stInventory_t inventory;

	struct timer_head th;
	struct timer tr;
	struct timer tr_online;
	struct timer tr_query;

	char				 sim_mac[32];
	int					 sim_pir;
	struct timer sim_tr_pir;

	struct file_event_table fet;

	stLockQueue_t eq;
	bool run_flag;
	int pipe[2];
}stZWaveEnv_t;
#endif
