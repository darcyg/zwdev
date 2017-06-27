#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include "common.h"
#include "log.h"
#include "jansson.h"
#include "json_parser.h"
#include "hex.h"

#include "memory.h"
#include "zwave_api.h"
#include "zwave_class.h"
#include "zwave_class_init.h"

static int powerlevel_init(int id, char class, int version);
static int switch_binary_init(int id, char class, int version);
static int zwaveplus_info_init(int id, char class, int version);
static int association_init(int id, char class, int version);
static int association_grp_info_init(int id, char class, int version);
static int version_init(int id, char class, int version);
static int manufacturer_specific_init(int id, char class, int version);
static int device_reset_locally_init(int id, char class, int version);
static int battery_init(int id, char class, int version);
static int wakeup_init(int id, char class, int version);
static int notify_alarm_init(int id, char class, int version);
static int switch_multi_init(int id, char class, int version);
static int switch_all_init(int id, char class, int version);
static int protection_init(int id, char class, int version);
static int configure_init(int id, char class, int version);

static stClassCommandFuncs_t _zwave_class_init_funcs[] = {
	[0x73] = {0x73, powerlevel_init},
	[0x25] = {0x25, switch_binary_init},
	[0x5e] = {0x5e, zwaveplus_info_init},
	[0x85] = {0x85, association_init},
	[0x59] = {0x59, association_grp_info_init},
	[0x86] = {0x86, version_init},
	[0x72] = {0x72, manufacturer_specific_init},
	[0x5a] = {0x5a, device_reset_locally_init},
	[0x80] = {0x80, battery_init},
	[0x84] = {0x84, wakeup_init},
	[0x71] = {0x71, notify_alarm_init},
	[0x26] = {0x26, switch_multi_init},
	[0x27] = {0x27, switch_all_init},
	[0x75] = {0x75, protection_init},
	[0x70] = {0x70, configure_init},
};

int zwave_class_init_init(int id, char class, int _version) {
	log_info("[%d] : dev(%02x), class(%02x)", __LINE__, id&0xff, class);
	int ret = zwave_class_version_get(id, class);
	if (ret < 0) {
		log_err("[%d] get version failed: %d", __LINE__, ret);
		return -1;
	}
	log_info("[%d] class:%02x, version:%d", __LINE__, class&0xff, ret);
	int version = ret;

	json_t *jdev = memory_get_dev(id);
	json_t *jclasses = json_object_get(jdev, "classes");
	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);
	json_object_set_new(jclass, "version", json_integer(version));
	
	if (_zwave_class_init_funcs[class&0xff].init != NULL) {
		_zwave_class_init_funcs[class&0xff].init(id, class, version);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

static int powerlevel_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);

	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | power level*/
	/* 73        03        level */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}
	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	return 0;
}

static int switch_binary_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);

	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 25        03        value  */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}
	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	return 0;
}

static int zwaveplus_info_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x01;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 5e        02      value(3 v1) or value(5 v2) */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	return 0;
}

static int association_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;

	char command = 0x05;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 85        06      supportted groups(1bype v1/v2) */
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	char ars = outparam[5];
	int i = 0;
	for (i = 0; i < (ars&0xff); i++) {

		char gid = i+1;
		char command = 0x01;
		char buf[2] = {gid, 0x01};
		int ret = zwave_api_util_cc(id, class, command, buf, 2, 0, outparam, &outlen);
		if (ret < 0) {
			log_err("[%d] association set failed:%d", __LINE__, ret);
		}

		command = 0x02;
		
		ret = zwave_api_util_cc(id, class, command, &gid, 1, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 85        03      gid, maxgrp, reports to flow, node id,*/

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	}

	if (version == 2) {
		command = 0x0b;
		ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 85        0c      supportted groups(1bype v1/v2) */

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	}
	
	return 0;
}
static int association_grp_info_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	return 0;
}
static int version_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);

	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x11;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 86        12      5(v1) |  7(v2）+*/
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));
	
	return 0;
}
static int manufacturer_specific_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x04;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 72        05      5(v1) |  7(v2）+*/
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	if (version == 2) {
		char outparam[128];
		int outlen;
		char command = 0x06;
		//char inparam[] = {0x01};
		char inparam[] = {0x02};
		int ret = zwave_api_util_cc(id, class, command, inparam, 1, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 72       07      5(v1) |  7(v2）+*/

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	}
	
	return 0;
}

static int device_reset_locally_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	return 0;
}

static int battery_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 80        03      battery*/
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	return 0;
}


