#ifndef __ZWAVE_IFACE_H_
#define __ZWAVE_IFACE_H_

#include <jansson.h>
#include "zwave_device.h"

json_t *	zwave_iface_list();
int				zwave_iface_include();
int				zwave_iface_exclude(const char *mac);
json_t *	zwave_iface_info();

int zwave_iface_test();
int zwave_iface_viewall();

int zwave_iface_report_register(const char *mac);
int zwave_iface_report_unregister(const char *mac);
int zwave_iface_report_cmd(const char *mac, char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len);

int zwave_iface_device_switch_onoff(const char *mac,char ep, int onoff);

int zwave_iface_report_status(const char *mac);
#endif
