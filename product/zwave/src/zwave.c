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
#include "system.h"
#include "flash.h"
#include "memory.h"
#include "zwave_iface.h"
#include "zwave.h"
#include "zwave_api.h"
#include "zwave_class.h"
#include "zwave_class_init.h"

static stZWaveEnv_t ze = {
	.inventory = {},
	.th = {.first = NULL},
	.tr = {},
	.tr_online = {},
	.tr_query = {},
	.fet = {},
	.eq = {},
	.run_flag = true,
	.pipe = {-1, -1},
};


int		zwave_async_data(stDataFrame_t *dfr);

static int						zwave_util_sync_dev();

static int						zwave_util_get_id_by_mac(const char *mac);
static stInventory_t *zwave_util_get_inventory();
static int						zwave_util_set_node_info(stAddNodeToNetwork_t *antn);
static int						zwave_util_set_node_protoinfo(int id, stNodeProtoInfo_t *npi);
static int						zwave_util_class_init(int id, stAddNodeToNetwork_t *antn);

static const char *	zwave_parse_dev_mac(json_t *jdev);
static const char *	zwave_parse_dev_type(json_t *jdev);
static const char *	zwave_parse_dev_model(json_t *jdev) ;
static int					zwave_parse_dev_online(json_t *jdev);
static const char *	zwave_parse_dev_version(json_t *jdev);
static int					zwave_parse_dev_battery(json_t *jdev);

static int zwave_list(stEvent_t *e);
static int zwave_include(stEvent_t *e);
static int zwave_exclude(stEvent_t *e);
static int zwave_get(stEvent_t *e);
static int zwave_set(stEvent_t *e);
static int zwave_info(stEvent_t *e);
static int zwave_light_onoff(stEvent_t *e);
static int zwave_light_toggle(stEvent_t *e);
static int zwave_light_brightness(stEvent_t *e);

static ZWAVE_IFACE_FUNC zwave_iface_funcs[] = {
	[E_ZWAVE_LIST] = zwave_list,
	[E_ZWAVE_INCLUDE] = zwave_include,
	[E_ZWAVE_EXCLUDE] = zwave_exclude,
	[E_ZWAVE_GET] = zwave_get,
	[E_ZWAVE_SET] = zwave_set,
	[E_ZWAVE_INFO] = zwave_info,
	[E_ZWAVE_LIGHT_ONOFF] = zwave_light_onoff,
	[E_ZWAVE_LIGHT_TOGGLE] = zwave_light_toggle,
	[E_ZWAVE_LIGHT_BRIGHTNESS] = zwave_light_brightness,
};

static void *zwave_thread(void *arg) {
	log_info("[%s] %d : zwave module main loop", __func__, __LINE__);

	while (ze.run_flag) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&ze.th);
		if (file_event_poll(&ze.fet, next_timeout_ms) < 0) {
			log_warn("[%d] poll error: %m", __LINE__);
		}
	}

	return (void *)0;
}

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
static void pipe_in(void *arg, int fd) {
	char x;
	int ret = read(ze.pipe[0], &x, 1);
	ret = ret;
	zwave_step();
}
static void zwave_run(struct timer *timer) {
	stEvent_t *e = NULL;

	if (lockqueue_pop(&ze.eq, (void **)&e) && e != NULL) {

		//log_debug("zwave recv command");
		zwave_iface_funcs[e->eid](e);

		if (e->param != NULL) {
			json_decref((json_t*)e->param);
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

		json_t * jdev = memory_get_dev(id);
		int online; json_get_int(jdev, "online", &online);
		online -= 5*60;
		if (online <= 0) {
			online = 0;
		}
		json_object_del(jdev, "online");
		json_object_set_new(jdev, "online", json_integer(online));
	}

	timer_set(&ze.th, &ze.tr_online, 5*60*1000);
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

		json_t * jdev = memory_get_dev(id);
		json_t * jclasses = json_object_get(jdev, "classes");

		char sclass[32];
		char class = 0x80;
		sprintf(sclass, "%02x", class&0xff);
		json_t *jclass = json_object_get(jclasses, sclass);
		if (jclass != NULL) {
			continue;
		}


		const char *type = zwave_parse_dev_type(jdev);
		if (strcmp(type, "1212") == 0) {
			char outparam[128];
			int outlen;
			char class = 0x25;
			char command = 0x02;
			int ret = zwave_api_util_cc(id, class, command, NULL, 0, 0, outparam, &outlen);
			if (ret != 0) {
				log_err("[%d] exec class command error: %d", __LINE__, ret);
			}
		}
	}

	timer_set(&ze.th, &ze.tr_query, 5*60 * 1000);
}

