#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "log.h"
#include "timer.h"
#include "frame.h"
#include "transport.h"

//#define DEV "/dev/pts/3"
//#define DEV "/dev/ttyUSB0"
#define DEV "/dev/ttyACM0"
#define BAUD 115200

typedef struct stFrameState {
  stDataFrame_t *frameSend;
	stDataFrame_t *frameRecv;

  struct timer timerRecv;
  struct timer timerSend;

  int initFlag;

  int stateRecv;
  int stateSend;

	struct timer_head *th;
} stFrameState_t;

static stFrameState_t fs = {
	.frameSend = NULL,
	.frameRecv = NULL,
	.timerRecv = {},
	.timerSend = {},
	.initFlag = 0,
	.stateRecv = FRS_SOF_HUNT,
	.stateSend = FSS_READY,
};

int frame_len(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return df->len;
}

int frame_size(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return df->size;
}

char * frame_payload(stDataFrame_t *df) {
  if (df == NULL) {
    return NULL;
  }
  return df->payload;
}

int frame_timestamp(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return df->timestamp;
}
unsigned char frame_cmd(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return df->cmd;
}
unsigned char frame_type(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return df->type;  
}
unsigned char frame_checksum(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return df->checksum;
}
int frame_payload_full(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  return !!(df->size == df->len - 3);
}
int frame_checksum_valid(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  if (df->do_checksum == 0) {
    frame_calculate_checksum(df);
    df->do_checksum= 1;
  }
	if (df->checksum != df->checksum_cal) {
		log_debug("checksum is %02x,cacl: %02x", df->checksum, df->checksum_cal);
	}
  return (df->checksum_cal == df->checksum);
}

int frame_calculate_checksum(stDataFrame_t *df) {
  unsigned char calcChksum = 0xFF;
  
  calcChksum ^= (unsigned char)((df->len)&0xff); // Length
  calcChksum ^= df->type;     // Type
  calcChksum ^= df->cmd;      // Command

  int i = 0;
  for (i = 0; i < df->size; i++) {
    calcChksum ^= df->payload[i];      // Data
	}

  df->checksum_cal = calcChksum;

  return 0;
}

static FRAME_SEND_OVER_CALLBACK send_over_cb = NULL;
static FRAME_RECV_COMP_CALLBACK recv_over_cb = NULL;

static void frame_receive_timer_callback(struct timer *timer);
static void frame_send_timer_callback(struct timer *timer);
static void frame_ack();

int frame_init(void *_th, FRAME_SEND_OVER_CALLBACK _send_over_cb, 
										 FRAME_RECV_COMP_CALLBACK _recv_over_cb) {
  if (fs.initFlag != 0) {
    log_debug("frame state has been init!");
    return 0;
  }

	if (_send_over_cb == NULL || _recv_over_cb == NULL || _th == NULL) {
		log_debug("argments error!");
		return -1;
	}
	send_over_cb = _send_over_cb;
	recv_over_cb = _recv_over_cb;
	fs.th = (struct timer_head *)_th;

  if (transport_open(DEV, BAUD) != 0) {
    log_debug("transport open failed!");
    return -2;
  }
  
  timer_init(&fs.timerRecv, frame_receive_timer_callback);
  timer_init(&fs.timerSend, frame_send_timer_callback);

	fs.stateRecv = FRS_SOF_HUNT;
	fs.stateSend = FSS_READY;

	fs.initFlag = 1;

	return 0;
}

int frame_getfd() {
  return transport_getfd();
}

int frame_receive_reset() {
  timer_cancel(fs.th, &fs.timerRecv);
  timer_cancel(fs.th, &fs.timerSend);

	fs.stateRecv = FRS_SOF_HUNT;
	fs.stateSend = FSS_READY;

	return 0;
}

int frame_free() {
  timer_cancel(fs.th, &fs.timerRecv);
  timer_cancel(fs.th, &fs.timerSend);

	fs.stateRecv = FRS_SOF_HUNT;
	fs.stateSend = FSS_READY;

  if (transport_close() != 0) {
    log_debug("transport_close failed!");
    return -1;
  }
  
  fs.initFlag = 0;
  return 0;
}


