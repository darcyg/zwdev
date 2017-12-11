#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "common.h"
#include "lockqueue.h"
#include "log.h"
#include "jansson.h"
#include "json_parser.h"
#include "hex.h"
#include "timer.h"
#include "file_event.h"

#include "frame.h"
#include "zwave.h"
#include "zwave_api.h"
#include "zwave_class.h"
#include "zwave_class_init.h"
#include "zwave_device.h"
#include "zwave_device_storage.h"
#include "zwave_util.h"
#include "zwave_iface.h"


static stZWaveEnv_t ze = {
	.th = NULL,
	.fet = NULL,

	.tr = {},
	.tr_online = {},
	.tr_query = {},

	.eq = {},

	.inventory = {},
};

int zwave_test();

int										zwave_async_data(stDataFrame_t *dfr);

static stInventory_t *zwave_util_get_inventory();
static int						zwave_util_sync_dev();
static int						zwave_util_rpt_status();

static int						zwave_util_class_init(stZWaveDevice_t *zd);


static ZWAVE_IFACE_FUNC zwave_iface_funcs[] = {
};

static void zwave_in(void *arg, int fd) {
	log_info("[%d] -- ", __LINE__);
		
	stDataFrame_t *dfr;
	int ret = zwave_api_util_wait_frame(&dfr, 1000, 0x00);
	if (ret == APIERR_FRAME_RECV_TOU) {
		log_warn("not recv any valid data !");
		return;
	}

	log_debug_hex("zwave recv async data:", dfr->payload, dfr->size);
	
	/* deal the frame */
	zwave_async_data(dfr);

	FREE(dfr); dfr = NULL;
}
static void zwave_step() {
	timer_cancel(ze.th, &ze.tr);
	timer_set(ze.th,&ze.tr, 10);
}
static void zwave_run(struct timer *timer) {
	stEvent_t *e = NULL;

	if (lockqueue_pop(&ze.eq, (void **)&e) && e != NULL) {

		log_info("zwave recv command");
		zwave_iface_funcs[e->eid](e);

		if (e->param != NULL) {
			free(e->param);
			e->param = NULL;
		}

		FREE(e);
		zwave_step();
	} 
}

static void zwave_online_run(struct timer *timer) {

	int i = 0;

	stInventory_t *inv = zwave_util_get_inventory();
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		if (id == 1) {
			continue;
		}

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		} 

		stZWaveDevice_t *zd = device_get_by_nodeid(id);
		if (zd == NULL) {
			continue;
		}

		int online = zd->online;
		if (online > 0) { //online 
			online -= 5*60;
			if (online <= 0) {
				online = 0;
				zwave_util_rpt_status(zd);
			}
		} else if (online <= 0) { //offline , do nothing
			online = 0;
		}

		zd->online = online;
	}

	timer_set(ze.th, &ze.tr_online, 5*60*1000);
}

static void zwave_query_run(struct timer *timer) {
	int i = 0;
	stInventory_t *inv = zwave_util_get_inventory();
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		if (id == 1) {
			continue;
		}

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		} 

		stZWaveDevice_t *zd = device_get_by_nodeid(id);
		if (zd == NULL) {
			continue;
		}
		if (device_is_lowpower(zd)) {
			continue;
		}
		
		/* query the basic class at endpoint 0*/
		char outparam[128];
		int outlen;
		int ret = zwave_api_util_cc(id, 0, 0x20, 0x02, NULL, 0, 0, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
		}
	}

	timer_set(ze.th, &ze.tr_query, 5*60 * 1000);
}

