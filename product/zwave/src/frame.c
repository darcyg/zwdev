#include "frame.c"

typedef strcut stFrameState {
  void **receive_queue;
  int receive_queue_size;
  int receive_head;
  int receive_tail;

  void **send_queue;
  int send_head;
  int send_tail;
  int send_queue_size;

  stDataFrame *df;

  struct timer frame_send_state_timer;
  struct timer frame_receive_state_timer;

  int frame_state_init_flag;

  int frame_state_receive_state;
  int frame_state_send_state;

  
} stFrameState_t;

static stFrameState_t fs = {
  .frame_state_init_flag = 0,
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
  return !!(df->size == df->len);
}
int frame_checksum_valid(stDataFrame_t *df) {
  if (df == NULL) {
    return 0;
  }
  if (df->do_checksum == 0) {
    frame_calculate_checksum(df);
    df->do_checksum= 1;
  }
  return (df->checksum_cal == df->checksum);
}

int frame_calculate_checksum(stDataFrame_t *df) {
  unsigned char calcChksum = 0xFF;
  
  calcChksum ^= (unsigned char)(df->len&0xff); // Length
  calcChksum ^= df->type;     // Type
  calcChksum ^= df->cmd;      // Command

  int i = 0;
  for (i = 0; i < df->len; i++)
    calcChksum ^= df->payload[i];      // Data

  df->checksum = calcChksum;

  return 0;
}

int frame_state_init(int receive_queue_max_size, int send_queue_max_size) {
  int ret;

  if (fs.frame_state_init_flag != 0) {
    log_debug("frame state has been init!");
    return 0;
  }
  
  if (receive_queue_max_size < 10) {
    log_debug("frame state receive queue too small");
    return -1;
  }
  if (send_queue_max_size < 10) {
    log_debug("frame state receive queue too small");
    return -2;
  }

  if (transport_open() != 0) {
    log_debug("transport open failed!");
    return -3;
  }
  
  fs.receive_queue_size = receive_queue_max_size;
  fs.send_queue_size = send_queue_max_size;
  fs.receive_queue = MALLOC(sizeof(void *) * receive_queue_size);
  fs.send_queue     = MALLOC(sizeof(void*) * send_queue_size);
  if (fs.receive_queue == NULL || fs.send_queue == NULL) {
    log_debug("frame state memory out!");
    ret = -3;
    goto fail;
  }
  fs.receive_head = fs.receive_tail = 0;
  fs.send_tail = fs.send_tail = 0;

  
  timer_init(&fs.frame_send_state_timer);
  timer_init(&fs.frame_receive_state_timer);

  fs.frame_state_init_flag = 1;

  ret = 0;
 fail:
    if (receive_queue != NULL) {
      FREE(receive_queue);
      receive_queue = NULL;
    }
    if (send_queue != NULL) {
      FREE(send_queue);
      send_queue = NULL;
    }
    if (transport_is_open()) {
      transport_close();
    }
 done:
    
  return ret;
}

int frame_state_getfd() {
  return transport_getfd();
}

int frame_receive_state_reset() {
  timer_reset(&fs.frame_send_state_timer);
  timer_reset(&fs.frame_receive_state_timer);
  frame_send_state_queue_free();
  frame_receive_state_queue_free();
  
  fs.frame_state_receive_state = FRS_SOF_HUNT;
  fs.frame_state_send_state = FSS_WAIT_SEND;
}

int frame_state_free() {
  timer_cancle(&fs.frame_send_state_timer);
  timer_cancle(&fs.frame_receive_state_timer);
  
  if (transport_close() != 0) {
    log_debug("transport_close failed!");
    return -1;
  }
  
  frame_send_state_queue_free();
  frame_receive_state_queue_free();
  FREE(fs.receive_queue);
  FREE(fs.send_queue);
  
  fs.frame_state_receive_state = FRS_SOF_HUNT;
  fs.frame_state_send_state = FSS_WAIT_SEND;
  

  
  fs.frame_state_init_flag = 0;
  return 0;
}



stDataFrame_t * frame_receive_state_get_frame() {
  if (fs.receive_head == fs.receive_tail) {
    return NULL;
  }
  int idx = fs.receive_tail;
  fs.receive_tail++;
  if (fs.receive_tail == fs.receive_queue_size) {
    fs.receive_tail = 0;
  }

  return fs.receive_queue[fs.receive_tail];
}



