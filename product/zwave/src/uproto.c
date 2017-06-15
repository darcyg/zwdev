#include "uproto.h"
#include "statemachine.h"
#include "log.h"
#include "json_parser.h"
#include "system.h"
#include "zwave.h"

static stUprotoEnv_t ue;

static int uproto_handler_event(stEvent_t *event);

static void ubus_receive_event(struct ubus_context *ctx,struct ubus_event_handler *ev, 
																const char *type,struct blob_attr *msg);
static int uproto_handler_cmd(const char *cmd);
static int _uproto_handler_cmd(const char *from, 
															 const char *to,
													     const char *ctype,
														   const char *mac, 
															 int dtime, 
															 const char *uuid,
															 const char *command,	
															 const char *cmdmac,
															 const char *attr,
															 json_t *value);
static int uproto_response_ucmd(const char *uuid, int retval);
static int uproto_report_umsg(const char *submac, const char *attr, json_t *jret);

static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value);


static int set_gw_reboot(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int get_gw_wifi_settings(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_gw_wifi_settings(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int get_gw_cur_time(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_gw_cur_time(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int get_gw_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_gw_factory_reset(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_gw_firmware(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_gw_remove_shell(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int get_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_mod_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_find_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_device_light_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_device_light_toggle(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_device_light_brightness(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static stUprotoCmd_t ucmds[] = {
	{"getAttribute", uproto_cmd_handler_attr_get},
	{"setAttribute", uproto_cmd_handler_attr_set}
};

static stUprotoAttrCmd_t uattrcmds[] = {
	{"gateway.reboot",						NULL,									set_gw_reboot},
	{"gateway.wifi_settings",			get_gw_wifi_settings, set_gw_wifi_settings},
	{"gateway.current_time",			get_gw_cur_time,			set_gw_cur_time},
	{"gateway.status",						get_gw_status,				NULL},
	{"gateway.factory_reset",			NULL,									set_gw_factory_reset},
	{"gateway.upgrade_firmware",  NULL,									set_gw_firmware},
	{"gateway.remote_shell",			NULL,									set_gw_remove_shell},

	{"mod.device_list",						get_mod_device_list,	NULL},
	{"mod.add_device",						NULL,									set_mod_add_device},
	{"mod.del_device",						NULL,									set_mod_del_device},
	{"mod.find_device",						NULL,									set_mod_find_device},

	{"device.light.onoff",				NULL,									set_device_light_onoff},
	{"device.light.toggle",				NULL,									set_device_light_toggle},
	{"device.light.brightness",		NULL,									set_device_light_brightness},
};

static struct blob_buf b;

int uproto_init(void *_th, void *_fet) {
	ue.th = _th;
	ue.fet = _fet;
	
	timer_init(&ue.step_timer, uproto_run);

	lockqueue_init(&ue.msgq);
	
	ue.ubus_ctx = ubus_connect(NULL);
  memset(&ue.listener, 0, sizeof(ue.listener));
  ue.listener.cb = ubus_receive_event;
  ubus_register_event_handler(ue.ubus_ctx, &ue.listener, "DS.ZWAVE");
  file_event_reg(ue.fet, ue.ubus_ctx->sock.fd, uproto_in, NULL, NULL);
	return 0;
}

int uproto_step() {
	timer_cancel(ue.th, &ue.step_timer);
	timer_set(ue.th, &ue.step_timer, 10);
	return 0;
}
int uproto_push_msg(int eid, void *param, int len) {
	if (eid == UE_SEND_MSG) {
		stEvent_t *e = MALLOC(sizeof(stEvent_t));
		if (e == NULL) {
			 return -1;
		}
		e->eid = eid;
		e->param = param;
		lockqueue_push(&ue.msgq, e);
		uproto_step();
	}
	return 0;
}
void uproto_run(struct timer *timer) {
	stEvent_t *e = NULL;
	if (lockqueue_pop(&ue.msgq, (void **)&e) && e != NULL) {
		uproto_handler_event(e);
		FREE(e);
		uproto_step();
	}
}

static int uproto_handler_event(stEvent_t *e) {
	if (e == NULL) {
		return 0;
	}

	if (e->eid == UE_SEND_MSG && e->param != NULL) {
		json_t *jmsg = e->param;

    char *smsg= json_dumps(jmsg, 0);
    if (smsg != NULL) {
			blob_buf_init(&b, 0);
		  blobmsg_add_string(&b, "PKT", smsg);
      ubus_send_event(ue.ubus_ctx, "DS.ZWAVE", b.head);
      free(smsg);
		}

		json_decref(jmsg);
	}
	return 0;
}

void uproto_in(void *arg, int fd) {
	ubus_handle_event(ue.ubus_ctx);
}


static void ubus_receive_event(struct ubus_context *ctx,struct ubus_event_handler *ev, 
																const char *type,struct blob_attr *msg) {
  char *str;

  log_debug("-----------------[ubus msg]: handler ....-----------------");
  str = blobmsg_format_json(msg, true);
  if (str != NULL) {
    log_debug("[ubus msg]: [%s]", str);

    json_error_t error;
    json_t *jmsg = json_loads(str, 0, &error);
    if (jmsg != NULL) {
      const char *spkt = json_get_string(jmsg, "PKT");
      if (spkt != NULL) {
        log_debug("pks : %s", spkt);
        uproto_handler_cmd(spkt);
      } else {
        log_debug("not find 'PKT' item!");
      }
      json_decref(jmsg);
    } else {
      log_debug("error: on line %d: %s", error.line, error.text);
    }
    free(str);
  } else {
    log_debug("[ubus msg]: []");
  }
  log_debug("-----------------[ubus msg]: handler over-----------------");
}


static int uproto_handler_cmd(const char *cmd) {
	json_error_t error;
  json_t *jpkt = json_loads(cmd, 0, &error);
  if (jpkt == NULL) {
    log_debug("error: on line %d: %s", error.line, error.text);
		goto parse_jpkt_error;
  }
	

	const char *from = json_get_string(jpkt, "from");
  const char *to = json_get_string(jpkt, "to");
  /* CLOUD, GATEWAY, NXP, GREENPOWER BLUETOOTH <ZB3> */
  if (strcmp(from, "CLOUD") != 0 ) {
    log_debug("now not support ubus source : %s", from);
    goto parse_jpkt_error;
  }
  if (strcmp(to, "ZWAVE") != 0) {
    log_debug("now not support ubus dest : %s", to);
    goto parse_jpkt_error;
  }


  /* registerReq, registerRsp, reportAttribute, reportAttributeResp cmd cmdResult */
  const char *ctype = json_get_string(jpkt, "type");
  if (strcmp(ctype, "cmd") != 0) {
    log_debug("now not support ubus type : %s", ctype);
    goto parse_jpkt_error;
  }

  const char *mac = json_get_string(jpkt, "mac");
  int   dtime = 0; json_get_int(jpkt, "time", &dtime);

	/* verify jdata */
  json_t	*jdata = json_object_get(jpkt, "data");
  if (jdata == NULL) {
    log_debug("not find data item!");
    goto parse_jpkt_error;
  }

	const char *uuid = json_get_string(jdata, "id");
	const char *command = json_get_string(jdata, "command");
	json_t *jarg = json_object_get(jdata, "arguments");
	if (jarg == NULL) {
		log_debug("not find arguments!");
		goto parse_jpkt_error;
	}

	const char *cmdmac = json_get_string(jarg, "mac");
	const char *attr   = json_get_string(jarg, "attribute");
	json_t *    value  = json_object_get(jarg, "value");


  log_debug("from:%s,to:%s,type:%s,time:%d,,uuid:%s,command:%s,cmdmac:%s, attr:%s",
						from, to, ctype, dtime, uuid, command, cmdmac, attr);

	_uproto_handler_cmd(from, to, ctype, mac, dtime, uuid, command, cmdmac, attr, value);
		
parse_jpkt_error:
  if (jpkt != NULL) json_decref(jpkt);
	return -1;
}

static stUprotoCmd_t *uproto_search_ucmd(const char *command) {
	int i;
	for (i = 0; i < sizeof(ucmds)/sizeof(ucmds[0]); i++) {
		if (strcmp(command, ucmds[i].name) == 0) {
			return &ucmds[i];
		}
	}
	return NULL;
}
static stUprotoAttrCmd_t *uproto_search_uattrcmd(const char *attr) {
	int i;
	for (i = 0; i < sizeof(uattrcmds)/sizeof(uattrcmds[0]); i++) {
		if (strcmp(attr, uattrcmds[i].name) == 0) {
			return &uattrcmds[i];
		}
	}
	return NULL;
}

static int _uproto_handler_cmd(const char *from, 
															 const char *to,
													     const char *ctype,
														   const char *mac, 
															 int dtime, 
															 const char *uuid,
															 const char *command,	
															 const char *cmdmac,
															 const char *attr,
															 json_t *value) {
	stUprotoCmd_t *ucmd = uproto_search_ucmd(command);	
	if (ucmd == NULL) {
		log_debug("not support command:%s", command);
		return -1;
	}

	return ucmd->handler(uuid, cmdmac, attr, value);
}

static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_debug("not support attribute:%s", attr);
		return -1;
	}
	return uattrcmd->get(uuid, cmdmac, attr, value);
}
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_debug("not support attribute:%s", attr);
		return -1;
	}
	return uattrcmd->set(uuid, cmdmac, attr, value);
}

static int uproto_response_ucmd(const char *uuid, int retval) {
	json_t *jumsg = json_object();

	const char *from				= "ZWAVE";
	const char *to					= "CLOUD";
	const char *deviceCode	= "00000";
	const char *type				= "cmdResult";
	int ctime								= system_current_time_get(time);
	char mac[32];             system_mac_get(mac);

	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(mac));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));

	json_t *jdata = json_object();
	json_object_set_new(jdata, "id",	 json_string(uuid));
	json_object_set_new(jdata, "code", json_integer(retval));
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);


	return 0;
}

static int uproto_report_umsg(const char *submac, const char *attr, json_t *jret) {
	json_t *jumsg = json_object();

	const char *from				= "ZWAVE";
	const char *to					= "CLOUD";
	const char *deviceCode	= "00000";
	const char *type				= "reportAttribute";
	int ctime								= system_current_time_get(time);
	char mac[32];             system_mac_get(mac);

	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(mac));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));

	json_t *jdata = json_object();
	json_object_set_new(jdata, "attribute", json_string(attr));
	//char submac[32];				
	json_object_set_new(jdata, "mac", json_string(submac));
	json_object_set_new(jdata, "value", jret);
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////

