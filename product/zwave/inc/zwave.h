#ifndef __ZWAVE_H_
#define __ZWAVE_H_


json_t *zwave_device_list();
int zwave_find_device();
int zwave_del_device(const char *mac);

int zwave_device_light_onoff(const char *mac, int val);
int zwave_device_light_toggle(const char *mac);
int zwave_device_light_brightness(const char *mac, int val);


json_t * zwave_device_rpt(const char *submac, const char *attr, const char *value);
#endif