static int wakeup_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x04;
	//int wui = 15 * 60;
	int wui = 1 * 15;
	char inparam[4] = {(wui>>16)&0xff, (wui>>8)&0xff,(wui>>0)&0xff, 0x01};
	int ret = zwave_api_util_cc(id, class, command, inparam, sizeof(inparam), 0, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 84        06      interval*/
	command = 0x05;
	ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	return 0;
}


static int notify_alarm_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	if (version < 3){ return 0; }

	
	char support_rpt;
	char support_evt;
	char v1_alarm;
	{
		char outparam[128];
		int outlen;
		char command = 0x07;
		int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);

		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));

		support_rpt = outparam[5+1];
		int i = 0;
		for (i = 0; i < 8; i++) {
			if ((support_rpt&0xff) == ((1 << i)&0xff)) {
				break;
			}
		}
		support_rpt = i&0xff;
		
		v1_alarm = !!(outparam[5+0]&0x80);
	}

		
	{
		
		char outparam[128];
		int outlen;
		char inparam[1] = {support_rpt};
		char command = 0x01;
		int ret = zwave_api_util_cc(id, class, command, inparam, 1, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);


		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}
		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
		
		support_evt = outparam[5+2];
	}

	{

		char outparam[128];
		int outlen;
		char inparam[2] = {support_rpt, 0xff};
		char command = 0x06;
		int ret = zwave_api_util_cc(id, class, command, inparam, 2, 0, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

	}

	{
		v1_alarm  = v1_alarm;
		support_evt = support_evt;
		/*
		char outparam[128];
		int outlen;
		char inparam[3] = {v1_alarm, support_rpt, support_evt};
		char command = 0x04;
		int ret = zwave_api_util_cc(id, class, command, inparam, 3, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);


		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}
		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
		*/
	}
	
	return 0;
}

