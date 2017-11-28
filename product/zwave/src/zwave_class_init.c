#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include "common.h"
#include "log.h"

#include "zwave_api.h"
#include "zwave_class.h"
#include "zwave_class_init.h"
#include "zwave_class_cmd.h"

static int powerlevel_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int switch_binary_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int zwaveplus_info_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int association_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int association_grp_info_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int version_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int manufacturer_specific_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int device_reset_locally_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int battery_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int wakeup_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int notify_alarm_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int switch_multi_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int switch_all_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int protection_init(stZWaveDevice_t *zd, stZWaveClass_t *class);
static int configure_init(stZWaveDevice_t *zd, stZWaveClass_t *class);

static stClassCommandFuncs_t _zwave_class_init_funcs[256] = {
	//[0x73] = {0x73, powerlevel_init},
	//[0x25] = {0x25, switch_binary_init},
	//[0x5e] = {0x5e, zwaveplus_info_init},

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

int zwave_class_init_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d] : dev(%02x), class(%02x)", __LINE__, zd->bNodeID&0xff, class->classid&0xff);
	int ret = zwave_class_version_get(zd->bNodeID, class->classid&0xff);
	if (ret < 0) {
		log_err("[%d] get version failed: %d", __LINE__, ret);
		return -1;
	}
	log_info("[%d] class:%02x, version:%d", __LINE__, class->classid&0xff, ret);
	int version = ret;
	

	if (class->version < 0) {
		class->version = version;
		stZWClass_t *c = zcc_get_class(class->classid, class->version);
		if (c != NULL) {
			char cmds[MAX_CMD_NUM];
			int cmdcnt = zcc_get_class_cmd_rpt(c, cmds);
			if (cmdcnt > 0) {
				device_add_cmds(class, cmdcnt, cmds);
			}
		}
	} 
	
	if (_zwave_class_init_funcs[class->classid&0xff].init != NULL && class->version >= 1) {
		_zwave_class_init_funcs[class->classid&0xff].init(zd, class);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
static int powerlevel_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | power level*/
	/* 73        03        level */
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}
	
	return 0;
}

static int switch_binary_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 25        03        value  */
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}

	return 0;
}

static int zwaveplus_info_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);
	
	char outparam[128];
	int outlen;
	char command = 0x01;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 5e        02      value(3 v1) or value(5 v2) */
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}

	return 0;
}

static int association_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);


	char outparam[128];
	int outlen;
	char command = 0x05;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 85        06      supportted groups(1bype v1/v2) */
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd == NULL) {
		log_warn("not support command : %02X", cmdid&0xff);
		return 0;
	}
	device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	
	char ars = outparam[5];
	int i = 0;
	for (i = 0; i < (ars&0xff); i++) {
		char gid = i+1;
		char command = 0x01;
		char buf[2] = {gid, 0x01};
		int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, buf, 2, 0, outparam, &outlen);
		if (ret < 0) {
			log_err("[%d] association set failed:%d", __LINE__, ret);
		}

		command = 0x02;
		ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, &gid, 1, 1, outparam, &outlen);
		if (ret < 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 85        03      gid, maxgrp, reports to flow, node id,*/
		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	}

	if (class->version == 2) {
		command = 0x0b;
		int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 85        0c      supportted groups(1bype v1/v2) */
		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	}
	return 0;
}
static int association_grp_info_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);
	return 0;
}
static int version_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x11;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid, command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 86        12      5(v1) |  7(v2）+*/
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}

	return 0;
}
static int manufacturer_specific_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x04;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 72        05      5(v1) |  7(v2）+*/
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}

	if (class->version == 2) {
		char outparam[128];
		int outlen;
		char command = 0x06;
		//char inparam[] = {0x01};
		char inparam[] = {0x02};
		int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 1, 1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		/* rxStatus | sourceNode | len */
		/* class | command | value */
		/* 72       07      5(v1) |  7(v2）+*/
		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	}
	return 0;
}

static int device_reset_locally_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);
	return 0;
}

static int battery_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x02;
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0, 1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 80        03      battery*/
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}

	return 0;
}

static int wakeup_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x04;
	//int wui = 15 * 60;
	int wui = 15*60;
	char inparam[4] = {(wui>>16)&0xff, (wui>>8)&0xff,(wui>>0)&0xff, 0x01};
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, sizeof(inparam), 0, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* 84        06      interval*/
	command = 0x05;
	ret = zwave_api_util_cc(zd->bNodeID,0,  class->classid,  command, NULL, 0,  1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}
	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}
	
	return 0;
}

