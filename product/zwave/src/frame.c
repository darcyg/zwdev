#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "log.h"

#include "frame.h"
#include "serial.h"

static int fd = -1;
static int state = FRS_SOF_HUNT;

int frame_len(stDataFrame_t *df) {
  return df->len;
}

int frame_size(stDataFrame_t *df) {
  return df->size;
}

char * frame_payload(stDataFrame_t *df) {
  return df->payload;
}

int frame_timestamp(stDataFrame_t *df) {
  return df->timestamp;
}
unsigned char frame_cmd(stDataFrame_t *df) {
  return df->cmd;
}
unsigned char frame_type(stDataFrame_t *df) {
  return df->type;  
}
unsigned char frame_checksum(stDataFrame_t *df) {
  return df->checksum;
}
int frame_payload_full(stDataFrame_t *df) {
  return !!(df->size == df->len - 3);
}
int frame_checksum_valid(stDataFrame_t *df) {
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

int	frame_init(const char *dev, int buad) {
	int ret = serial_open(dev, buad);
	if (ret <= 0) {
		log_err("serial open %s(%d) failed!", dev, buad);
		return -1;
	}
	fd = ret;
	serial_flush(fd);
	return 0;
}

stDataFrame_t *	frame_make(int api, char *param, int paramsize) {
	int size = sizeof(stDataFrame_t) + paramsize;

	stDataFrame_t *df = MALLOC(size);
	if (df == NULL) {
		return NULL;
	}

	df->sof			= SOF_CHAR;
	df->len			= paramsize+3;
	df->type		= 0x00,
	df->cmd			= api;
	df->payload = (char *)(df + 1);
	df->size		= paramsize;
	if (df->size > 0) {
		memcpy(df->payload, param, paramsize);
	}
	df->checksum = 0;
	df->timestamp = time(NULL);
	df->checksum_cal = 0;
	df->do_checksum = 0;
	df->trycnt =0;
	df->error = 0;
	
	return df;
}

int	frame_send(stDataFrame_t *df, int timeout) {
	if (fd <= 0) {
		log_err("frame layer not initted!");
		return -1;
	}

	if (df == NULL) {
		log_warn("frame send argments error: null data frame.");
		return -2;
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
  serial_write(fd, &x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif

	if (df->len != df->size + 3) {
		df->len = df->size + 3;
	}
  x = (df->len)&0xff;
  serial_write(fd, &x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif

  x = df->type;
  serial_write(fd, &x, 1, 80);
#if 1
	debug_buf[debug_len++] = x;
#endif

  x = df->cmd;
	serial_write(fd, &x, 1, timeout);
#if 1
	debug_buf[debug_len++] = x;
#endif
  
	if (df->size > 0) {
		serial_write(fd, df->payload, df->size, 80);
#if 1
		memcpy(debug_buf + debug_len, df->payload, df->size);
		debug_len += df->size;
#endif
	}

	x = df->checksum;
  serial_write(fd, &x, 1, 80);
	debug_buf[debug_len++] = x;

#if 1
	log_debug_hex("SerialData:", debug_buf,  debug_len);
#endif

	return 0;
}

/* return ack ,  nak , can , frame*/
/*
 * 0 - ok
 * 1 - ack 
 * 2 - nak
 * 3 - can 
 * 5 - recv timeout
 */
int	frame_recv(stDataFrame_t **frame, int timeout) {
	char ch;
	int ret;
	stDataFrame_t *df = NULL;
	int tout = timeout;

	do {
		ret = serial_read(fd, &ch, 1, tout);
		if (ret != 1) {
			if (state == FRS_SOF_HUNT) {
				return FR_TIMEOUT;
			} else {
				state = FRS_SOF_HUNT;
				df->error = FE_RECV_TIMEOUT;
				*frame = df;
				return FR_OK;
			}
			break;
		}
		switch (state) {
			case FRS_SOF_HUNT:
				if ((ch&0xff) == SOF_CHAR) {
					//log_debug("%d: %02x, %02x", __LINE__, ch, SOF_CHAR);
					state = FRS_LENGTH;
					tout = FRAME_RECV_NEXT_CH_TIMEOUT;
				} else if ((ch&0xff) == ACK_CHAR) {
					state = FRS_SOF_HUNT;
					return FR_ACK;
				} else if ((0xff&ch) == NAK_CHAR) {
					log_debug("%d nak", __LINE__);
					state = FRS_SOF_HUNT;
					return FR_NAK;
				} else if ((0xff&ch) == CAN_CHAR) {
					log_debug("%d can", __LINE__);
					state = FRS_SOF_HUNT;
					return FR_CAN;
				} else {
					log_debug("%d something else", __LINE__);
					;
				}
				break;
			case FRS_LENGTH:
				if ((ch&0xff) <= MIN_FRAME_SIZE || (ch&0xff) >= MAX_FRAME_SIZE) {
					state = FRS_SOF_HUNT;
				} else {

					df = MALLOC(sizeof(stDataFrame_t) + ch-3);
					if (df != NULL) {
						memset(df, 0, sizeof(stDataFrame_t) + ch-3);
						df->sof = SOF_CHAR;
						df->len = ch;
						df->size = 0;
						df->payload = (char*)(df + 1);

						state = FRS_TYPE;
					} else {
						state = FRS_SOF_HUNT;
					}
				}
				break;
			case FRS_TYPE:
				if ((ch&0xff) == REQUEST_CHAR || (ch&0xff) == RESPONSE_CHAR) {
					df->type = ch;
					state = FRS_COMMAND;
				}  else {
					state = FRS_SOF_HUNT;
					df->error = FE_RECV_ERROR_TYPE;
					*frame = df;
					return FR_OK;
				}
				break;
			case FRS_COMMAND:
				df->cmd = ch;
				if (frame_payload_full(df)) {
					state = FRS_CHECKSUM;
				} else {
					state = FRS_DATA;
				}
				break;
			case FRS_DATA:
				df->payload[df->size++] = ch;
				if (frame_payload_full(df)) {
					state = FRS_CHECKSUM;
				} else {
					state = FRS_DATA;
				}
				break;
			case FRS_CHECKSUM:
				df->checksum = ch;
				if (frame_checksum_valid(df)) {
					df->error = FE_NONE;
					frame_ack();
				} else {
					df->error = FE_RECV_CHECKSUM;
				}

				state = FRS_SOF_HUNT;
				
				*frame = df;
				return FR_OK;
				break;
			default:
				state = FRS_SOF_HUNT;
				if (df != NULL) {
					FREE(df);
					df = NULL;
				}
				break;
		}
	} while (ret == 1);

	return FR_TIMEOUT;
}

int	frame_ack() {
	char x = ACK_CHAR;
	serial_write(fd, &x, 1, 80);
	return 0;
}
