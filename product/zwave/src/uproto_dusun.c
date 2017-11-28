#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "common.h"
#include "log.h"
#include "hex.h"
#include "jansson.h"
#include "json_parser.h"

#include "zwave_iface.h"
#include "zwave_util.h"
#include "zwave_device.h"


#include "uproto.h"


/* Funcstion for cmd */
static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_rpt(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static stUprotoCmd_t ucmds[] = {
	{"getAttribute", uproto_cmd_handler_attr_get},
	{"setAttribute", uproto_cmd_handler_attr_set},
	{"reportAttribute", uproto_cmd_handler_attr_rpt},
};


/* Functions for attr */
static int get_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_new_device_added(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_device_deleted(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int rpt_device_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int get_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);


static int rpt_zone_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static stUprotoAttrCmd_t uattrcmds[] = {
	/* mod */
	{"mod.device_list",						get_mod_device_list,	NULL,									rpt_mod_device_list},
	{"mod.del_device",						NULL,									set_mod_del_device,		NULL},
	{"mod.new_device_added",			NULL,									NULL,									rpt_new_device_added},
	{"mod.add_device",						NULL,									set_mod_add_device,		NULL},
	{"device.status",							NULL,									NULL,									rpt_device_status},
	{"mod.device_deleted",				NULL,									NULL,									rpt_device_deleted},
	{"mod.zwave_info",						get_zwave_info,				NULL,									rpt_zwave_info},

	/* switch */
	{"device.switch.onoff",				NULL,									set_switch_onoff,			rpt_switch_onoff},
	{"device.zone_status",				NULL,									NULL,									rpt_zone_status},
};

//Receive/////////////////////////////////////////////////////////////////////////////
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
		log_warn("not support command:%s", command);
		return -1;
	}


	log_info("handler name : %s", ucmd->name);
	return ucmd->handler(uuid, cmdmac, attr, value);
}

int uproto_handler_cmd_dusun(const char *cmd) {
	json_error_t error;
	json_t *jpkt = json_loads(cmd, 0, &error);
	if (jpkt == NULL) {
		log_warn("error: on line %d: %s", error.line, error.text);
		goto parse_jpkt_error;
	}


	const char *from = json_get_string(jpkt, "from");
	const char *to = json_get_string(jpkt, "to");
	/* CLOUD, GATEWAY, NXP, GREENPOWER BLUETOOTH <ZB3> */
	if (strcmp(from, "CLOUD") != 0 ) {
		log_warn("now not support ubus source : %s", from);
		goto parse_jpkt_error;
	}
	if (strcmp(to, UPROTO_ME) != 0) {
		log_warn("now not support ubus dest : %s", to);
		goto parse_jpkt_error;
	}


	/* registerReq, registerRsp, reportAttribute, reportAttributeResp cmd cmdResult */
	const char *ctype = json_get_string(jpkt, "type");
	if (strcmp(ctype, "cmd") != 0) {
		log_warn("now not support ubus type : %s", ctype);
		goto parse_jpkt_error;
	}

	const char *mac = json_get_string(jpkt, "mac");
	int   dtime = 0; json_get_int(jpkt, "time", &dtime);

	/* verify jdata */
	json_t	*jdata = json_object_get(jpkt, "data");
	if (jdata == NULL) {
		log_warn("not find data item!");
		goto parse_jpkt_error;
	}

	const char *uuid = json_get_string(jdata, "id");
	const char *command = json_get_string(jdata, "command");
	json_t *jarg = json_object_get(jdata, "arguments");
	if (jarg == NULL) {
		log_warn("not find arguments!");
		goto parse_jpkt_error;
	}

	const char *cmdmac = json_get_string(jarg, "mac");
	const char *attr   = json_get_string(jarg, "attribute");
	json_t *    value  = json_object_get(jarg, "value");


	log_info("from:%s,to:%s,type:%s,time:%d,,uuid:%s,command:%s,cmdmac:%s, attr:%s",
			from, to, ctype, dtime, uuid, command, cmdmac, attr);

	_uproto_handler_cmd(from, to, ctype, mac, dtime, uuid, command, cmdmac, attr, value);

parse_jpkt_error:
	if (jpkt != NULL) json_decref(jpkt);
	return -1;
}

//Response////////////////////////////////////////////////////////////////////////
static int uproto_response_ucmd(const char *uuid, int retval) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
	const char *to					= "CLOUD";
	const char *deviceCode	= "00000";
	const char *type				= "cmdResult";
	int ctime								= system_current_time_get(); 
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
//Report///////////////////////////////////////////////////////////////////////////
static int uproto_report_umsg(const char *submac, const char *attr, json_t *jret) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
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

	int ep = -1; json_get_int(jret, "ep", &ep);
	if (json_object_del(jret, "ep") == 0) {
		json_object_set_new(jdata, "ep", json_integer(ep));
	}

	json_object_set_new(jdata, "value", jret);
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);

	return 0;
}

