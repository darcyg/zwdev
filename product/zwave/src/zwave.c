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

stZWaveCache_t zc;

static stZWaveEnv_t ze = {
	.th = NULL,
	.fet = NULL,

	.tr = {},
	.tr_online = {},
	.tr_query = {},

	.eq = {},
};


int		zwave_async_data(stDataFrame_t *dfr);

static int						zwave_util_sync_dev();

static int						zwave_util_get_id_by_mac(const char *mac);
static stInventory_t *zwave_util_get_inventory();
static int						zwave_util_class_init(int id, stAddNodeToNetwork_t *antn);
static json_t*				zwave_util_list();
static int						zwave_util_listchange_auto_report();

static const char *	zwave_parse_dev_mac(json_t *jdev);
static const char *	zwave_parse_dev_type(json_t *jdev);
static const char *	zwave_parse_dev_model(json_t *jdev) ;
static int					zwave_parse_dev_online(json_t *jdev);
static const char *	zwave_parse_dev_version(json_t *jdev);
static int					zwave_parse_dev_battery(json_t *jdev);

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
	timer_cancel(&ze.th, &ze.tr);
	timer_set(&ze.th,&ze.tr, 10);
}
static void zwave_run(struct timer *timer) {
	stEvent_t *e = NULL;

	if (lockqueue_pop(&ze.eq, (void **)&e) && e != NULL) {

		//log_debug("zwave recv command");
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
	int offline_flag = 0;

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
				offline_flag++;
			}
		} else if (online <= 0) { //offline , do nothing
			online = 0;
		}

		zd->online = online;
	}

	timer_set(&ze.th, &ze.tr_online, 5*60*1000);

	if (offline_flag) {
		zwave_util_listchange_auto_report();
	}
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
		if (device_is_lowpower(zd)) {
			continue;
		}
		
		/* query the basic class at endpoint 0*/
		char outparam[128];
		int outlen;
		int ret = zwave_api_util_cc(id, 0x20, 0x02, NULL, 0, 0, outparam, &outlen);
		if (ret != 0) {
			log_err("[%d] exec class command error: %d", __LINE__, ret);
		}
	}

	timer_set(&ze.th, &ze.tr_query, 5*60 * 1000);
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

	int ret = zwave_api_ZWaveGetVersion(&zc.ver);
	if (ret != 0) return -2;
	ret = zwave_api_SerialApiGetInitData(&zc.initdata);
	if (ret != 0) return -2;
	ret = zwave_api_SerialApiGetCapabilities(&zc.caps);
	if (ret != 0) return -2;
	ret = zwave_api_MemoryGetId(&ze.id);
	if (ret != 0) return -2;
	ret = zwave_api_ZWaveGetSucNodeId(&ze.sucid);
	if (ret != 0) return -2;
	ret = zwave_util_sync_dev();
	if (ret != 0) return -2;
	
	lockqueue_init(&ze.eq);

	file_event_reg(ze.fet, frame_getfd(), zwave_in, NULL, NULL);

	timer_set(&ze.th, &ze.tr, 10);
	timer_set(&ze.th, &ze.tr_online, 10);
	timer_set(&ze.th, &ze.tr_query, 10);

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
int zwave_include() {
	log_debug("[%d]", __LINE__);

	stAddNodeToNetwork_t antn;
	int ret = zwave_api_ZWaveAddNodeToNetwork(&antn);

	if (ret == 0) {
		stNodeProtoInfo_t npi = {0};
		int id = antn.bSource&0xff;
		zwave_api_ZWaveGetNodeProtoInfo(id&0xff, &npi);
		device_add(id&0xff, npi.Basic, npi.Generic, npi.Specific, 
								npi.Security, npi.capability, 
								antn.len - 3, antn.commandclasses);

		stZWaveDevice_t *zd = device_get_by_nodeid(id&0xff);
		if (zd != NULL) {
			zwave_util_class_init(zd);
		}

		stInventory_t *inv = zwave_util_get_inventory();
		zwave_api_SerialApiGetInitData(&inv->initdata);
		zwave_util_sync_dev();
	}

	return ret;
}
int zwave_exclude(char mac[8]) {
	log_debug("[%d]", __LINE__);

	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		return 0;
	}

	int ret = zwave_api_ZWaveRemoveNodeFromNetwork();

	zwave_api_SerialApiGetInitData(&ze.inventory.initdata);
	zwave_util_sync_dev();

	return ret;
}

