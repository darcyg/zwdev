#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdio.h>
#include <jansson.h>

#include "hashmap.h"

int			memory_init(struct hashmap *_hm);
json_t* memory_get_dev(int did);
int			memory_set_dev(int did, json_t *jdev);
int			memory_del_dev(int did);

int			memory_test();

#endif
