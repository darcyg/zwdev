#ifndef __ZWAVE_DEVICE_H_
#define __ZWAVE_DEVICE_H_

#define MAX_CMD_DATA_LEN		32
#define MAX_DEV_NUM					10

/* when a command has been received, wo query a command table and decied if it can be store in the command data part, 
 * if yes, we only store the data to the data pointer
 * if not, we parse it to a parse function to parse it to a plat data such as endpoint ...
 */
typedef struct stZWaveCommand {
	char						cmdid;
	int							len;
	char						data[32];
	int							nbit;
	int							nbit_next;
} stZWaveCommand_t;

typedef struct stZWaveClass {
	char						classid;
	int							version;
	int							cmdcnt;
	stZWaveCommand_t	*cmds;
	int							nbit;
	int							nbit_next;
	int							nbit_cmds_head;
}stZWaveClass_t;

typedef struct stZWaveEndPoint {
	char	ep;

	char	basic;
	char	generic;
	char	specific;

	int		classcnt;
	stZWaveClass_t *classes;

		int		nbit;
	int   nbit_next;
	int		nbit_classes_head;
}stZWaveEndPoint_t;

typedef struct stZWaveDevice {
	char							bNodeID;
	char							security;
	char							capability;

	int								used;

	stZWaveEndPoint_t	root;
	int								subepcnt;
	stZWaveEndPoint_t *subeps;
	int								nbit;
	int								nbit_subeps_head;

	int								online;
	char							mac[8];
}stZWaveDevice_t;

typedef struct stZWaveCache {
	int								devcnt;
	stZWaveDevice_t		devs[MAX_DEV_NUM];
}stZWaveCache_t;


int device_add(char bNodeID, char basic, char generic, char specific, char security, char capability, int classcnt, char *classes); 
int device_del(char bNodeID);
stZWaveDevice_t *device_get_by_nodeid(char bNodeID);
stZWaveDevice_t *device_get_by_extaddr(char extaddr[8]);

int device_add_subeps(stZWaveDevice_t *zd, int epcnt, char *eps);
int device_fill_ep(stZWaveEndPoint_t *ze, char ep, char basic, char generic, char specific, int classcnt, char *classes);
stZWaveEndPoint_t *device_get_subep(stZWaveDevice_t *zd, char ep);

int device_add_cmds(stZWaveClass_t *class, int cmdcnt, char *cmds);
stZWaveCommand_t *device_get_cmd(stZWaveClass_t *class, char cmd);
int device_update_cmds_data(stZWaveCommand_t *zc, char *data, int len);


char *device_get_extaddr(stZWaveDevice_t *zd);


stZWaveClass_t *device_get_class(stZWaveDevice_t *zd, char ep, char classid);


const char *device_make_macstr(stZWaveDevice_t *zd);
int device_get_battery(stZWaveDevice_t *zd);
int device_get_online(stZWaveDevice_t *zd);
const char *device_make_modelstr(stZWaveDevice_t *zd);
int device_is_lowpower(stZWaveDevice_t *zd);
const char *device_make_typestr(stZWaveDevice_t *zd);
const char *device_make_versionstr(stZWaveDevice_t *zd);
char device_get_nodeid_by_mac(const char *mac);
#endif