int zwave_init(void *_th, void *_fet, const char *dev, int buad) {
	log_info("[%d]", __LINE__);

	timer_init(&ze.tr, zwave_run);
	timer_init(&ze.tr_online, zwave_online_run);
	timer_init(&ze.tr_query, zwave_query_run);
	file_event_init(&ze.fet);

	if (frame_init(dev, buad) != 0) {
		log_err("[%d] error init frame : %s,%d", __LINE__, dev, buad);
		return -1;
	}

	int ret = zwave_api_ZWaveGetVersion(&ze.inventory.ver);
	if (ret != 0) return -2;
	ret = zwave_api_SerialApiGetInitData(&ze.inventory.initdata);
	if (ret != 0) return -2;
	ret = zwave_api_SerialApiGetCapabilities(&ze.inventory.caps);
	if (ret != 0) return -2;
	ret = zwave_api_MemoryGetId(&ze.inventory.id);
	if (ret != 0) return -2;
	ret = zwave_api_ZWaveGetSucNodeId(&ze.inventory.sucid);
	if (ret != 0) return -2;
	ret = zwave_util_sync_dev();
	if (ret != 0) return -2;
	

	lockqueue_init(&ze.eq);
	file_event_reg(&ze.fet, frame_getfd(), zwave_in, NULL, NULL);
	int ret = pipe(ze.pipe);
	ret = ret;
	file_event_reg(&ze.fet, ze.pipe[0], pipe_in, NULL, NULL);

	timer_set(&ze.th, &ze.tr, 10);
	timer_set(&ze.th, &ze.tr_online, 10);
	timer_set(&ze.th, &ze.tr_query, 10);

	pthread_t pid;
	pthread_create(&pid, NULL, zwave_thread, NULL);

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

	//zwave_step();
	int ret = write(ze.pipe[1], "A", 1);
	ret = ret;

	return 0;
}


////////////////////////////////////////////////////////////////////////////
static int zwave_list(stEvent_t *e) {
	log_debug("[%d]", __LINE__);

	json_t *jdevs = json_array();
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
		
		const char *mac = zwave_parse_dev_mac(jdev);
		const char *type = zwave_parse_dev_type(jdev);
		const char *model = zwave_parse_dev_model(jdev);
		int online = zwave_parse_dev_online(jdev);
		const char *version = zwave_parse_dev_version(jdev);
		int battery = zwave_parse_dev_battery(jdev);

		json_t *jitem = json_object();
		json_object_set_new(jitem,	"mac",			json_string(mac));
		json_object_set_new(jitem,	"type",			json_string(type));
		json_object_set_new(jitem,	"model",		json_string(model));
		json_object_set_new(jitem,	"online",		json_integer(online));
		json_object_set_new(jitem,	"version",	json_string(version));
		json_object_set_new(jitem,	"battery",	json_integer(battery));

		json_array_append_new(jdevs, jitem);
	}

	zwave_iface_push(jdevs);

	return 0;
}

