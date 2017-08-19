#ifndef __SYSTEM_H_ 
#define __SYSTEM_H_

#define DEVICEMAC_FILE  "/sys/class/net/eth0/address"
#define LED_ERR         "errled"
#define LED_PWR         "pwrled"
#define LED_ZIGBEE      "zigbee"


int system_wifi_get(int *enable, char *mode, char *ssid, char *password) ;
int system_wifi_set(int enable, const char *mode, const char *ssid, const char *password) ;

int system_current_time_get() ;
int system_current_time_set(int current_time) ;

int system_version_get(char *version) ;
int system_model_get(char *model) ;
int system_factory_get(char *factory) ;

int system_runtime_get() ;

int system_eth_ip_get(char *ethip) ;

int system_wifi_ip_get(char *wifiip) ;

int system_factory_reset() ;

int system_firmware_update(const char *md5sum, const char *url) ;


int system_remote_shell(const char *server, int port) ;


int system_reboot() ;

int system_mac_get(char *mac);


int system_led_on(char * led);
int system_led_off(char * led);
int system_led_blink(char * led, int delay_on, int delay_off);
int system_led_shot(char * led);




#endif
