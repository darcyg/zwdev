#ifndef _APP_H_
#define _APP_H_

#include "statemachine.h"
#include "timer.h"
#include "file_event.h"
#include "lockqueue.h"
#include "api.h"

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
}stDevice_t;

typedef struct stAppEnv {
	struct file_event_table *fet;
	struct timer_head *th;

	struct timer step_timer;
	stLockQueue_t eq;
	stLockQueue_t cmdq;
	stLockQueue_t msgq;



	int dev_num;
	stDevice_t devs[MAX_DEVICE_NUM];

	stVersion_t ver;
	stInitData_t initdata;
	stCapabilities_t caps;
	stId_t id;
	stSucNodeId_t sucid;

	int lastid;
}stAppEnv_t;


typedef struct stAppCmd {
	int len;
	void *param;
}stAppCmd_t;


enum {
	S_IDLEING = 0,

	S_INITTING = 1,
	

	S_CLASSING = 2,

	S_INCLUDING = 3,
	
	S_EXCLUDING = 4,
	
	S_COMMANDING = 5,
};

enum {
	E_INIT = 0,
	E_CLASS = 1,
	E_INCLUDE = 2,
	E_EXCLUDE = 3,
	E_COMMAND = 4,

	E_INIT_OVER = 5,

	E_CLASS_STEP = 6,

	E_INCLUDE_OVER = 7,
	E_EXCLUDE_OVER = 8,
	E_COMMAND_OVER = 9,
};

int app_init(void *_th, void *_fet);
int app_step();
int app_push(stEvent_t *e);
void app_run(struct timer *timer);
void app_in(void *arg, int fd);
void app_util_push_cmd(int eid, void *param, int len);
void app_util_push_msg(int eid, void *param, int len);
stAppEnv_t* app_util_getae();


#endif
