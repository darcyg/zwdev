#ifndef __ZWAVE_CLASS_INIT_H_
#define __ZWAVE_CLASS_INIT_H_

typedef int (*CLASS_INIT)(int id, char class, int version);

typedef struct stClassCommandFuncs {
	char							class;
	CLASS_INIT				init;
}stClassCommandFuncs_t;


int zwave_class_init_init(int id, char class, int version);


#endif