int frame_receive_state_step() {
  char ch;

  do {
    int ret = transport_read(&ch, 1, 8000);
    switch (fs.frame_state_receive_state) {
    case FRS_SOF_HUNT:
      if (ch == SOF_CHAR) {
	fs.frame_state_receive_state = FRS_LENGTH;
      } else if (ch == ACK_CHAR) {
	frame_ack_received();
      } else if (ch == NAK_CHAR) {
	frame_nak_received();
      } else if (ch == CAN_CHAR) {
	frame_can_received();
      } else {
	;
      }
      break;
    case FRS_LENGTH:
      if (ch <= MIN_FRAME_SIZE || ch >= MAX_FRAME_SIZE) {
	fs.frame_state_receive_state = FRS_SOF_HUNT;
      } else {
	fs.frame_state_receive_state = FRS_TYPE;

	fs.df = MALLOC(sizeof(stDataFrame_t) + ch);
	if (fs.df != NULL) {
	  memset(&fs.df, sizeof(fs.df));
	  fs.df->sof = SOF_CHAR;
	  fs.df->len = ch;
	  fs.df->size = 0;
	  fs.df->state = fs.frame_state_receive_state;
	  fs.df->payload = fs.df + 1;
	}
      }
      break;
    case FRS_TYPE:
      if (ch == REQUEST_CHAR || ch == RESPONSE_CHAR) {
	fs.frame_state_receive_state = FRS_COMMAND;
      }  else {
	fs.frame_state_receive_state = FRS_SOF_HUNT;
      }
      break;
    case FRS_COMMAND:
      if (frame_payload_full(fs.df)) {
	fs.frame_state_receive_state = FRS_CHECKSUM;
      } else {
	fs.frame_state_receive_state = FRS_DATA;
      }
      break;
    case FRS_DATA:
      fs.df->payload[size++] = ch;
      if (frame_payload_full(fs.df)) {
	fs.frame_state_receive_state = FRS_CHECKSUM;
      } else {
	fs.frame_state_receive_state = FRS_DATA;
      }
      break;
    case FRS_CHECKSUM:
      fs.df->checksum = ch;
      if (frame_checksum_valid(fs.df)) {
	frame_send_ack();
	frame_received(fs.df);
      } else {
	frame_send_nak();
	FREE(fs.df);
	fs.df = NULL;
      }
      fs.frame_state_receive_state = FRS_SOF_HUNT;
      break;
    default:
      fs.frame_state_receive_state = FRS_SOF_HUNT;
      if (fs.df != NULL) {
	FREE(fs.df);
	fs.df = NULL;
      }
      break;
    }
  }
  /* receive and parse */
}

void frame_receive_state_timer_callback(void *timer) {
  ;
}

int frame_send_state_send(stDataFrame_t *df) {
  int ret;
  
  if (fs.send_head + 1 == fs.send_tail) {
    log_debug("send queue full!");
    ret = -1;
    goto send_done;
  }
  if ( ((fs.send_head+1)%fs.send_queue_size) == fs.send_tail) {
    log_debug("send queue full!");
    goto send_done;
  }
  fs.send_queue[fs.send_head] = df;
  fs.send_head = (fs.send_head + 1)%fs.send_queue_size;

 send_done:
  if (fs.send_head != fs.send_tail) {
    stDataFrame_t *sdf = fs.send_queue[fs.send_head];
    if (sdf->state == FSS_WAIT_SEND) {
      frame_send(sdf);
      sdf->state = FSS_SENDDED_AND_WAIT_ACK_NAK;
      timer_reset(&fs.frame_send_state_timer);
    }
  }
}

void frame_send_state_timer_callback(void *timer) {
  if (fs.send_head == fs.send_tail) {
    return;
  }

  stDataFrame_t *sdf = fs.send_queue[fs.send_head];
  fs.send_head = (fs.send_head + 1) % fs.send_queue_size;
  
  if (sdf->state == FSS_SENDDED_AND_WAIT_ACK_NAK) {
    sdf->state == FSS_TX_TIMEOUT;
    session_send_callback(sdf);
  }

  if (fs.send_head != fs.send_tail) {
    stDataFrame_t *sdf = fs.send_queue[fs.send_head];
    if (sdf->state == FSS_WAIT_SEND) {
      frame_send(sdf);
      sdf->state = FSS_SENDDED_AND_WAIT_ACK_NAK;
      timer_reset(&fs.frame_send_state_timer);
    }
  }  
}


int frame_send(stDataFrame_t *df) {
  frame_calculate_checksum(df);
  char x;

  x = SOF_CHAR;
  transport_write(&x, 1);
  x = df->len&0xff;
  transport_write(&x, 1);
  x = df->type;
  transport_write(&x, 1);
  x = df->cmd;
  transport_write(&x, 1);
  
  transport_write(df->payload, df->size);

  transport_write(df->checksum);
}

int frame_received(stDataFrame_t *df) {
  //session_recv_callback(df);
}
