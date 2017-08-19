#include "zwave_device.h"

int device_add(char bNodeID, char basic, char generic, 
								char specific, char security, char capability,
								 int classcnt, char *classes) {
}

int device_del(char bNodeID) {
}

stZWaveDevice_t *device_get_by_nodeid(char bNodeID) {
}

stZWaveDevice_t *device_get_by_extaddr(char extaddr[8]) {
}

int device_add_subeps(stZWaveDevice_t *zd, int epcnt, char *eps) {

}

int device_fill_subep(stZWaveDevice_t *zd, char ep, char basic, char generic,
											 char specific, int classcnt, char *classes) {
}

stZWaveEndPoint_t *device_get_subep(stZWaveDevice_t *zd, char ep) {
}

int device_add_cmds(stZWaveClass_t *class, int cmdcnt, char *cmds) {
}

stZWaveCommand_t *device_get_cmd(stZWaveClass_t *class, char cmd) {
}

int device_update_cmds_data(stZWaveCommand_t *zc) {
}