int zwave_init(void *_th, void *_fet, const char *dev, int buad) {
	log_info("[%d]", __LINE__);

	ze.th		= _th;
	ze.fet	= _fet;

	timer_init(&ze.tr, zwave_run);
	timer_init(&ze.tr_online, zwave_online_run);
	timer_init(&ze.tr_query, zwave_query_run);

	if (frame_init(dev, buad) != 0) {
		log_err("[%d] error init frame : %s,%d", __LINE__, dev, buad);
		return -1;
	}


	stInventory_t *inv = zwave_util_get_inventory();
	int ret = zwave_api_ZWaveGetVersion(&inv->ver);
	if (ret != 0) return -2;
	ret = zwave_api_SerialApiGetInitData(&inv->initdata);
	if (ret != 0) return -2;
	ret = zwave_api_SerialApiGetCapabilities(&inv->caps);
	if (ret != 0) return -2;
	ret = zwave_api_MemoryGetId(&inv->id);
	if (ret != 0) return -2;
	ret = zwave_api_ZWaveGetSucNodeId(&inv->sucid);
	if (ret != 0) return -2;
	ret = zwave_util_sync_dev();
	if (ret != 0) return -2;
	
	lockqueue_init(&ze.eq);

	file_event_reg(ze.fet, frame_getfd(), zwave_in, NULL, NULL);

	timer_set(ze.th, &ze.tr, 10);
	timer_set(ze.th, &ze.tr_online, 10);
	timer_set(ze.th, &ze.tr_query, 10);

	return 0;
}

int zwave_push(int eid, void *param, int len) {
	log_info("[%d]", __LINE__);

	stEvent_t *e = MALLOC(sizeof(stEvent_t));
	if (e == NULL) {
		return -1;
	}
	e->eid = eid;
	e->size = len;
	e->param = param;
	lockqueue_push(&ze.eq, e);

	zwave_step();

	return 0;
}


////////////////////////////////////////////////////////////////////////////
stInventory_t* zwave_get_inventory() {
	return zwave_util_get_inventory();
}
int zwave_include() {
	log_debug("[%d]", __LINE__);

	system_led_blink("zwled", 500, 500);
	stAddNodeToNetwork_t antn;
	int ret = zwave_api_ZWaveAddNodeToNetwork(&antn);

	if (ret == 0) {
		stNodeProtoInfo_t npi = {0};
		int id = antn.bSource&0xff;
		zwave_api_ZWaveGetNodeProtoInfo(id&0xff, &npi);
		device_add(id&0xff, npi.Basic, npi.Generic, npi.Specific, 
								npi.Security, npi.Capability, 
								antn.len - 3, antn.commandclasses);

		stZWaveDevice_t *zd = device_get_by_nodeid(id&0xff);
		if (zd != NULL) {
			zd->online = 45*60;

			zwave_util_class_init(zd);

			ds_add_device(zd);

			zwave_iface_report_register(device_get_extaddr(zd));
		}

		stInventory_t *inv = zwave_util_get_inventory();
		zwave_api_SerialApiGetInitData(&inv->initdata);
		zwave_util_sync_dev();
	}
	system_led_off("zwled");

	return ret;
}
int zwave_exclude(char mac[8]) {
	log_debug("[%d]", __LINE__);

	system_led_blink("zwled", 500, 500);

	//stZWaveDevice_t *zd = device_get_by_extaddr(mac);

	int ret = zwave_api_ZWaveRemoveNodeFromNetwork();

	zwave_api_SerialApiGetInitData(&ze.inventory.initdata);
	zwave_util_sync_dev();

	system_led_off("zwled");
	return ret;
}

int zwave_remove_failed_node(const char *mac) {
	log_debug("[%d]", __LINE__);
	
	stZWaveDevice_t *zd = device_get_by_extaddr((char*)mac);
	if (zd == NULL) {
		return -1;
	}

	stNodeInfo_t ni;
	zwave_api_ZWaveRequestNodeInfo(zd->bNodeID, &ni);

	int ret = zwave_api_ZWaveIsFailedNode(zd->bNodeID);
	if (ret < 0) {
		log_warn("api call failed!");
		return -2;
	}

	if (ret == 0) {
		log_warn("not %02X is not a failed node!", zd->bNodeID&0xff);
		return -3; /* not failed node */
	}

	ret = zwave_api_ZWaveRemoveFailedNodeId(zd->bNodeID);
	zwave_api_SerialApiGetInitData(&ze.inventory.initdata);
	zwave_util_sync_dev();

	return ret;
}

