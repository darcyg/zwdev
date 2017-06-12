#ifndef __FLASH_H_
#define __FLASH_H_



int flash_init(const char *base);
int flash_load_attr(int did, const char *attr, char *value);
int flash_save_attr(int did, const char *attr, char *value);

int flash_load_class(int did, char *cls, int *cnt);
int flash_save_class(int did, char *cls, int cnt);

int flash_save_basic_generic_specific(int did, char b, char g, char s);
int flash_load_basic_generic_specific(int did, char *b, char *g, char *s);

int flash_remove_device(int did);

#endif