int frame_receive_step() {
	char ch;
	int ret;

	do {
		ret = transport_read(&ch, 1, 4080);
		if (ret != 1) {
			break;
		}
		switch (fs.stateRecv) {
			case FRS_SOF_HUNT:
				if ((ch&0xff) == SOF_CHAR) {
					//log_debug("%d: %02x, %02x", __LINE__, ch, SOF_CHAR);
					fs.stateRecv = FRS_LENGTH;
					timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
				} else if ((ch&0xff) == ACK_CHAR) {
					//log_debug("%d", __LINE__);
					if (fs.frameSend != NULL) {
						fs.frameSend->error = FE_SEND_ACK;
						fs.frameSend->trycnt++;
					}

					timer_cancel(fs.th, &fs.timerSend);
					stDataFrame_t *tmp = fs.frameSend;
					fs.frameSend = NULL;
					fs.stateSend = FSS_READY;

					if (send_over_cb != NULL) {
						send_over_cb(tmp);
					}
				} else if ((0xff&ch) == NAK_CHAR) {
					//log_debug("%d", __LINE__);
					if (fs.frameSend != NULL) {
						fs.frameSend->error = FE_SEND_NAK;
						fs.frameSend->trycnt++;
					}

					timer_cancel(fs.th, &fs.timerSend);
					stDataFrame_t *tmp = fs.frameSend;
					fs.frameSend = NULL;
					fs.stateSend = FSS_READY;

					if (send_over_cb != NULL) {
						send_over_cb(tmp);
					}
				} else if ((0xff&ch) == CAN_CHAR) {
					//log_debug("%d", __LINE__);
					if (fs.frameSend != NULL) {
						fs.frameSend->error = FE_SEND_CAN;
						fs.frameSend->trycnt++;
					}

					timer_cancel(fs.th, &fs.timerSend);
					fs.frameSend = NULL;
					stDataFrame_t *tmp = fs.frameSend;
					fs.stateSend = FSS_READY;

					if (send_over_cb != NULL) {
						send_over_cb(tmp);
					}
				} else {
					//log_debug("%d", __LINE__);
					;
				}
				break;
			case FRS_LENGTH:
				if ((ch&0xff) <= MIN_FRAME_SIZE || (ch&0xff) >= MAX_FRAME_SIZE) {
					fs.stateRecv = FRS_SOF_HUNT;
					timer_cancel(fs.th, &fs.timerRecv);
				} else {
					fs.stateRecv = FRS_TYPE;

					fs.frameRecv = MALLOC(sizeof(stDataFrame_t) + ch-3);
					if (fs.frameRecv != NULL) {
						memset(fs.frameRecv, 0, sizeof(stDataFrame_t) + ch-3);
						fs.frameRecv->sof = SOF_CHAR;
						fs.frameRecv->len = ch;
						fs.frameRecv->size = 0;
						fs.frameRecv->payload = (char*)(fs.frameRecv + 1);
						timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
					} else {
						timer_cancel(fs.th, &fs.timerRecv);
					}
				}
				break;
			case FRS_TYPE:
				if ((ch&0xff) == REQUEST_CHAR || (ch&0xff) == RESPONSE_CHAR) {
					fs.frameRecv->type = ch;
					fs.stateRecv = FRS_COMMAND;
					timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
				}  else {
					fs.stateRecv = FRS_SOF_HUNT;

					fs.frameRecv->error = FE_RECV_TIMEOUT;

					if (recv_over_cb != NULL) {
						recv_over_cb(fs.frameRecv);
						fs.frameRecv = NULL;
					}

					timer_cancel(fs.th, &fs.timerRecv);
				}
				break;
			case FRS_COMMAND:
					fs.frameRecv->cmd = ch;
				if (frame_payload_full(fs.frameRecv)) {
					fs.stateRecv = FRS_CHECKSUM;
					timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
				} else {
					fs.stateRecv = FRS_DATA;
					timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
				}
				break;
			case FRS_DATA:
				fs.frameRecv->payload[fs.frameRecv->size++] = ch;
				if (frame_payload_full(fs.frameRecv)) {
					timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
					fs.stateRecv = FRS_CHECKSUM;
				} else {
					timer_set(fs.th, &fs.timerRecv, FRAME_RECV_NEXT_CH_TIMEOUT);
					fs.stateRecv = FRS_DATA;
				}
				break;
			case FRS_CHECKSUM:
				fs.frameRecv->checksum = ch;
				if (frame_checksum_valid(fs.frameRecv)) {
					fs.frameRecv->error = FE_NONE;
					frame_ack();
				} else {
					fs.frameRecv->error = FE_RECV_CHECKSUM;
				}

				if (recv_over_cb != NULL) {
					recv_over_cb(fs.frameRecv);
				} else {
					FREE(fs.frameRecv);
				}

				fs.frameRecv = NULL;
			
				fs.stateRecv = FRS_SOF_HUNT;
				timer_cancel(fs.th, &fs.timerRecv);
				break;
			default:
				fs.stateRecv = FRS_SOF_HUNT;
				timer_cancel(fs.th, &fs.timerRecv);
				if (fs.frameRecv != NULL) {
					FREE(fs.frameRecv);
					fs.frameRecv = NULL;
				}
				break;
		}
	} while (ret != 1);

	return 0;
}

