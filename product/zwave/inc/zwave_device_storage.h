#ifndef __ZWAVE_DEVICE_STORAGE_H_
#define __ZWAVE_DEVICE_STORAGE_H_

#include "device.h"


#define BIT_BYPE_NUM(x)	((x + 7)/8)
typedef struct stDsSize {
	int maxnum;
	int size;
	int tag;
}stDsSize_t;

typedef struct stDsElem {
	int   tag;
	int		maxnum;
	int		num;
	int		size;
	char *map;
}stDsElem_t;

typedef struct stDsHeader {
	int				ecnt;
	stDsElem_t	*elems;
}stDsHeader_t;


int ds_init(const char *path);
int ds_load_alldevs(stZWaveDevice_t *zd);
int ds_add_device(stZWaveDevice_t *zd);
int ds_del_device(stZWaveDevice_t *zd);
int ds_update_cmd_data(stZWaveCommand_t *cmd);

#endif
