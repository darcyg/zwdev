#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#define MAX_STATE_NUM 15
#define MAX_EVENT_NUM 15


typedef struct stEvent {
	int eid;
	void *param;
}stEvent_t;

typedef struct stEventHandler {
	int eid;
	void *action;
	void *transition;
}stEventHandler_t;

typedef struct stState {
	int numevent;
	void *param;
	stEventHandler_t eventhandlers[MAX_EVENT_NUM];
}stState_t;

typedef struct stStateMachine {
	int numstate;
	int state;
	int initstate;
	stState_t states[MAX_STATE_NUM];
}stStateMachine_t;

typedef void * (*ACTIOIN)(stStateMachine_t *, stEvent_t *);
typedef int	(*TRANSITION)(stStateMachine_t *, stEvent_t *, void *acret);

int state_machine_init(stStateMachine_t *sm);
int state_machine_reset(stStateMachine_t *sm);
int state_machine_get_state(stStateMachine_t *sm);
int state_machine_set_state(stStateMachine_t *sm, int state);
int state_machine_step(stStateMachine_t *sm, stEvent_t *event);
int state_machine_free(stStateMachine_t *sm);


/* state must start at zero */
#endif
