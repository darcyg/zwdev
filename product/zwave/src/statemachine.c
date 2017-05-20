#include "common.h"
#include "statemachine.h"
#include "log.h"


int state_machine_init(stStateMachine_t *sm) {
	state_machine_reset(sm);
	return 0;
}
int state_machine_reset(stStateMachine_t *sm) {
	sm->state = sm->initstate;

	int i = 0;
	for (i = 0; i < sm->numstate; i++) {
		sm->states[i].param = NULL;
	}
	return 0;
}
int state_machine_get_state(stStateMachine_t *sm) {
	return sm->state;
}

int state_machine_set_state(stStateMachine_t *sm, int state) {
	sm->state = state;
	return 0;
}
int state_machine_step(stStateMachine_t *sm, stEvent_t *event) {
	int i = 0, j = 0;
	int state = state_machine_get_state(sm);

	void *action = NULL;
	void *transition = NULL;
	void *acret = NULL;
	for (j = 0; j < sm->numstate; j++) {
		if (state != sm->states[j].stateid) {
			continue;
		}
		for (i = 0; i < sm->states[j].numevent; i++) {
			if (sm->states[j].eventhandlers[i].eid == event->eid) {
				action = sm->states[j].eventhandlers[i].action;
				transition = sm->states[j].eventhandlers[i].transition;
				break;
			}
		}
	}
	if (action != NULL) {
		acret = ((ACTIOIN)action)(sm, event);
	} else {
		;
	}

	int next_state;
	if (transition != NULL) {
		next_state = ((TRANSITION)transition)(sm, event, acret);
	} else {
		next_state = state;
	}

	if (next_state != state) {
		log_info("state change from %d to %d", state, next_state);
	}

	//if (acret != NULL) {
	//	FREE(acret);
	//}

	state_machine_set_state(sm, next_state);

	return 0;
}
int state_machine_free(stStateMachine_t *sm) {
	state_machine_reset(sm);
	return 0;
}

stState_t *state_machine_search_state(stStateMachine_t *sm, int sid) {
	int i = 0;
	for (i = 0; i < sm->numstate; i++) {
		if (sid == sm->states[i].stateid) {
			return &sm->states[i];
		}
	}

	return NULL;
}