static void frame_receive_timer_callback(struct timer *timer) {
	fs.stateRecv = FRS_SOF_HUNT;
	timer_cancel(fs.th, &fs.timerRecv);

	if (fs.frameRecv != NULL) {
		fs.frameRecv->error = FE_RECV_TIMEOUT;

		if (recv_over_cb != NULL) {
			recv_over_cb(fs.frameRecv);
		} else {
			FREE(fs.frameRecv);
		}
		fs.frameRecv = NULL;
	}
}

//////////////////////////////////////////////////////////////////
int frame_send(stDataFrame_t *df) {
	if (fs.initFlag == 0) {
		log_err("frame layer not initted!");
		return -1;
	}

	if (fs.stateSend != FSS_READY) {
		log_warn("frame state busy!");
		return -2;
	}

	if (df == NULL) {
		log_warn("frame send argments error: null data frame.");
		return -3;
	}

	if (!frame_checksum_valid(df)) {
		frame_calculate_checksum(df);
		df->checksum = df->checksum_cal;
	}

  char x;
#if 1
	char debug_buf[256];
	int  debug_len = 0;
#endif

  x = SOF_CHAR;
  transport_write(&x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif

	if (df->len != df->size + 3) {
		df->len = df->size + 3;
	}
  x = (df->len)&0xff;
  transport_write(&x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif

  x = df->type;
  transport_write(&x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif

  x = df->cmd;
  transport_write(&x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif
  
	if (df->size > 0) {
		transport_write(df->payload, df->size, 80);
#if 1
		memcpy(debug_buf + debug_len, df->payload, df->size);
		debug_len += df->size;
#endif
	}

	x = df->checksum;
  transport_write(&x, 1, 80);
	debug_buf[debug_len++] = x;

	timer_set(fs.th, &fs.timerSend, FRAME_WAIT_NAK_ACK_TIMEOUT);


	fs.frameSend = df;

	fs.stateSend = FSS_WAIT_ACK_NAK;

#if 1
	log_debug_hex("SerialData:", debug_buf,  debug_len);
#endif

	return 0;
}

static void frame_send_timer_callback(struct timer *timer) {

	if (fs.frameSend != NULL) {
		fs.frameSend->error = FE_SEND_TIMEOUT;
		fs.frameSend->trycnt++;
	}

	if (send_over_cb == NULL) {
		return;
	}

	stDataFrame_t *tmp = fs.frameSend;
	fs.frameSend = NULL;
	fs.stateSend = FSS_READY;

	send_over_cb(tmp);;
}

static void frame_ack() {
	char x = ACK_CHAR;
	transport_write(&x, 1, 80);
}