static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	log_info("handler name : %s", uattrcmd->name);
	if (uattrcmd->get == NULL) {
		log_warn("get function is null!!!");
		return -2;
	}
	return uattrcmd->get(uuid, cmdmac, attr, value);
}
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	if (uattrcmd->set == NULL) {
		log_warn("set function is null!!!");
		return -2;
	}
	return uattrcmd->set(uuid, cmdmac, attr, value);
}

static int uproto_cmd_handler_attr_rpt(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	if (uattrcmd->rpt == NULL) {
		log_warn("rpt function is null!!!");
		return -2;
	}
	return uattrcmd->rpt(uuid, cmdmac, attr, value);
}

// Functions //////////////////////////////////////////////////////////////////////////////////////////
static int get_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	//_uproto_handler_cmd(from, to, ctype, mac,dtime, uuid, command,cmdmac,attr,value)
	_uproto_handler_cmd("", "", "", "", 0, uuid, "reportAttribute", cmdmac, "mod.device_list", NULL);

	uproto_response_ucmd(uuid, 0);
	
	return 0;
}
static int rpt_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);


	//json_t *jret = json_object();
	//json_t *jdevices = zwave_iface_list();
	//json_object_set_new(jret, "device_list", jdevices);
	json_t *jret = zwave_iface_list();

	char submac[32];
	system_mac_get(submac);
	uproto_report_umsg(submac, attr, jret);

	return 0;
}
static int get_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	//_uproto_handler_cmd(from, to, ctype, mac,dtime, uuid, command,cmdmac,attr,value)
	_uproto_handler_cmd("", "", "", "", 0, uuid, "reportAttribute", cmdmac, "mod.zwave_info", NULL);

	uproto_response_ucmd(uuid, 0);

	return 0;
	
}
static int rpt_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	json_t *jret = zwave_iface_info();

	char submac[32];
	system_mac_get(submac);
	uproto_report_umsg(submac, attr, jret);

	return 0;
}



static int set_mod_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	
	const char *macstr		= json_get_string(value, "mac");
	const char *typestr  = json_get_string(value, "type");
	if (macstr == NULL || typestr == NULL) {
		log_warn("error arguments (mac/type null?)!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -2;
	}

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), macstr, 0);
	int ret = zwave_iface_exclude(mac);

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);

	return 0;
}

static int rpt_new_device_added(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	uproto_report_umsg(cmdmac, attr, value);

	return 0;
}

static int rpt_device_deleted(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	uproto_report_umsg(cmdmac, attr, value);

	return 0;
}
static int set_mod_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	int ret = zwave_iface_include();
	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);

	return 0;
}

static int rpt_device_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}

static int set_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	const char *val   = json_get_string(value, "value");
	if (val == NULL) { 
		log_warn("error arguments (value null?)!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -2;
	}

	int ep = 0;
	json_get_int(value, "ep", &ep);

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), cmdmac, 0);

	int ret = zwave_iface_device_switch_onoff(mac, ep, !!(val[0] - '0'));

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);
	return 0;
}

static int rpt_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}

static int rpt_zone_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}

//rpt interface///////////////////////////////////////////////////////////////////////////////////////
int uproto_rpt_register_dusun(const char *extaddr) {
	log_info("[%s]", __func__);
		
	
	//_uproto_handler_cmd(from, to, ctype, mac,dtime, uuid, command,cmdmac,attr,value)
	stZWaveDevice_t *zd = device_get_by_extaddr((char *)extaddr);
	if (zd == NULL) {
		char buf[32];
		hex_string(buf, sizeof(buf), (u8*)extaddr, 8, 1, 0);
		log_warn("can't find this device at device table:%s!", buf);
		return 0;
	}

	json_t *jret = json_object();
	json_object_set_new(jret, "mac", json_string(device_make_macstr(zd)));
	json_object_set_new(jret, "type", json_string(device_make_typestr(zd)));
	json_object_set_new(jret, "version", json_string(device_make_versionstr(zd)));
	json_object_set_new(jret, "model", json_string(device_make_versionstr(zd)));
	json_object_set_new(jret, "online", json_integer(device_get_online(zd)));
	json_object_set_new(jret, "battery", json_integer(device_get_battery(zd)));

	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "mod.new_device_added", jret);

	char mac[32];             system_mac_get(mac);
	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", mac, "mod.device_list", NULL);
	return 0;
}