static int set_gw_reboot(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	system_reboot();

	uproto_response_ucmd(uuid, 0);

	return 0;
}


static int get_gw_wifi_settings(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);
	

	json_t *jret = json_object();
	if (jret == NULL) {
		log_debug("out of memory!");
		return -1;
	}

	int enable;
	char password[32];
	char ssid[32];
	char mode[16];

	system_wifi_get(&enable, mode, ssid, password);

	json_object_set_new(jret, "enableld",	json_integer(enable));
	json_object_set_new(jret, "password",	json_string(password));
	json_object_set_new(jret, "ssid",			json_string(ssid));
	json_object_set_new(jret, "mode",			json_string(mode));


	char submac[32];
	system_mac_get(submac);
	uproto_report_umsg(submac, attr, jret);

	uproto_response_ucmd(uuid, 0);
	

	return 0;
}
static int set_gw_wifi_settings(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}

	int enable; json_get_int(value, "enable", &enable);
	const char *password	= json_get_string(value, "password");
	const char *ssid			= json_get_string(value, "ssid");
	const char *mode			= json_get_string(value, "mode");
	if (password == NULL || ssid == NULL || mode == NULL) {
		log_debug("error arguments (pass/ssid/mode)!");
		return -2;
	}

	int ret = system_wifi_set(enable, mode, ssid, password);

	uproto_response_ucmd(uuid, ret);

	return 0;
}

