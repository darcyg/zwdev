#ifndef _FRAME_H_
#define _FRAME_H_

typedef struct stDataFrame {
  unsigned char sof;
  int len;
  unsigned char type;
  unsigned char cmd;
  char *payload;
  int size; /* payload size */
  unsigned char checksum;
  int timestamp;
  unsigned char checksum_cal;
  int do_checksum;
	
	int trycnt;
	int error;
}stDataFrame_t;

int frame_len(stDataFrame_t *df);
int frame_size(stDataFrame_t *df);
char * frame_payload(stDataFrame_t *df);
int frame_timestamp(stDataFrame_t *df);
unsigned char frame_cmd(stDataFrame_t *df);
unsigned char frame_type(stDataFrame_t *df);
unsigned char frame_checksum(stDataFrame_t *df);
int frame_payload_full(stDataFrame_t *df);
int frame_checksum_valid(stDataFrame_t *df);
int frame_calculate_checksum(stDataFrame_t *df);

enum emFrameReceiveState {
  FRS_SOF_HUNT = 0x00,
  FRS_LENGTH = 0x01,
  FRS_TYPE = 0x02,
  FRS_COMMAND = 0x03,
  FRS_DATA = 0x04,
  FRS_CHECKSUM = 0x05,
}emFrameReceiveState_t;

enum emFrameSendState {
  FSS_READY = 0x00,
  FSS_WAIT_ACK_NAK = 0x01,
};

enum emFrameError {
  FE_NONE = 0x00,
	FE_SEND_TIMEOUT = 0x01,
	FE_SEND_ACK = 0x02,
	FE_SEND_NAK = 0x03,
	FE_SEND_CAN = 0x04,
	FE_RECV_CHECKSUM = 0x05,
	FE_RECV_TIMEOUT = 0x06,
};

#define SOF_CHAR 0xfe
#define ACK_CHAR 0x06
#define NAK_CHAR 0x15
#define CAN_CHAR 0x18

#define FRAME_WAIT_NAK_ACK_TIMEOUT 1000
#define FRAME_RECV_NEXT_CH_TIMEOUT 200

#define MIN_FRAME_SIZE 1 
#define MAX_FRAME_SIZE 233

#define REQUEST_CHAR  0x00
#define RESPONSE_CHAR 0x01

typedef void (*FRAME_SEND_OVER_CALLBACK)(stDataFrame_t *sf);
typedef void (*FRAME_RECV_COMP_CALLBACK)(stDataFrame_t *sf);
int frame_init(void *_th, FRAME_SEND_OVER_CALLBACK _send_over_cb, 
										 FRAME_RECV_COMP_CALLBACK _recv_over_cb);
int frame_getfd();

int frame_receive_reset();
int frame_receive_step();


int frame_send(stDataFrame_t *df);

int frame_free();

#endif
