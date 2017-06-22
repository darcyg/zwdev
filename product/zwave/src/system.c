#include "system.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int system_wifi_get(int *enable, char *mode, char *ssid, char *password) {
	log_debug("[%s] not implement system api", __func__);
	return 0;
}
int system_wifi_set(int enable, const char *mode, const char *ssid, const char *password) {
	log_debug("[%s] not implement system api", __func__);
	return 0;
}

int system_current_time_get() {
	return time(NULL);
}
int system_current_time_set(int current_time) {
	log_debug("[%s] not implement system api", __func__);
	return 0;
}

int system_version_get(char *version) {
	strcpy(version, "zwave_v1.0.0_beta1");
	return 0;
}
int system_model_get(char *model) {
	strcpy(model, "DSI0078");
	return 0;
}
int system_factory_get(char *factory) {
	strcpy(factory, "dusun");
	return 0;
}

int system_runtime_get() {
	FILE *fp  = popen("cat /proc/uptime | cut -d " " -f 1", "r");
	if (fp == NULL) {
		return 0;
	}

	char buf[128];
  char *_yum = fgets(buf, sizeof(buf), fp);
	_yum = _yum;
	fclose(fp);
	buf[strlen(buf)] = 0;

	int runtime;
	if (sscanf(buf, "%d", &runtime) != 1) {
		return 0;
	}

	return runtime;
}

int system_eth_ip_get(char *ethip) {
	//FILE *fp  = popen("ifconfig ra0 | grep \"inet addr\" | xargs | cut -d \" \" -f 2 | cut -d \":\" -f 2", "r");
	FILE *fp  = popen("ifconfig eth0 | grep \"inet addr\" | xargs | cut -d \" \" -f 2 | cut -d \":\" -f 2", "r");
	if (fp == NULL) {
		strcpy(ethip, "");
		return -1;
	}

	char buf[128];
  char *_yum = fgets(buf, sizeof(buf), fp);
	_yum = _yum;
	fclose(fp);
	int len = strlen(buf);
	if (buf[len - 1] == '\n' || buf[len - 1] == '\r') {
		buf[len - 1] = 0;
	}

	strcpy(ethip, buf);
	
	return 0;
}

int system_wifi_ip_get(char *wifiip) {
	//FILE *fp  = popen("ifconfig eth0.2 | grep \"inet addr\" | xargs | cut -d \" \" -f 2 | cut -d \":\" -f 2", "r");
	FILE *fp  = popen("ifconfig eth0 | grep \"inet addr\" | xargs | cut -d \" \" -f 2 | cut -d \":\" -f 2", "r");
	if (fp == NULL) {
		strcpy(wifiip, "");
		return -1;
	}

	char buf[128];
	int size = fread(buf, 1, sizeof(buf), fp);
	buf[size] = 0;

	strcpy(wifiip, buf);
	
	return 0;
}

int system_factory_reset() {
	int ret = system("touch /tmp/do_reset.sh");	
	ret = ret;
	return 0;
}

int system_firmware_update(const char *md5sum, const char *url) {
	int ret = system("touch /tmp/do_upgrade.sh");	
	ret = ret;
	return 0;
}


int system_remote_shell(const char *server, int port) {
	const char *pass = "dusun123456";
	const char *user = "root";
	char cmd[256];
	sprintf(cmd, "remote -p %s ssh -y -g -N -R %s:%d:127.0.0.1:22 %s@%s > /dev/null &", pass, server, port, user, server);
	int ret = system(cmd);
	ret = ret;
	return 0;
}


int system_reboot() {
	int ret = system("touch /tmp/do_reboot.sh");
	ret = ret;
	return 0;
}

int system_mac_get(char *mac) {
  FILE * fp = fopen("/sys/class/net/eth0/address", "r");
  if (fp == NULL) {
    return -1;
  }
	char *_yum = fgets(mac, 20, fp);
	_yum = _yum;
  fclose(fp);

  int len = strlen(mac);
  if (len < 17) {
    return -1;
  }

  if (mac[len-1] == '\n') {
    mac[len-1] = '\0';
    len = len - 1;
  }

  return 0;
}


/* 
 * led stuff 
 */
static int  write_led_attribute(char * led, char * att, char * value) {
    char path[100];
    char str[20];
    sprintf(path, "/sys/class/leds/%s/%s", led, att);
    sprintf(str, "%s\n", value);
    FILE * fp = fopen(path, "w");
    if (!fp)
        return -1;

    fputs(str, fp);
    fclose(fp);
    return 0;
}

int system_led_on(char * led)
{
  write_led_attribute(led, "trigger", "none");
  write_led_attribute(led, "brightness", "1");
	return 0;
}

int system_led_off(char * led)
{
  write_led_attribute(led, "trigger", "none");
  write_led_attribute(led, "brightness", "0");
	return 0;
}

int system_led_blink(char * led, int delay_on, int delay_off)
{
  char delay_on_str[16];
  char delay_off_str[16];

  sprintf(delay_on_str, "%d", delay_on);
  sprintf(delay_off_str, "%d", delay_off);

  write_led_attribute(led, "trigger", "timer");
  write_led_attribute(led, "delay_on", delay_on_str);
  write_led_attribute(led, "delay_off", delay_off_str);
	return 0;
}

int system_led_shot(char * led)
{
  write_led_attribute(led, "trigger", "oneshot");
  write_led_attribute(led, "shot", "1");
	return 0;
}