static int zwave_include(stEvent_t *e) {
	log_debug("[%d]", __LINE__);

	stAddNodeToNetwork_t antn;
	int ret = zwave_api_ZWaveAddNodeToNetwork(&antn);

	json_t *jret = json_object();
	json_object_set_new(jret, "ret", json_integer(ret));
	zwave_iface_push(jret);

	if (ret == 0) {
		zwave_api_SerialApiGetInitData(&ze.inventory.initdata);
		zwave_util_sync_dev();

		zwave_util_set_node_info(&antn);

		int id = antn.bSource&0xff;
		stNodeProtoInfo_t npi;
		ret = zwave_api_ZWaveGetNodeProtoInfo(id&0xff, &npi);
		if (ret == 0) {
			zwave_util_set_node_protoinfo(id&0xff, &npi);
		}

		zwave_util_class_init(id&0xff, &antn);
	}
	return 0;
}
static int zwave_exclude(stEvent_t *e) {
	log_debug("[%d]", __LINE__);
	json_t *jarg = (json_t*)e->param;
	const char *mac = json_get_string(jarg, "mac");
	mac = mac;

	int ret = zwave_api_ZWaveRemoveNodeFromNetwork();

	json_t *jret = json_object();
	json_object_set_new(jret, "ret", json_integer(ret));
	zwave_iface_push(jret);

	if (ret == 0) {
		zwave_api_SerialApiGetInitData(&ze.inventory.initdata);
		zwave_util_sync_dev();
	}
	return 0;
}
static int zwave_get(stEvent_t *e) {
	log_debug("[%d]", __LINE__);
	return 0;
}
static int zwave_set(stEvent_t *e) {
	log_debug("[%d]", __LINE__);
	return 0;
}
static int zwave_info(stEvent_t *e){
	log_debug("[%d]", __LINE__);

	json_t *jinfo = json_object();

	char buf[64];
	stInventory_t *inv = zwave_util_get_inventory();
	stId_t *i = &inv->id;
	
	sprintf(buf, "%08X", i->HomeID);
	json_object_set_new(jinfo, "HomeID", json_string(buf));
	sprintf(buf, "%02X", i->NodeID);
	json_object_set_new(jinfo, "NodeID", json_string(buf));
	
	
	zwave_iface_push(jinfo);

	return 0;
}
static int zwave_light_onoff(stEvent_t *e) {
	log_debug("[%d]", __LINE__);
	
	json_t *jarg = (json_t*)e->param;

	const char *mac = json_get_string(jarg, "mac");
	int onoff; json_get_int(jarg, "onoff", &onoff);

	stInventory_t *inv = zwave_util_get_inventory();
	int i = 0;
	json_t *jret = json_object();
	json_object_set_new(jret, "ret", json_integer(-1));
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
		char smac[32];

		if (strlen(mac) <= 4) {
			sprintf(smac, "%02X", id&0xff);
		}  else {
			strcpy(smac, zwave_parse_dev_mac(jdev));
		}
		if (strcmp(smac, mac) != 0) {
			continue;
		}

		
		char outparam[128];
		int outlen;
		char inparam[1] = {onoff ? 0xff : 0x00};
		char class = 0x25;
		char command = 0x01;
		log_info("id:%d, class:%02x, command:%02x", id, class, command);
		int ret = zwave_api_util_cc(id, class, command, inparam, 1, 0, outparam, &outlen);
		
		if (ret < 0) {
			log_err("[%d] binray switch onoff failed: %d", __LINE__, ret);
			break;
		}	

		command = 0x02;
		log_info("id:%d, class:%02x, command:%02x", id, class, command);
		ret = zwave_api_util_cc(id, class, command, NULL, 0, 0, outparam, &outlen);
		if (ret < 0) {
			log_err("[%d] binray switch onoff get failed: %d", __LINE__, ret);
			break;
		}	
		
		json_object_del(jret, "ret");
		json_object_set_new(jret, "ret", json_integer(0));
	}

	zwave_iface_push(jret);

	return 0;
}
static int zwave_light_toggle(stEvent_t *e) {
	log_debug("[%d]", __LINE__);
	return 0;
}
static int zwave_light_brightness(stEvent_t *e) {
	log_debug("[%d]", __LINE__);
	return 0;
}