static int zwave_light_onoff(char mac[8], char ep, int onoff) {
	log_debug("[%d]", __LINE__);
	
	stInventory_t *inv = zwave_util_get_inventory();
	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		return -1;
	}

	char outparam[128];
	int  outlen;
	char inparam[1] = {onoff ? 0xff : 0x00};
	log_info("id:%d, class:%02x, command:%02x", id, class, command);
	int ret = zwave_api_util_cc(id, ep, 0x25, 0x01, inparam, 1, 0, outparam, &outlen);
	if (ret < 0) {
		log_err("[%d] binray switch onoff failed: %d", __LINE__, ret);
		break;
	}	

	command = 0x02;
	log_info("id:%d, class:%02x, command:%02x", id, class, command);
	ret = zwave_api_util_cc(id, ep, class, command, NULL, 0, 0, outparam, &outlen);
	if (ret < 0) {
		log_err("[%d] binray switch onoff get failed: %d", __LINE__, ret);
		break;
	}	
		
	return ret;
}

///////////////////////////////////////////parse/////////////////////////////////////////
int zwave_parse_dev_mac(stZWaveDevice_t *zd, char mac[8])  {
	memset(mac, zd->bNodeID, 8);

	stZWaveClass_t *class = device_get_class(0, 0x72);
	if (class != NULL && class->version >= 2) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			memcpy(mac, cmd->data + 2, 8);
		}
	}
	
	log_warn("no manufacturer id, use zwave node id as mac!!!");
	return 0;
}

static const char *zwave_parse_dev_type(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(0, 0x25);
	if (class != NULL) {
		return "1213";
	}
	
	class = device_get_class(0, 0x71);
	if (class != NULL) {
		return "1209";
	}

	return "unkonw";
}
static const char *zwave_parse_dev_model(stZWaveDevice_t *zd)  {
	return "unknow";
}
static int zwave_parse_dev_online(stZWaveDevice_t *zd) {
	int online = zd->online;
	return !!online;
}
static const char *zwave_parse_dev_version(stZWaveDevice_t *zd){
	stZWaveClass_t *class = device_get_class(0, 0x86);
	if (class != NULL) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			char *buf = cmd->data;
			static char version[32];
			sprintf(version, "%02X-%02X.%02X-%02X.%02X", buf[0]&0xff, buf[1]&0xff,buf[2]&0xff, buf[3]&0xff, buf[4]&0xff);
			return version;
		}
	}

	log_info("%d", __LINE__);
	return "unknow";
}

static int zwave_parse_dev_battery(json_t *jdev) {

	stZWaveClass_t *class = device_get_class(0, 0x80);
	if (class != NULL) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			char *buf = cmd->data;
			int battery = buf[0]&0xff;
		}
	}

	return 100;
}

static int zwave_util_get_id_by_mac(const char *mac) {
	int i = 0;
		
	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		return -1;
	}
	
	return zd->bNodeID;
}

static stInventory_t *zwave_util_get_inventory() {
	return &zc.inventory;
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

		json_t * jdev = memory_get_dev(id);
		char *sdev = json_dumps(jdev, 0);
		if (sdev != NULL) {
			log_info("[%d] dev:%02x: %s", __LINE__, id&0xff, sdev);
			FREE(sdev);
			sdev = NULL;
		}

		log_info("[%d] request info : %d", __LINE__, id);
		stNodeInfo_t ni;
		zwave_api_ZWaveRequestNodeInfo(id, &ni);
		log_info("[%d] request info : %d over", __LINE__, id);
	
	}
