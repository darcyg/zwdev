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
  int state;
  int error;
  unsigned char checksum_cal;
  int do_checksum;
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
  FRS_RX_TIMEOUT = 0x06,
  FRS_RX_DONE = 0x07,
}emFrameReceiveState_t;

enum emFrameSendState {
  FSS_WAIT_SEND = 0x00,
  FSS_SENDDED_AND_WAIT_ACK_NAK = 0x01,
  FSS_TX_TIMEOUT = 0x02,
  FSS_TX_DONE = 0x03,
};

enum emFrameError {
  FE_NONE = 0x00,
};

#define SOF_CHAR 0xfe


int frame_state_init(int receive_queue_max_size, int send_queue_max_size);
int frame_state_getfd();

int frame_receive_state_reset();
int frame_receive_state_step();
stDataFrame_t * frame_receive_state_get_frame();
void frame_receive_state_timer_callback(void *timer);

int frame_send_state_send(stDataFrame_t *df);
void frame_send_state_timer_callback(void *timer);

int frame_state_free();

int frame_send(stDataFrame_t *df);
int frame_received(stDataFrame_t df);
#endif