///////////////////////////////////////////parse/////////////////////////////////////////
static const char *zwave_parse_dev_mac(json_t *jdev)  {
	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", 0x72&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);

	if (jclass != NULL) {
		int version; json_get_int(jclass, "version", &version);
		if (version == 2) {
			char scommand[32];
			sprintf(scommand, "%02x", 0x07&0xff);
			const char *svalue = json_get_string(jclass, scommand);

			if (svalue != NULL) {
				char buf[32];
				hex_parse((u8*)buf, sizeof(buf), svalue, NULL);

				static char mac[32];
				hex_string(mac, sizeof(mac), (const u8*)buf+2, 8, 1, 0);
				return mac;
			}
		}
	}

	log_warn("no manufacturer id, use zwave node id as mac!!!");
	int id; json_get_int(jdev, "id", &id);
	log_info("id is %d", id);
	static char sid[32];
	sprintf(sid, "%02x", id&0xff);
	return sid;
}

static const char *zwave_parse_dev_type(json_t *jdev) {
	json_t *jclasses = json_object_get(jdev, "classes");
	
	char sclass[32];
	sprintf(sclass, "%02x", 0x25&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);
	if (jclass != NULL) {
		return "1212";
	}

	
	sprintf(sclass, "%02x", 0x71&0xff);
	jclass = json_object_get(jclasses, sclass);
	if (jclass != NULL) {
		return "1209";
	}

	
	return "unkonw";
}
static const char *zwave_parse_dev_model(json_t *jdev)  {
	return "unknow";
}
static int zwave_parse_dev_online(json_t *jdev) {
	int online = 0; json_get_int(jdev, "online", &online);
	return !!online;
}
static const char *zwave_parse_dev_version(json_t *jdev){
	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", 0x86&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);
	
	if (jclass != NULL) {
		char scommand[32];
		sprintf(scommand, "%02x", 0x12&0xff);
		const char *svalue = json_get_string(jclass, scommand);

		if (svalue != NULL) {
			char buf[32];
			hex_parse((u8*)buf, sizeof(buf), svalue, NULL);
				
			static char version[32];
			sprintf(version, "%02X-%02X.%02X-%02X.%02X", buf[0]&0xff, buf[1]&0xff,buf[2]&0xff, buf[3]&0xff, buf[4]&0xff);
			return version;
		}
	}
	log_info("%d", __LINE__);
	return "unknow";
}

static int zwave_parse_dev_battery(json_t *jdev) {
	json_t *jclasses = json_object_get(jdev, "classes");

	char sclass[32];
	sprintf(sclass, "%02x", 0x80&0xff);
	json_t *jclass = json_object_get(jclasses, sclass);
	
	if (jclass != NULL) {
		char scommand[32];
		sprintf(scommand, "%02x", 0x03&0xff);
		const char *svalue = json_get_string(jclass, scommand);
		if (svalue != NULL) {
			char buf[32];
			hex_parse((u8*)buf, sizeof(buf), svalue, NULL);
			int battery = buf[0]&0xff;
			return battery;
		}
	}

	return 100;
}

static int zwave_util_get_id_by_mac(const char *mac) {
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

		json_t * jdev = memory_get_dev(id);
		char smac[32];

		if (strlen(mac) <= 4) {
			sprintf(smac, "%02X", id&0xff);
		}  else {
			strcpy(smac, zwave_parse_dev_mac(jdev));
		}
		log_info("smac:%s,mac:%s", smac, mac);
		if (strcmp(smac, mac) != 0) {
			continue;
		}

		return id;
	}
	return -1;
}