static int get_gw_cur_time(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	json_t *jret = json_object();
	if (jret == NULL) {
		log_debug("out of memory!");
		return -1;
	}

	int current_time = system_current_time_get(&current_time);

	json_object_set_new(jret, "time",	json_integer(current_time));

	char submac[32];
	system_mac_get(submac);
	uproto_report_umsg(submac, attr, jret);

	uproto_response_ucmd(uuid, 0);

	return 0;
}
static int set_gw_cur_time(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}

	int current_time; json_get_int(value, "time", &current_time);
	if (current_time <= 0) {
		log_debug("error arguments (pass/ssid/mode)!");
		return -2;
	}

	int ret = system_current_time_set(current_time);

	uproto_response_ucmd(uuid, ret);

	return 0;
}

static int get_gw_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	json_t *jret = json_object();
	if (jret == NULL) {
		log_debug("out of memory!");
		return -1;
	}

	char version[32];
	char model[32];
	char factory[32];
	int current_time;
	int uptime;
	char ethip[32];
	char wifiip[32];
		
	system_version_get(version);
	system_model_get(model);
	system_factory_get(factory);
	current_time = system_current_time_get();
	uptime = system_runtime_get();
	system_eth_ip_get(ethip);
	system_wifi_ip_get(wifiip);

	json_object_set_new(jret, "version",				json_string(version));
	json_object_set_new(jret, "model",					json_string(model));
	json_object_set_new(jret, "factory",				json_string(factory));
	json_object_set_new(jret, "current_time",	json_integer(current_time));
	json_object_set_new(jret, "uptime",				json_integer(uptime));
	json_object_set_new(jret, "ethernet_ip",		json_string(ethip));
	json_object_set_new(jret, "wireless_ip",		json_string(wifiip));


	char submac[32];
	system_mac_get(submac);
	uproto_report_umsg(submac, attr, jret);

	uproto_response_ucmd(uuid, 0);

	return 0;
}