#endif
	return 0;
}

int zwave_async_data(stDataFrame_t *dfr) {
#if 0
	if (dfr->cmd != 0x04) {
		log_warn("[%d] unsupport aysnc data:%02x", __LINE__, dfr->cmd);
		return 0;
	}
	
	/* rxStatus | sourceNode | len */
	/* class | command | value */
	/* xx        xx       xxx     version */
	int id = dfr->payload[1]&0xff;
	char class = dfr->payload[3]&0xff;
	char command = dfr->payload[4]&0xff;
	
	json_t *jdev = memory_get_dev(id);

	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", class&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	char scommand[32];
	sprintf(scommand, "%02x", command&0xff);
	json_t *jcommand = json_object_get(jclass, scommand);
	if (jcommand != NULL) {
		json_object_del(jclass, scommand);
	}
	char svalue[(dfr->payload[2] - 2) * 3+1];
	hex_string(svalue, sizeof(svalue), (const u8*)&dfr->payload[5], dfr->payload[2] - 2, 1, ' ');
	json_object_set_new(jclass, scommand, json_string(svalue));

	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);

	int online_old; json_get_int(jdev, "online", &online_old);
	json_object_del(jdev, "online");
	json_object_set_new(jdev, "online", json_integer(15*60));

	if (online_old <= 0) {
		zwave_util_listchange_auto_report();
	}
	

	{
		if ((class&0xff) == 0x25 && command == 0x03) {
			const char *mac = zwave_parse_dev_mac(jdev);
			json_t *jrpt = json_object();
			json_object_set_new(jrpt, "mac", json_string(mac));

			json_object_set_new(jrpt, "attr", json_string("device.light.onoff"));

			char rptbuf_val[32];
			sprintf(rptbuf_val, "%d", !!dfr->payload[5]);
			json_object_set_new(jrpt, "value", json_string(rptbuf_val));

			zwave_iface_report(jrpt);
		} else if ((class&0xff) == 0x71 && command == 0x05) {
			if (dfr->payload[9] == 0x07 && dfr->payload[10] == 0x08) {
				const char *mac = zwave_parse_dev_mac(jdev);
				json_t *jrpt = json_object();
				json_object_set_new(jrpt, "mac", json_string(mac));

				json_object_set_new(jrpt, "attr", json_string("device.zone_status"));
				json_object_set_new(jrpt, "zone", json_string("pir"));
				char rptbuf_val[32];
				
				strcpy(ze.sim_mac, mac);
				ze.sim_pir = 1;

				sprintf(rptbuf_val, "%d", ze.sim_pir);
				json_object_set_new(jrpt, "value", json_string(rptbuf_val));

				zwave_iface_report(jrpt);

				timer_set(&ze.th, &ze.sim_tr_pir, 30*1000);
			}
		}
	}
#endif

	system_led_shot("zigbee");
	return 0;
}


static int	zwave_util_sync_dev() {
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
				device_del(zd->bNodeID);
				ds_del_device(zd);
			}
			continue;
		}

		//stZWaveDevice_t *zd = device_get_by_nodeid(id);
	}	

	return 0;
}

static int	zwave_util_class_init(stZWaveDevice_t *zd) {
	int i = 0;
	for (i = 0; i < zd->root.classcnt; i++) {
		//zwave_class_init_init(zd, zd->root.classes[i].classid, 0);
	}
	return 0;
}

static int	zwave_util_listchange_auto_report() {
	/*
	json_t *jmsg = json_object();

	json_t *jdevs = zwave_util_list();
	json_object_set_new(jmsg, "device_list", jdevs);

	char gmac[32]; system_mac_get(gmac);
	json_object_set_new(jmsg, "mac", json_string(gmac));

	json_object_set_new(jmsg, "attr", json_string("mod.device_list"));
	
	zwave_iface_report(jmsg);
	*/

	return 0;
}
