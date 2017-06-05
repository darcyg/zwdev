#ifndef _UPROTO_H_
#define _UPROTO_H_

#include "timer.h"
#include "file_event.h"
#include "lockqueue.h"

#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubus.h>
#include <jansson.h>

enum {
	UE_SEND_MSG = 0x00,
};

typedef struct stUprotoEnv {
	struct file_event_table *fet;
	struct timer_head *th;
	struct timer step_timer;
	stLockQueue_t msgq;

  struct ubus_context *ubus_ctx;
  struct ubus_event_handler listener;
}stUprotoEnv_t;

typedef int (*UPROTO_HANDLER)(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
typedef struct stUprotoCmd {
	const char *name;
	UPROTO_HANDLER handler;
}stUprotoCmd_t;

typedef int (*UPROTO_CMD_GET)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef int (*UPROTO_CMD_SET)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef struct stUprotoAttrCmd {
	const char *name;
	UPROTO_CMD_GET get;
	UPROTO_CMD_SET set;
}stUprotoAttrCmd_t;


int uproto_init(void *_th, void *_fet);
int uproto_step();
int uproto_push_msg(int eid, void *param, int len);
void uproto_run(struct timer *timer);
void uproto_in(void *arg, int fd);


#endif
