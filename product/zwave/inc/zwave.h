#ifndef __ZWAVE_H_
#define __ZWAVE_H_

#include "frame.h"
#include "zwave_api.h"

int zwave_init(void *_th, void *_fet, const char *dev, int buad);

typedef int (*ZWAVE_IFACE_FUNC)(stEvent_t *e);

enum {
	E_UNUSED = 0,
};
int zwave_push(int eid, void *param, int len);

int zwave_test();

typedef struct stZWaveEnv {
	struct timer_head *th;
	struct file_event_table *fet;

	struct timer tr;
	struct timer tr_online;
	struct timer tr_query;

	stLockQueue_t eq;

	stInventory_t inventory;

}stZWaveEnv_t;

//int zwave_list(); move to zwave_iface
int zwave_include();
int zwave_exclude(char mac[8]);
//int zwave_info(); move to zwave_iface
int zwave_switch_onoff(char mac[8], char ep, int onoff);
int zwave_remove_failed_node(const char *mac);

stInventory_t* zwave_get_inventory();

#endif