int zwave_switch_onoff(char mac[8], char ep, int onoff) {
	log_debug("[%d]", __LINE__);
	
	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		return -1;
	}

	char outparam[128];
	int  outlen;
	char inparam[1] = {onoff ? 0xff : 0x00};
	log_info("id:%d, class:%02x, command:%02x", zd->bNodeID, 0x25, 0x01);
	int ret = zwave_api_util_cc(zd->bNodeID, ep, 0x25, 0x01, inparam, 1, 0, outparam, &outlen);
	if (ret < 0) {
		log_err("[%d] binray switch onoff failed: %d", __LINE__, ret);
		return -2;
	}	

	log_info("id:%d, class:%02x, command:%02x", zd->bNodeID, 0x25, 0x02);
	ret = zwave_api_util_cc(zd->bNodeID, ep, 0x25, 0x02, NULL, 0, 0, outparam, &outlen);
	if (ret < 0) {
		log_err("[%d] binray switch onoff get failed: %d", __LINE__, ret);
		return -3;
	}	
		
	return ret;
}

//////////////////////////////////////////////////////////////////////
int zwave_test() {
#if 0
	stInventory_t *inv = zwave_util_get_inventory();

	int i = 0;
	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		if (id == 1) {
			continue;
		}

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			continue;
		} 

		log_info("[%d] request info : %d", __LINE__, id);
		stNodeInfo_t ni;
		zwave_api_ZWaveRequestNodeInfo(id&0xff, &ni);
		stNodeProtoInfo_t npi = {0};
		zwave_api_ZWaveGetNodeProtoInfo(id&0xff, &npi);
		device_add(id&0xff, npi.Basic, npi.Generic, npi.Specific, 
								npi.Security, npi.Capability, 
								ni.len -3, ni.commandclasses);
		stZWaveDevice_t *zd = device_get_by_nodeid(id&0xff);
		if (zd != NULL) {
			zwave_util_class_init(zd);

			ds_add_device(zd);
		}

		zwave_util_sync_dev();

		log_info("[%d] request info : %d over", __LINE__, id);
	}
#endif
	return 0;
}

