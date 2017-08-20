#ifndef __ZWAVE_CLASS_INIT_H_
#define __ZWAVE_CLASS_INIT_H_

#include "zwave_device.h"

typedef int (*CLASS_INIT)(stZWaveDevice_t *zd, stZWaveClass_t *class);

typedef struct stClassCommandFuncs {
	char							class;
	CLASS_INIT				init;
}stClassCommandFuncs_t;


int zwave_class_init_init(stZWaveDevice_t *zd, stZWaveClass_t *class);


#endif
