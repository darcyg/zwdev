#ifndef __ZWAVE_IFACE_H_
#define __ZWAVE_IFACE_H_

#include <jansson.h>

json_t *	zwave_iface_list();
int				zwave_iface_include();
int				zwave_iface_exclude(const char *mac);
json_t *	zwave_iface_info();

int zwave_iface_device_light_onoff(const char *mac, int onoff);

int zwave_iface_init();
int zwave_iface_wait();
int zwave_iface_push(json_t *elem);


int zwave_iface_report(json_t *rpt);

int zwave_iface_test();

#endif
