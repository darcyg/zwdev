#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "log.h"

#include "zwave_api.h"
#include "zwave_class.h"

int zwave_class_version_get(int id, char class) {
	log_info("[%d] dev(%02x), class(%02x)", __LINE__, id&0xff, class&0xff);
	
	char outparam[128];
	char inparam[1] = {class&0xff};
	int outlen;
	int ret = zwave_api_util_cc(id, 0x86, 0x13, inparam, sizeof(inparam), 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -1;
	}

	/* rxStatus | sourceNode | len              */
	/* class    | command    | class  | version */
	/* 86          14                           */
	if ((outparam[5]&0xff) != (class&0xff)) {
		log_err("[%d] not corrent class to get: want(%02x), get(%02x)", __LINE__, class&0xff, outparam[5]&0xff);
		return -2;
	}

	int version = outparam[6] & 0xff;

	return version;

}