static int set_gw_factory_reset(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);
	

	system_factory_reset();
	
	uproto_response_ucmd(uuid, 0);
	return 0;
}
static int set_gw_firmware(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}

	const char *md5sum = json_get_string(value, "md5sum");
	const char *url = json_get_string(value, "url");

	if (md5sum == NULL || url == NULL) {
		log_debug("error arguments (md5sum/url null?)!");
		return -2;
	}

	system_firmware_update(md5sum, url);

	uproto_response_ucmd(uuid, 0);

	return 0;
}
static int set_gw_remove_shell(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}

	const char *server = json_get_string(value, "server");
	int port=0;  json_get_int(value, "port", &port);

	if (server == NULL || port == 0) {
		log_debug("error arguments (server/port null?)!");
		return -2;
	}

	system_remote_shell(server, port);

	uproto_response_ucmd(uuid, 0);

	return 0;
}

static int get_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	json_t *jret = json_object();
	if (jret == NULL) {
		log_debug("out of memory!");
		return -1;
	}

	json_t *jdevices = zwave_device_list();

	json_object_set_new(jret, "device_list", jdevices);


	char submac[32];
	system_mac_get(submac);
	uproto_report_umsg(submac, attr, jret);

	uproto_response_ucmd(uuid, 0);
	
	return 0;
}

static int set_mod_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}
	
	const char *mac		= json_get_string(value, "mac");
	const char *type  = json_get_string(value, "type");

	if (mac == NULL || type == NULL) {
		log_debug("error arguments (server/port null?)!");
		return -2;
	}

	/* now not support */

	uproto_response_ucmd(uuid, -1);

	return 0;
}
static int set_mod_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}
	
	const char *mac		= json_get_string(value, "mac");
	const char *type  = json_get_string(value, "type");
	if (mac == NULL || type == NULL) {
		log_debug("error arguments (server/port null?)!");
		return -2;
	}


	zwave_del_device(mac);

	uproto_response_ucmd(uuid, -1);


	return 0;
}
static int set_mod_find_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	int ret = zwave_find_device();
	
	uproto_response_ucmd(uuid, ret);

	return 0;
}

static int set_device_light_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}
	
	const char *val		= json_get_string(value, "value");
	if (val == NULL) {
		log_debug("error arguments (value null?)!");
		return -2;
	}

	int ret = zwave_device_light_onoff(cmdmac, !!(val[0] - '0'));

	uproto_response_ucmd(uuid, ret);

	return 0;
}
static int set_device_light_toggle(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	int ret = zwave_device_light_toggle(cmdmac);

	uproto_response_ucmd(uuid, ret);
	
	return 0;
}
static int set_device_light_brightness(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_debug("[%s]", __func__);

	if (value == NULL) {
		log_debug("error arguments!");
		return -1;
	}
	
	const char *val		= json_get_string(value, "value");
	if (val == NULL) {
		log_debug("error arguments (value null?)!");
		return -2;
	}

	int ival = 0;
	int ret;
	if (sscanf(val, "%d", &ival) == 1) {
		ret = zwave_device_light_brightness(cmdmac, val[0] - '0');
	} else {
		log_debug("error brightness value");
		return -3;
	}

	uproto_response_ucmd(uuid, ret);
	return 0;
}

void uproto_report_dev_attr(const char *submac, const char *type, const char *attr, const char *value) {

	if (strcmp(type, "1212") == 0) {
		json_t * jval = zwave_device_light_rpt(attr,value);
		uproto_report_umsg(submac, "device.light.onoff", jval);
	} else if (strcmp(type, "1209") == 0) {
		json_t * jval = zwave_device_pir_rpt(attr, value);
		uproto_report_umsg(submac, "device.zone", jval);
	}
}