static int notify_alarm_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);


	if (class->version < 3){ return 0; }

	
	char support_rpt;
	char support_evt;
	char v1_alarm;
	{
		char outparam[128];
		int outlen;
		char command = 0x07;
		int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}
		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	
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
		int ret = zwave_api_util_cc(zd->bNodeID,0,  class->classid,  command, inparam, 1,  1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}
		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	
		support_evt = outparam[5+2];
	}

	{

		char outparam[128];
		int outlen;
		char inparam[2] = {support_rpt, 0xff};
		char command = 0x06;
		int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 2,  0, outparam, &outlen);
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
		int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 3,  1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}
		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
		*/
	}
	
	return 0;
}

static int switch_multi_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	{
		char outparam[128];
		int outlen;
		char command = 0x01;
		char inparam[2] = {0x02, 0x02};
		int ret = 0;
		if (class->version  == 1) {
			ret = zwave_api_util_cc(zd->bNodeID,0,  class->classid, command, inparam, 1,  0, outparam, &outlen);
		} else if (class->version >= 2) {
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid, command, inparam, 2,  0, outparam, &outlen);
		}
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}
	
		command = 0x02;
		ret = zwave_api_util_cc(zd->bNodeID,0,  class->classid,  command, NULL, 0,  1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	}

	if (class->version >= 3) {
		char outparam[128];
		int outlen;
		char command = 0x06;
		int ret = zwave_api_util_cc(zd->bNodeID,0,  class->classid,  command, NULL, 0,  1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	}
	
	return 0;
}

static int switch_all_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	char outparam[128];
	int outlen;
	char command = 0x01;
	char inparam[1] = {0x03};
	int ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 1,  0, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}
	
	command = 0x02;
	ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
	if (ret != 0) {
		log_err("[%d] exec class command error: %d", __LINE__, ret);
		return -2;
	}

	char cmdid = outparam[4]&0xff;
	stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
	if (cmd != NULL) {
		device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
	} else {
		log_warn("not support command : %02X", cmdid&0xff);
	}

	return 0;
}

static int protection_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	log_info("[%d]", __LINE__);

	if (class->version == 1) {
		char outparam[128];
		int outlen;
		char command = 0x01;
		char inparam[1] = {0x00};
		int ret;
		ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 1,  0, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		command = 0x02;
		ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
			return -2;
		}

		char cmdid = outparam[4]&0xff;
		stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
		if (cmd != NULL) {
			device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
		} else {
			log_warn("not support command : %02X", cmdid&0xff);
		}
	} else if (class->version == 2) {	
		int timeout_support = 0;
		int exclusive_support = 0;
		int local_state = 0;
		int rf_state = 0;
		{
			char outparam[128];
			int outlen;
			char command = 0x04;
			int ret;
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			char cmdid = outparam[4]&0xff;
			stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
			if (cmd != NULL) {
				device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
			} else {
				log_warn("not support command : %02X", cmdid&0xff);
			}

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
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 1,  0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			command = 0x0a;
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}
			char cmdid = outparam[4]&0xff;
			stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
			if (cmd != NULL) {
				device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
			} else {
				log_warn("not support command : %02X", cmdid&0xff);
			}
		}

		if (exclusive_support) {
			char outparam[128];
			int outlen;
			char command = 0x06;
			char inparam[1] = {0x01};
			int ret;
			ret = zwave_api_util_cc(zd->bNodeID,0,  class->classid, command, inparam, 1,  0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			command = 0x07;
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			char cmdid = outparam[4]&0xff;
			stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
			if (cmd != NULL) {
				device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
			} else {
				log_warn("not support command : %02X", cmdid&0xff);
			}
		}

		if (1) {
			char outparam[128];
			int outlen;
			char command = 0x01;
			char inparam[2] = {0x00, 0x00};
			int ret;
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, inparam, 2,  0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}

			command = 0x02;
			ret = zwave_api_util_cc(zd->bNodeID, 0, class->classid,  command, NULL, 0,  1, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
				return -2;
			}
			char cmdid = outparam[4]&0xff;
			stZWaveCommand_t *cmd = device_get_cmd(class, cmdid);
			if (cmd != NULL) {
				device_update_cmds_data(cmd, &outparam[5], outparam[2] - 2);
			} else {
				log_warn("not support command : %02X", cmdid&0xff);
			}
		}
	}

	return 0;
}
static int configure_init(stZWaveDevice_t *zd, stZWaveClass_t *class) {
	return 0;
}