int uproto_rpt_unregister_dusun(const char *extaddr) {
	log_info("[%s]", __func__);

	json_t *jret = json_object();
	char sbuf[32];
	hex_string(sbuf, sizeof(sbuf), (u8*)extaddr, 8, 1, 0);
	json_object_set_new(jret, "mac", json_string(sbuf));

	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", sbuf, "mod.device_deleted", jret);

	char mac[32];             system_mac_get(mac);
	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", mac, "mod.device_list", NULL);
	return 0;
}

int uproto_rpt_status_dusun(const char *extaddr) {
	log_info("[%s]", __func__);

	stZWaveDevice_t *zd = device_get_by_extaddr((char *)extaddr);
	if (zd == NULL) {
		char buf[32];
		hex_string(buf, sizeof(buf), (u8*)extaddr, 8, 1, 0);
		log_warn("can't find this device at device table:%s!", buf);
		return 0;
	}

	json_t *jret = json_object();
	json_object_set_new(jret, "mac", json_string(device_make_macstr(zd)));
	json_object_set_new(jret, "type", json_string(device_make_typestr(zd)));
	json_object_set_new(jret, "version", json_string(device_make_versionstr(zd)));
	json_object_set_new(jret, "model", json_string(device_make_versionstr(zd)));
	json_object_set_new(jret, "online", json_integer(device_get_online(zd)));
	json_object_set_new(jret, "battery", json_integer(device_get_battery(zd)));

	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.status", jret);

	return 0;
}

int uproto_rpt_cmd_dusun(const char *extaddr, unsigned char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len) {

	log_info("[%s]", __func__);

	stZWaveDevice_t *zd = device_get_by_extaddr((char *)extaddr);
	if (zd == NULL) {
		char buf[32];
		hex_string(buf, sizeof(buf), (u8*)extaddr, 8, 1, 0);
		log_warn("can't find this device at device table:%s!", buf);
		return 0;
	}

	if ((clsid == 0x25 && cmdid == 0x03) || (clsid == 0x20 && cmdid == 0x03)) { /* switch onoff */
		json_t *jret = json_object(); 
		char sbuf[32];
		sprintf(sbuf, "%d", !!buf[0]);
		json_object_set_new(jret, "value", json_string(sbuf));
		sprintf(sbuf, "%d", ep);
		json_object_set_new(jret, "ep", json_string(sbuf));

		_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.switch.onoff", jret);
	} else if ((clsid&0xff) == 0x71 && (cmdid&0xff) == 0x05) { /* security event rpt */
		if (len == 8) {
			char notification_type		= buf[4]&0xff;
			char notification_event	= buf[5]&0xff;
			char paramlen						= buf[6]&0xff;
			char param								= buf[7]&0xff;
			if (notification_type == 0x07 && notification_event == 0x08) {
				json_t *jret = json_object(); 

				char sbuf[32];
				sprintf(sbuf, "%d", !!param);
				json_object_set_new(jret, "value", json_string(sbuf));
				
				json_object_set_new(jret, "ep", json_integer(ep&0xff));

				json_object_set_new(jret, "zone", json_string("pir"));

				_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.zone_status", jret);
			} else {
				log_warn("not support class(%02X) cmd(%02X)  notification(%02X), event(%02x)\n", 
							clsid&0xff, cmdid&0xff, notification_type&0xff, notification_event&0xff);
			}
		} else if ((clsid&0xff) == 0x80 && (cmdid&0xff) == 0x03) { /* battery event rpt */
			//char bl = buf[4]&0xff;

			json_t *jret = json_object(); 

			json_object_set_new(jret, "mac", json_string(device_make_macstr(zd)));
			json_object_set_new(jret, "type", json_string(device_make_typestr(zd)));
			json_object_set_new(jret, "version", json_string(device_make_versionstr(zd)));
			json_object_set_new(jret, "model", json_string(device_make_versionstr(zd)));
			json_object_set_new(jret, "online", json_integer(device_get_online(zd)));
			json_object_set_new(jret, "battery", json_integer(device_get_battery(zd)));

			_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.status", jret);
		} else {
			log_warn("class(%02X) cmd(%02X) data too short\n", clsid, cmdid);
		}
	} else {
		log_warn("not support class(%02X) cmd(%02X)\n", clsid, cmdid);
	}

	return 0;
}


int uproto_rpt_attr_dusun(const char *extaddr, unsigned char ep, unsigned char clsid,  const char *buf, int len) {
	log_info("[%s]", __func__);
	/*
	*/
	return 0;
}

