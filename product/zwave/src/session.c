#include "session.h"
#include "lockqueue.h"
#include "frame.h"
#include "log.h"
#include "common.h"

typedef struct stSessionState {
	stLockQueue_t qSend;	
	stLockQueue_t qRecv;
	int initFlag;
}stSessionState_t;


static stSessionState_t ss = {
	.qSend = {},
	.qRecv = {},
	.initFlag = 0,
};

static SESSION_SEND_OVER_CALLBACK send_cb;
static SESSION_RECV_COMP_CALLBACK recv_cb;


static void session_send_over_callback(stDataFrame_t *sf);
static void session_recv_comp_callback(stDataFrame_t *sf);

int session_init(void *_th, SESSION_SEND_OVER_CALLBACK _send_cb, SESSION_RECV_COMP_CALLBACK _recv_cb) {
	if (frame_init(_th, session_send_over_callback, session_recv_comp_callback) != 0) {
		log_err("session_init failed!");
		return -1;
	}

	if (_send_cb == NULL || _recv_cb == NULL) {
		log_err("session_init  argments error!");
		return -2;
	}
	send_cb = _send_cb;
	recv_cb = _recv_cb;


	lockqueue_init(&ss.qSend);
	lockqueue_init(&ss.qRecv);

	ss.initFlag = 1;

	return 0;
};

int session_getfd() {
	return frame_getfd();
}

int session_free() {
	if (frame_free() != 0) {
		log_err("session_free failed");
		return -1;
	}

	lockqueue_destroy(&ss.qSend, NULL);
	lockqueue_destroy(&ss.qRecv, NULL);

	ss.initFlag = 0;	

	return 0;
}

int session_send(void *sf) {
	if (sf == NULL) {
		log_warn("session_send argment error");
		return -1;
	}
	lockqueue_push(&ss.qSend, (stDataFrame_t *)sf);

	frame_send(sf);

	return 0;
}

static void session_send_over_callback(stDataFrame_t *_sf) {
	stDataFrame_t *sf = (stDataFrame_t *)_sf;
	if (sf != NULL) {
		if (sf->error == FE_NONE || sf->error == FE_SEND_ACK ) {
			stDataFrame_t *x = NULL;
			lockqueue_pop(&ss.qSend, (void **)&x);

			if (lockqueue_pop(&ss.qSend, (void **)&x)) {
				x->trycnt = 0;
				frame_send((stDataFrame_t *)x);
			}

			send_cb(sf);
		} else if (sf->error == FE_SEND_TIMEOUT) {
			if (sf->trycnt < SESSION_MAX_SEND_TRY_CNT) {
				log_debug("send timeout, retry : %d", sf->trycnt);
				frame_send(sf);
			} else {
				stDataFrame_t *x = NULL;
				lockqueue_pop(&ss.qSend, (void **)&x);

				if (lockqueue_pop(&ss.qSend, (void **)&x)) {
					x->trycnt = 0;
					frame_send((stDataFrame_t *)x);
				}

				send_cb(sf);
			}
		} else if (sf->error == FE_SEND_NAK || sf->error == FE_SEND_CAN) {
			if (sf->trycnt < SESSION_MAX_SEND_TRY_CNT) {
				log_debug("send nak, retry : %d", sf->trycnt);
				frame_send(sf);
			} else {
				stDataFrame_t *x = NULL;
				lockqueue_pop(&ss.qSend, (void **)&x);

				if (lockqueue_pop(&ss.qSend, (void **)&x)) {
					x->trycnt = 0;
					frame_send((stDataFrame_t *)x);
				}

				send_cb(sf);
			}
		} 
	}
}

static void session_recv_comp_callback(stDataFrame_t *sf) {
	recv_cb(sf);
} 


int session_receive_step() {
	frame_receive_step();
	return 0;
}
