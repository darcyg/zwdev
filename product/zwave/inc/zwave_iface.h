#ifndef __ZWAVE_IFACE_H_
#define __ZWAVE_IFACE_H_

#include <jansson.h>
#include "zwave_device.h"

json_t *	zwave_iface_list();
int				zwave_iface_include();
int				zwave_iface_exclude(const char *mac);
json_t *	zwave_iface_info();

int zwave_iface_device_light_onoff(const char *mac, int onoff);

int zwave_iface_test();

int zwave_iface_viewall();

int zwave_iface_switch_onoff_rpt(stZWaveDevice_t *zd, char ep, stZWaveClass_t *zcls, stZWaveCommand_t *zcmd);


int zwave_iface_report_devcie_list();
#endif