static int switch_multi_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	{
		char outparam[128];
		int outlen;
		char command = 0x01;
		char inparam[2] = {0x02, 0x02};
		int ret;
		if (version  == 1) {
			ret = zwave_api_util_cc(id, class, command, inparam, 1, 0, outparam, &outlen);
		} else if (version >= 2) {
			ret = zwave_api_util_cc(id, class, command, inparam, 2, 0, outparam, &outlen);
		}
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}
	
		command = 0x02;
		ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	}
	if (version >= 3) {
		char outparam[128];
		int outlen;
		char command = 0x06;
		int ret = zwave_api_util_cc(id, class, command, NULL, 0, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));

	}
	
	return 0;
}
static int switch_all_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	char outparam[128];
	int outlen;
	char command = 0x01;
	char inparam[1] = {0x03};
	int ret;
	ret = zwave_api_util_cc(id, class, command, inparam, 1, 0, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}
	
	command = 0x02;
	ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", outparam[4]&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}

	char svalue[(outparam[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	return 0;
}
static int protection_init(int id, char class, int version) {
	log_info("[%d]", __LINE__);
	if (version < 1) {
		log_err("[%d] %02x class version error: %d", __LINE__, class, version);
		return -1;
	}

	if (version == 1) {
		char outparam[128];
		int outlen;
		char command = 0x01;
		char inparam[1] = {0x00};
		int ret;
		ret = zwave_api_util_cc(id, class, command, inparam, 1, 0, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		command = 0x02;
		ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		json_t *jdev = memory_get_dev(id);

		json_t *jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);

		char scommand[32];
		sprintf(scommand, "%02x", outparam[4]&0xff);
		json_t *jcommand = json_object_get(jclass, scommand);
		if (jcommand != NULL) {
			json_object_del(jclass, scommand);
		}

		char svalue[(outparam[2] - 2) * 3+1];
		hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
		json_object_set_new(jclass, scommand, json_string(svalue));
	} else if (version == 2) {	
		int timeout_support = 0;
		int exclusive_support = 0;
		int local_state = 0;
		int rf_state = 0;
		{
			char outparam[128];
			int outlen;
			char command = 0x04;
			int ret;
			ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			json_t *jdev = memory_get_dev(id);

			json_t *jclasses = json_object_get(jdev, "classes");

			char sclass[32];
			sprintf(sclass, "%02x", class&0xff);
			json_t *jclass = json_object_get(jclasses, sclass);

			char scommand[32];
			sprintf(scommand, "%02x", outparam[4]&0xff);
			json_t *jcommand = json_object_get(jclass, scommand);
			if (jcommand != NULL) {
				json_object_del(jclass, scommand);
			}

			char svalue[(outparam[2] - 2) * 3+1];
			hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
			json_object_set_new(jclass, scommand, json_string(svalue));

			timeout_support = !!(outparam[5]&0x1);
			exclusive_support = !!(outparam[5]&0x2);
			local_state = outparam[5+1];
			rf_state = outparam[5+1];
			local_state = local_state;
			rf_state = rf_state;
		}

		if (timeout_support) {
			char outparam[128];
			int outlen;
			char command = 0x09;
			char inparam[1] = {0x00};
			int ret;
			ret = zwave_api_util_cc(id, class, command, inparam, 1, 0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			command = 0x0a;
			ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			json_t *jdev = memory_get_dev(id);

			json_t *jclasses = json_object_get(jdev, "classes");

			char sclass[32];
			sprintf(sclass, "%02x", class&0xff);
			json_t *jclass = json_object_get(jclasses, sclass);

			char scommand[32];
			sprintf(scommand, "%02x", outparam[4]&0xff);
			json_t *jcommand = json_object_get(jclass, scommand);
			if (jcommand != NULL) {
				json_object_del(jclass, scommand);
			}

			char svalue[(outparam[2] - 2) * 3+1];
			hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
			json_object_set_new(jclass, scommand, json_string(svalue));
		}

		if (exclusive_support) {
			char outparam[128];
			int outlen;
			char command = 0x06;
			char inparam[1] = {0x01};
			int ret;
			ret = zwave_api_util_cc(id, class, command, inparam, 1, 0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			command = 0x07;
			ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			json_t *jdev = memory_get_dev(id);

			json_t *jclasses = json_object_get(jdev, "classes");

			char sclass[32];
			sprintf(sclass, "%02x", class&0xff);
			json_t *jclass = json_object_get(jclasses, sclass);

			char scommand[32];
			sprintf(scommand, "%02x", outparam[4]&0xff);
			json_t *jcommand = json_object_get(jclass, scommand);
			if (jcommand != NULL) {
				json_object_del(jclass, scommand);
			}

			char svalue[(outparam[2] - 2) * 3+1];
			hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
			json_object_set_new(jclass, scommand, json_string(svalue));
		}

		if (1) {
			char outparam[128];
			int outlen;
			char command = 0x01;
			char inparam[2] = {0x00, 0x00};
			int ret;
			ret = zwave_api_util_cc(id, class, command, inparam, 2, 0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			command = 0x02;
			ret = zwave_api_util_cc(id, class, command, NULL,  0, 1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			json_t *jdev = memory_get_dev(id);

			json_t *jclasses = json_object_get(jdev, "classes");

			char sclass[32];
			sprintf(sclass, "%02x", class&0xff);
			json_t *jclass = json_object_get(jclasses, sclass);

			char scommand[32];
			sprintf(scommand, "%02x", outparam[4]&0xff);
			json_t *jcommand = json_object_get(jclass, scommand);
			if (jcommand != NULL) {
				json_object_del(jclass, scommand);
			}

			char svalue[(outparam[2] - 2) * 3+1];
			hex_string(svalue, sizeof(svalue), (const u8*)&outparam[5], outparam[2] - 2, 1, ' ');
			json_object_set_new(jclass, scommand, json_string(svalue));
		}
	}

	return 0;
}
static int configure_init(int id, char class, int version) {
	return 0;
}