int zwave_async_data(stDataFrame_t *dfr) {
	if (dfr->cmd != 0x04) {
		log_warn("[%d] unsupport aysnc data:%02x", __LINE__, dfr->cmd);
		return 0;
	}
	
	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* xx        xx       xxx     version */
	int id = dfr->payload[1]&0xff;
	char classid = dfr->payload[3]&0xff;
	char cmdid = dfr->payload[4]&0xff;
	char *data = &dfr->payload[5];
	int len = dfr->payload[2] - 2;
	char ep = 0;
	

	stZWaveDevice_t *zd = device_get_by_nodeid(id&0xff);
	if (zd == NULL) {
		log_warn("no such device : %02X", id&0xff);
		return  0;
	}

	int online_old = zd->online;
	zd->online = 45 * 60;;
	if (online_old <= 0) {
		zwave_util_rpt_status(zd);
	}

	stZWaveClass_t *zcls = device_get_class(zd, 0, classid);
	if (zcls == NULL) {
		log_warn("no such class : %02X", classid&0xff);
		return  0;
	}

	stZWaveCommand_t *zcmd = device_get_cmd(zcls, cmdid);
	if (zcmd == NULL) {
		log_warn("no such cmd : %02X", cmdid&0xff);
		zwave_iface_report_cmd(zd->mac, ep, classid, cmdid, data, len);
		return  0;
	} 


	device_update_cmds_data(zcmd, data, len);
	ds_update_cmd_data(zcmd);
	{
	#if 0
		if ((classid&0xff) == 0x25 && cmdid == 0x03) {
			zwave_iface_switch_onoff_rpt(zd, 0, zcls, zcmd);
		} else if ((classid&0xff) == 0x71 && cmdid == 0x05) {
			if (dfr->payload[9] == 0x07 && dfr->payload[10] == 0x08) {
				/*
				zwave_iface_notification_rpt(zd, 0, zc, zcmd);
				*/
			}
		} else if ((classid&0xff) == 0x60 && cmdid == 0x0d) {
			/* srcep | dstep | dstclass | dstcommand | data... */
			char ep = dfr->payload[5 + 0];
			zcls = device_get_class(zd, ep, dfr->payload[5 + 2]);
			zcmd = device_get_cmd(zcls, dfr->payload[5+3]);
			device_update_cmds_data(zcmd, &dfr->payload[5+4], dfr->payload[2] - 2 -4);
			zwave_iface_switch_onoff_rpt(zd, ep, zcls, zcmd);
		}
	#else
		if ((classid&0xff) == 0x60 && cmdid == 0x0d) {
			ep = dfr->payload[5 + 0];
			classid = dfr->payload[5 + 2];
			cmdid = dfr->payload[5 + 3];
			data = &dfr->payload[5+4];
			len = dfr->payload[2] - 2 - 4;
			//device_update_cmds_data(zcmd, data, len);
		}

		zwave_iface_report_cmd(zd->mac, ep, classid, cmdid, data, len);

		/* 电池电量和motion一起上报 */
		if  ((classid&0xff) == 0x71  && (cmdid&0xff) == 0x05) {
			if (len == 8) {
				char notification_type		= data[4]&0xff;
				char notification_event	= data[5]&0xff;
				char paramlen						= data[6]&0xff;
				char param								= data[7]&0x7f;
				log_debug("param is %02X", param&0xff);
				if (notification_type == 0x07 && notification_event == 0x08) {
					stZWaveClass_t	 *zcls_batt = device_get_class(zd, 0, 0x80);
					if (zcls_batt != NULL) {
						stZWaveCommand_t *zcmd_batt = device_get_cmd(zcls_batt, 0x03);
						if (zcmd_batt != NULL) {
							device_update_cmds_data(zcmd_batt, &param, sizeof(param));
							ds_update_cmd_data(zcmd_batt);
							char buf_batt[5];
							memcpy(buf_batt, data, 4);
							buf_batt[4] = param;
							zwave_iface_report_cmd(zd->mac, ep, 0x80, 0x03, buf_batt, sizeof(buf_batt));
						}
					}
				}
			}
		}
	#endif

	}

	system_led_shot("zwled");
	return 0;
}


static int	zwave_util_sync_dev() {
	log_info("[%d] -- ", __LINE__);
	stInventory_t *inv = zwave_util_get_inventory();
	int i = 0;

	for (i = 0; i < inv->initdata.nodes_map_size * 8; i++) {
		int id			= i+1;

		if (id == 1) {
			continue;
		}

		int id_bit	= (inv->initdata.nodes_map[i/8] >> (i%8))&0x1;
		if (id_bit == 0) {
			stZWaveDevice_t *zd = device_get_by_nodeid(id);
			if (zd != NULL) {
				zwave_iface_report_unregister(device_get_extaddr(zd));
				ds_del_device(zd);
				device_del(zd->bNodeID);
			}
			continue;
		}

		//stZWaveDevice_t *zd = device_get_by_nodeid(id);
	}	

	return 0;
}

static int	zwave_util_class_init(stZWaveDevice_t *zd) {
	log_info("[%d] -- ", __LINE__);
	int i = 0;
	for (i = 0; i < zd->root.classcnt; i++) {
		zwave_class_init_init(zd, &zd->root.classes[i]);
	}
	return 0;
}

static int						zwave_util_rpt_status(stZWaveDevice_t *zd) {
	log_info("[%d] -- ", __LINE__);
	zwave_iface_report_status(zd->mac);
	return 0;
}

///////////////////////////////////////////parse/////////////////////////////////////////
static stInventory_t *zwave_util_get_inventory() {
	return &ze.inventory;
}



