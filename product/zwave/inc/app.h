#ifndef _APP_H_
#define _APP_H_

#include "statemachine.h"
#include "timer.h"
#include "file_event.h"
#include "lockqueue.h"
#include "api.h"
#include "hashmap.h"
#include "jansson.h"

#define MAC_MAX_LEN 32
#define MAX_CLASS_NUM 32
#define MAX_DEVICE_NUM 234

typedef struct stDevice {
	int id;
	char mac[MAC_MAX_LEN];
	char basic;
	char generic;
	char specific;
	int  clen;
	char class[MAX_CLASS_NUM];

	int fnew;
	int online;
	long lasttime;
}stDevice_t;

typedef struct stInventory {
	stVersion_t ver;
	stInitData_t initdata;
	stCapabilities_t caps;
	stId_t id;
	stSucNodeId_t sucid;

	int dev_num;
	stDevice_t devs[MAX_DEVICE_NUM];

	struct hashmap hmattrs;
}stInventory_t;

typedef struct stAppEnv {
	struct file_event_table *fet;
	struct timer_head *th;

	struct timer step_timer;
	stLockQueue_t eq;
	stLockQueue_t eqMsg;
	struct timer online_timer;

	stInventory_t inventory;
}stAppEnv_t;

enum {
	aS_IDLEING = 0,

	aS_WORKING = 1
};

enum {
	aE_INIT = 0,
	aE_CLASS = 1,
	aE_ATTR = 2,
	aE_INCLUDE = 3,
	aE_EXCLUDE = 4,
	aE_GET = 5,
	aE_SET = 6,
	aE_ONLINE_CHECK = 7,
	aE_ATTR_NEW = 8,
	aE_FRESH_NODEMAP = 9,

	aE_OVER = 10,
};

typedef struct stAppCmd {
	void* param;
	int len;
}stAppCmd_t;

int		app_init(void *_th, void *_fet);
void	app_push(int eid, void *param, int len);
void	app_push_msg(int eid, void *param, int len);
int		app_step();
void	app_run(struct timer *timer);
void	app_in(void *arg, int fd);

void	app_online_check(struct timer *timer);

typedef struct stGetParam {
	int did;
	char *attr;
	char *value;
}stGetParam_t;
typedef struct stSetParam {
	int  did;
	char *attr;
	char *value;
}stSetParam_t;

int				app_zinit();
int				app_zclass();
int				app_zattr();
int				app_zattr_new();
json_t *	app_zlist();
int				app_zinclude();
int				app_zexclude(int did);
int				app_zexclude_by_mac(const char *mac);
int				app_zclass_cmd_get(int did, char *attr, char *value);
int				app_zclass_cmd_set(int did, char *attr, char *value);
int				app_zclass_cmd_get_by_mac(const char *mac, char *attr, char *value);
int				app_zclass_cmd_set_by_mac(const char *mac, char *attr, char *value);
int				app_zclass_cmd_rpt(int did, int cid, int aid, char *value, int value_lep);
int				app_zfresh_nodemap();
json_t *	app_zinfo();

stInventory_t *app_get_inventory();

#endif