static stInventory_t *zwave_util_get_inventory() {
	return &ze.inventory;
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

	json_object_del(jdev, "online");
	json_object_set_new(jdev, "online", json_integer(15*60));

	{
		if ((class&0xff) == 0x25 && command == 0x03) {
			const char *mac = zwave_parse_dev_mac(jdev);
			json_t *jrpt = json_object();
			json_object_set_new(jrpt, "mac", json_string(mac));

			json_object_set_new(jrpt, "attr", json_string("device.light.onoff"));
			json_object_set_new(jrpt, "value", json_integer(!!dfr->payload[5]));

			zwave_iface_report(jrpt);
		} else if ((class&0xff) == 0x71 && command == 0x05) {
			if (dfr->payload[9] == 0x07 && dfr->payload[10] == 0x08) {
				const char *mac = zwave_parse_dev_mac(jdev);
				json_t *jrpt = json_object();
				json_object_set_new(jrpt, "mac", json_string(mac));

				json_object_set_new(jrpt, "attr", json_string("zone.status"));
				json_object_set_new(jrpt, "name", json_string("pir"));
				json_object_set_new(jrpt, "value", json_integer(!!dfr->payload[3]));

				zwave_iface_report(jrpt);
			}
		}
	}

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
			memory_del_dev(id);
			flash_remove_dev(id);
			continue;
		}

		json_t * jdev = flash_load_dev(id);
		if (jdev == NULL) {
			jdev = json_object();
			json_object_set_new(jdev, "id", json_integer(id));
			memory_set_dev(id, jdev);
			flash_save_dev(id, jdev);
		} else {
			memory_del_dev(id);
			memory_set_dev(id, jdev);
		}
	}	

	return 0;
}

static int	zwave_util_set_node_info(stAddNodeToNetwork_t *antn) {
	int id = antn->bSource&0xff;
	json_t *jdev = memory_get_dev(id);
	if (jdev != NULL) {
		memory_del_dev(id);
		flash_remove_dev(id);
	}

	jdev = json_object();

	char buf[32];
	sprintf(buf, "%02x", id&0xff);
	json_object_set_new(jdev, "id", json_string(buf));
	sprintf(buf, "%02x", antn->basic&0xff); json_object_set_new(jdev, "basic", json_string(buf));
	sprintf(buf, "%02x", antn->generic&0xff); json_object_set_new(jdev, "generic", json_string(buf));
	sprintf(buf, "%02x", antn->specific&0xff); json_object_set_new(jdev, "specific", json_string(buf));
	json_object_set_new(jdev, "online", json_integer(15*60));

	json_t *jclasses = json_object();
	int i = 0;
	for (i = 0; i < antn->len - 3; i++) {
		sprintf(buf, "%02x", antn->commandclasses[i]&0xff);
		json_object_set_new(jclasses, buf, json_object());
	}
	json_object_set_new(jdev,"classes", jclasses);

	memory_del_dev(id);
	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);

	return 0;
}
static int	zwave_util_class_init(int id, stAddNodeToNetwork_t *antn) {
	json_t *jdev = memory_get_dev(id);
	if (jdev == NULL) {
		return -1;
	}

	int i = 0;
	for (i = 0; i < antn->len - 3; i++) {
		char class = antn->commandclasses[i]&0xff;
#if 1
		zwave_class_init_init(id&0xff, class, 0);
#else
		log_info("zwave class init ID: %02X, CLASS:%02X\n", id&0xff, class&0xff);
#endif
	}

	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);

	return 0;
}

static int	zwave_util_set_node_protoinfo(int id, stNodeProtoInfo_t *npi) {
	json_t *jdev = memory_get_dev(id);
	if (jdev == NULL) {
		return -1;
	}

	char buf[64];

	sprintf(buf, "%02x", npi->Capability&0xff);
	json_object_set_new(jdev, "capability", json_string(buf));

	sprintf(buf, "%02x", npi->Security&0xff);
	json_object_set_new(jdev, "security", json_string(buf));

	memory_set_dev(id, jdev);
	flash_save_dev(id, jdev);
	return 0;
}
