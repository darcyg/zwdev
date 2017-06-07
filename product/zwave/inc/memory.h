#ifndef __MEMORY_H_
#define __MEMORY_H_

int memory_init();
int memory_get_attr(int did, const char *attr, char *value);
int memory_set_attr(int did, const char *attr, char *value);

#endif
