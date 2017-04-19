#include "api.h"

static API_CALLBACK api_cb = NULL;

static void send_over(stDataFrame_t *df) {
	log_debug("%s send over!", cmdStringMap(df->cmd));
	FREE(df);
}

static void recv_over(stDataFrame_t *df) {
	log_debug("%s recv over!", cmdStringMap(df->cmd));
	FREE(df);
}

static stDataFrame_t * make_frame(emApi_t api, stParam_t param) {
	static int seq = 0;
	int size = sizeof(stDataFrame_t) + paramSizeMap[api];

	stDataFrame_t *df = MALLOC(size);
	if (df == NULL) {
		return NULL;
	}

	df->sof			= SOF_CHAR;
	df->len			= paramSizeMap[api] + 3;
	df->type		= 0x00,
	df->cmd			= api;
	df->payload = (char *)(df + 1);
	df->size		= paramSizeMap[api];
	df->checksum = 0;
	df->timestamp = time(NULL);
	df->checksum_cal = 0;
	df->do_checksum = 0;
	df->trycnt =0;
	df->error = 0;
	
	
	return df;
}

//////////////////////////////////////////////
int api_init(void *_th, API_CALLBACK _acb) {
	if (_acb != NULL) {
		api_cb = _acb;
	}

	if (frame_init(_th, send_over, recv_over) != 0) {
		log_debug("frame init failed!");
		return -1;
	}
}

int api_exec(emApi_t api, stParam_t param) {
	stDataFrame * df = make_frame(api, param);
	if (df == NULL) {
		log_debug("make frame failed!");
		return -1;
	}
	
	return frame_send(df);
}

int api_free() {
	frame_free();
	return 0;
}

int api_getfd() {
	return frame_getfd();
}



