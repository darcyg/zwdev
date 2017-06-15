#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdio.h>
#include "hashmap.h"

int memory_init(struct hashmap *_hm);
int memory_get_attr(int did, const char *attr, char *value);
int memory_set_attr(int did, const char *attr, char *value);

#endif
