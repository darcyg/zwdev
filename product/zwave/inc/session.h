#ifndef _SESSION_H_
#define _SESSION_H_



typedef void (*SESSION_SEND_OVER_CALLBACK)(void *sf);
typedef void (*SESSION_RECV_COMP_CALLBACK)(void *sf);

int session_init(void *_th, SESSION_SEND_OVER_CALLBACK send_cb, SESSION_RECV_COMP_CALLBACK recv_cb);

int session_send(void *sf);

int session_getfd();

int session_free();

int session_receive_step();




#endif
